#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"

#include "AvoidAutoCheck.hpp"

namespace clang::tidy::eigen {

class EigenModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AvoidAutoCheck>("eigen-avoid-auto");
  }
};

} // namespace clang::tidy::eigen

// Register the module
static clang::tidy::ClangTidyModuleRegistry::Add<clang::tidy::eigen::EigenModule>
    X("eigen-module", "Adds Eigen-related clang-tidy checks.");
