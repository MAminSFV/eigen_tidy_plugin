#pragma once

#include "clang-tidy/ClangTidyCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace clang::tidy::eigen {

class AvoidAutoCheck : public ClangTidyCheck {
public:
  AvoidAutoCheck(StringRef Name, ClangTidyContext *Context);

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  /// Check if a type belongs to the Eigen namespace
  bool isEigenType(const Type *Ty) const;

  /// Check if a record declaration is within the Eigen namespace
  bool isInEigenNamespace(const CXXRecordDecl *RD) const;

  /// Check if a type is an Eigen expression template (heuristic)
  bool isEigenExpression(const Type *Ty) const;

  /// Check if a type is a plain Eigen object (derives from PlainObjectBase)
  bool isEigenPlainObject(const Type *Ty) const;

  /// Configuration options
  bool AllowInRangeFor;
  bool OnlyExpressions;
  bool BanDecltypeAuto;
};

} // namespace clang::tidy::eigen
