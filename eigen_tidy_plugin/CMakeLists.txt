# Create the plugin as a MODULE library
add_library(eigen_tidy_plugin MODULE
    avoid_auto_check.cpp
    eigen_module.cpp
)

# Link against required libraries
target_link_libraries(eigen_tidy_plugin
    clangTidy
    clangTidyUtils
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangEdit
    clangAnalysis
    clangRewrite
)

# Ensure the plugin doesn't have a lib prefix and uses .so extension
set_target_properties(eigen_tidy_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)
