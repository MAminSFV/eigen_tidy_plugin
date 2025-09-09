#include "clang-tidy-eigen/AvoidAutoForEigenCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace eigen {

AvoidAutoForEigenCheck::AvoidAutoForEigenCheck(StringRef Name,
                                                ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {
  // Read configuration options with defaults
  auto AllowInRangeForOpt = Options.get("AllowInRangeFor");
  AllowInRangeFor = AllowInRangeForOpt.has_value() &&
                    (AllowInRangeForOpt.value() == "true" || AllowInRangeForOpt.value() == "1");

  auto OnlyExpressionsOpt = Options.get("OnlyExpressions");
  OnlyExpressions = OnlyExpressionsOpt.has_value() &&
                    (OnlyExpressionsOpt.value() == "true" || OnlyExpressionsOpt.value() == "1");

  auto BanDecltypeAutoOpt = Options.get("BanDecltypeAuto");
  BanDecltypeAuto = !BanDecltypeAutoOpt.has_value() ||
                    (BanDecltypeAutoOpt.value() != "false" && BanDecltypeAutoOpt.value() != "0");
}void AvoidAutoForEigenCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "AllowInRangeFor", AllowInRangeFor);
  Options.store(Opts, "OnlyExpressions", OnlyExpressions);
  Options.store(Opts, "BanDecltypeAuto", BanDecltypeAuto);
}

void AvoidAutoForEigenCheck::registerMatchers(MatchFinder *Finder) {
  // Match variable declarations with auto or decltype(auto)
  auto AutoVarDecl = varDecl(
      hasType(autoType()),
      hasInitializer(expr().bind("init"))
  ).bind("var");

  auto DecltypeAutoVarDecl = varDecl(
      hasType(decltypeType()),
      hasInitializer(expr().bind("init"))
  ).bind("decltype_var");

  Finder->addMatcher(AutoVarDecl, this);

  if (BanDecltypeAuto) {
    Finder->addMatcher(DecltypeAutoVarDecl, this);
  }
}

void AvoidAutoForEigenCheck::check(const MatchFinder::MatchResult &Result) {
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

  // Check if this is a range-based for loop variable and we allow it
  if (AllowInRangeFor) {
    // Check if the variable is declared in a range-based for loop
    // We need to look at the parent statements to find a CXXForRangeStmt
    auto Parents = Result.Context->getParents(*VD);
    for (const auto &Parent : Parents) {
      if (Parent.get<CXXForRangeStmt>()) {
        return;
      }
    }
  }

  // Verify the variable was actually spelled with auto/decltype(auto)
  TypeSourceInfo *TSI = VD->getTypeSourceInfo();
  if (!TSI) {
    return;
  }

  // Check if this variable's type was deduced from auto
  QualType VarType = VD->getType();
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

  // Check if the initializer is a call to .eval() - this should be allowed
  if (const Expr *Init = VD->getInit()) {
    if (const auto *Call = dyn_cast<CXXMemberCallExpr>(Init)) {
      if (const auto *Method = dyn_cast<CXXMethodDecl>(Call->getDirectCallee())) {
        if (Method->getName() == "eval") {
          return; // Allow auto with .eval() calls
        }
      }
    }
  }

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

  auto Diag = diag(VD->getLocation(),
                   "do not use '%0' for Eigen types or expressions; declare an "
                   "explicit Eigen type or assign the whole expression to a "
                   "concrete type (e.g., (expr).eval() into Eigen::Matrix<>). "
                   "See Eigen pitfalls: "
                   "https://libeigen.gitlab.io/eigen/docs-nightly/TopicPitfalls.html")
              << TypeKeyword;

  // Highlight the auto keyword
  Diag << SourceRange(TL.getBeginLoc(), TL.getEndLoc());
}

bool AvoidAutoForEigenCheck::isEigenType(const Type *Ty) const {
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

bool AvoidAutoForEigenCheck::isInEigenNamespace(const CXXRecordDecl *RD) const {
  if (!RD) {
    return false;
  }

  // Walk up the declaration context chain looking for the Eigen namespace
  const DeclContext *DC = RD->getDeclContext();
  while (DC) {
    if (const auto *ND = dyn_cast<NamespaceDecl>(DC)) {
      if (ND->getName() == "Eigen") {
        // Check if this is the top-level Eigen namespace (not nested in another namespace)
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

bool AvoidAutoForEigenCheck::isEigenExpression(const Type *Ty) const {
  if (!isEigenType(Ty)) {
    return false;
  }

  // Heuristic: Expression templates typically don't derive from PlainObjectBase
  return !isEigenPlainObject(Ty);
}

bool AvoidAutoForEigenCheck::isEigenPlainObject(const Type *Ty) const {
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
  // This is a heuristic - we look for inheritance from classes named PlainObjectBase
  // in the Eigen namespace
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
    if (ClassName.starts_with("Matrix") || ClassName.starts_with("Array") ||
        ClassName.starts_with("Vector") || ClassName == "Quaternion" ||
        ClassName == "AngleAxis" || ClassName == "Transform") {
      return true;
    }
  }

  return false;
}

} // namespace eigen
} // namespace tidy
} // namespace clang
