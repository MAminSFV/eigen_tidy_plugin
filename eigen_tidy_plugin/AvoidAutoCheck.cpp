#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Config/llvm-config.h"

#include "AvoidAutoCheck.hpp"

// Compatibility helper for LLVM versions
// LLVM 15 and earlier use startswith, LLVM 16+ use starts_with
#if LLVM_VERSION_MAJOR >= 16
#define LLVM_STARTS_WITH(str, prefix) (str).starts_with(prefix)
#else
#define LLVM_STARTS_WITH(str, prefix) (str).startswith(prefix)
#endif

using namespace clang::ast_matchers;

namespace clang::tidy::eigen {

AvoidAutoCheck::AvoidAutoCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {
  // Read configuration options with defaults
  auto OnlyExpressionsOpt = Options.get("OnlyExpressions");
  OnlyExpressions = OnlyExpressionsOpt.has_value() &&
                    (OnlyExpressionsOpt.value() == "true" || OnlyExpressionsOpt.value() == "1");

  auto BanDecltypeAutoOpt = Options.get("BanDecltypeAuto");
  BanDecltypeAuto = !BanDecltypeAutoOpt.has_value() ||
                    (BanDecltypeAutoOpt.value() != "false" && BanDecltypeAutoOpt.value() != "0");
}
void AvoidAutoCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "OnlyExpressions", OnlyExpressions);
  Options.store(Opts, "BanDecltypeAuto", BanDecltypeAuto);
}

void AvoidAutoCheck::registerMatchers(MatchFinder *Finder) {
  // Match variable declarations with auto or decltype(auto)
  // Match both direct auto and references to auto (auto&, auto&&)
  auto AutoVarDecl =
      varDecl(anyOf(hasType(autoType()), hasType(referenceType(pointee(autoType())))),
              hasInitializer(expr().bind("init")))
          .bind("var");

  auto DecltypeAutoVarDecl =
      varDecl(anyOf(hasType(decltypeType()), hasType(referenceType(pointee(decltypeType())))),
              hasInitializer(expr().bind("init")))
          .bind("decltype_var");

  Finder->addMatcher(AutoVarDecl, this);

  if (BanDecltypeAuto) {
    Finder->addMatcher(DecltypeAutoVarDecl, this);
  }
}

void AvoidAutoCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Var = Result.Nodes.getNodeAs<VarDecl>("var");
  const auto *DecltypeVar = Result.Nodes.getNodeAs<VarDecl>("decltype_var");
  const auto *Init = Result.Nodes.getNodeAs<Expr>("init");

  // Use whichever variable was matched
  const VarDecl *VD = Var ? Var : DecltypeVar;
  if (!VD || !Init) {
    return;
  }

  // Skip if in system header
  if (Result.Context->getSourceManager().isInSystemHeader(VD->getLocation())) {
    return;
  }

  // Verify the variable was actually spelled with auto/decltype(auto)
  TypeSourceInfo *TSI = VD->getTypeSourceInfo();
  if (!TSI) {
    return;
  }

  // Check if this variable's type was deduced from auto
  bool IsDeducedAuto = false;
  bool IsDecltypeAuto = false;

  // Check the TypeSourceInfo for auto keywords
  TypeLoc TL = TSI->getTypeLoc();

  // Function to recursively search for AutoTypeLoc in TypeLoc hierarchy
  std::function<bool(TypeLoc)> findAutoTypeLoc = [&](TypeLoc TL) -> bool {
    if (TL.getAs<AutoTypeLoc>()) {
      return true;
    }
    if (auto QTL = TL.getAs<QualifiedTypeLoc>()) {
      return findAutoTypeLoc(QTL.getUnqualifiedLoc());
    }
    if (auto RTL = TL.getAs<ReferenceTypeLoc>()) {
      return findAutoTypeLoc(RTL.getPointeeLoc());
    }
    if (auto PTL = TL.getAs<PointerTypeLoc>()) {
      return findAutoTypeLoc(PTL.getPointeeLoc());
    }
    return false;
  };

  if (TL.getAs<DecltypeTypeLoc>()) {
    IsDecltypeAuto = true;
  } else if (findAutoTypeLoc(TL)) {
    IsDeducedAuto = true;
  }

  if (!IsDeducedAuto && !IsDecltypeAuto) {
    return; // Not actually spelled with auto
  }

  // Get the deduced type
  QualType DeducedType = VD->getType();

  // Strip references, pointers, and cv-qualifiers
  DeducedType = DeducedType.getCanonicalType();
  while (DeducedType->isPointerType() || DeducedType->isReferenceType()) {
    if (DeducedType->isPointerType()) {
      DeducedType = DeducedType->getPointeeType();
    } else {
      DeducedType = DeducedType.getNonReferenceType();
    }
  }
  DeducedType = DeducedType.getUnqualifiedType();

  // Check if the type belongs to Eigen
  if (!isEigenType(DeducedType.getTypePtr())) {
    return;
  }

  // If OnlyExpressions is true, only warn for expression templates
  if (OnlyExpressions && isEigenPlainObject(DeducedType.getTypePtr())) {
    return;
  }

  // Emit the diagnostic
  StringRef TypeKeyword = IsDecltypeAuto ? "decltype(auto)" : "auto";

  auto Diag =
      diag(VD->getLocation(), "do not use '%0' for Eigen types or expressions; declare an "
                              "explicit Eigen type or assign the whole expression to a "
                              "concrete type (e.g., (expr).eval() into Eigen::Matrix<>). "
                              "See Eigen pitfalls: "
                              "https://libeigen.gitlab.io/eigen/docs-nightly/TopicPitfalls.html")
      << TypeKeyword;

  // Highlight the auto keyword
  Diag << SourceRange(TL.getBeginLoc(), TL.getEndLoc());
}

