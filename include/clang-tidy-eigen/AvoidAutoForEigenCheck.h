#ifndef CLANG_TIDY_EIGEN_AVOID_AUTO_FOR_EIGEN_CHECK_H
#define CLANG_TIDY_EIGEN_AVOID_AUTO_FOR_EIGEN_CHECK_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang-tidy/ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace eigen {

/// Diagnoses usage of 'auto' for variables whose deduced type belongs to the
/// Eigen namespace, including both plain objects and expression templates.
///
/// This check helps avoid common pitfalls with Eigen expression templates
/// where 'auto' can lead to performance issues or incorrect behavior due to
/// storing references to temporaries.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/eigen-avoid-auto.html
class AvoidAutoForEigenCheck : public ClangTidyCheck {
public:
  AvoidAutoForEigenCheck(StringRef Name, ClangTidyContext *Context);

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

} // namespace eigen
} // namespace tidy
} // namespace clang

#endif // CLANG_TIDY_EIGEN_AVOID_AUTO_FOR_EIGEN_CHECK_H
