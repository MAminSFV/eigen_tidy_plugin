#include "clang-tidy-eigen/AvoidAutoForEigenCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"

namespace clang {
namespace tidy {
namespace eigen {

class EigenModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AvoidAutoForEigenCheck>("eigen-avoid-auto");
  }
};

} // namespace eigen
} // namespace tidy
} // namespace clang

// Register the module
static clang::tidy::ClangTidyModuleRegistry::Add<clang::tidy::eigen::EigenModule>
    X("eigen-module", "Adds Eigen-related clang-tidy checks.");