bool AvoidAutoCheck::isEigenType(const Type *Ty) const {
  if (!Ty) {
    return false;
  }

  // Handle record types (classes/structs)
  if (const auto *RT = Ty->getAs<RecordType>()) {
    if (const auto *RD = dyn_cast<CXXRecordDecl>(RT->getDecl())) {
      return isInEigenNamespace(RD);
    }
  }

  // Handle template specializations
  if (const auto *TST = Ty->getAs<TemplateSpecializationType>()) {
    if (const auto *TD = TST->getTemplateName().getAsTemplateDecl()) {
      if (const auto *RD = dyn_cast<CXXRecordDecl>(TD->getTemplatedDecl())) {
        return isInEigenNamespace(RD);
      }
    }
  }

  return false;
}

bool AvoidAutoCheck::isInEigenNamespace(const CXXRecordDecl *RD) const {
  if (!RD) {
    return false;
  }

  // Walk up the declaration context chain looking for the Eigen namespace
  const DeclContext *DC = RD->getDeclContext();
  while (DC) {
    if (const auto *ND = dyn_cast<NamespaceDecl>(DC)) {
      if (ND->getName() == "Eigen") {
        // Check if this is the top-level Eigen namespace (not nested in another
        // namespace)
        const DeclContext *Parent = ND->getParent();
        if (Parent && isa<TranslationUnitDecl>(Parent)) {
          return true;
        }
      }
    }
    DC = DC->getParent();
  }

  return false;
}

bool AvoidAutoCheck::isEigenExpression(const Type *Ty) const {
  if (!isEigenType(Ty)) {
    return false;
  }

  // Heuristic: Expression templates typically don't derive from PlainObjectBase
  return !isEigenPlainObject(Ty);
}

bool AvoidAutoCheck::isEigenPlainObject(const Type *Ty) const {
  if (!Ty) {
    return false;
  }

  const auto *RT = Ty->getAs<RecordType>();
  if (!RT) {
    return false;
  }

  const auto *RD = dyn_cast<CXXRecordDecl>(RT->getDecl());
  if (!RD) {
    return false;
  }

  // Check if this class derives from Eigen::PlainObjectBase
  // This is a heuristic - we look for inheritance from classes named
  // PlainObjectBase in the Eigen namespace
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    QualType BaseType = Base.getType();
    if (const auto *BaseRT = BaseType->getAs<RecordType>()) {
      if (const auto *BaseRD = dyn_cast<CXXRecordDecl>(BaseRT->getDecl())) {
        if (BaseRD->getName() == "PlainObjectBase" && isInEigenNamespace(BaseRD)) {
          return true;
        }
        // Recursively check base classes
        if (isEigenPlainObject(BaseType.getTypePtr())) {
          return true;
        }
      }
    }
  }

  // Also check for common Eigen concrete types by name
  StringRef ClassName = RD->getName();
  if (isInEigenNamespace(RD)) {
    if (LLVM_STARTS_WITH(ClassName, "Matrix") || LLVM_STARTS_WITH(ClassName, "Array") ||
        LLVM_STARTS_WITH(ClassName, "Vector") || ClassName == "Quaternion" ||
        ClassName == "AngleAxis" || ClassName == "Transform") {
      return true;
    }
  }

  return false;
}

} // namespace clang::tidy::eigen
