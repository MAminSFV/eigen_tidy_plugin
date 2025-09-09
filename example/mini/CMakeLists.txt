cmake_minimum_required(VERSION 3.16)
project(eigen-example)

set(CMAKE_CXX_STANDARD 17)

# Find Eigen (optional - if available)
find_package(Eigen3 QUIET)

if(Eigen3_FOUND)
    message(STATUS "Found Eigen3: ${Eigen3_VERSION}")
    add_executable(example main.cpp)
    target_link_libraries(example Eigen3::Eigen)
else()
    message(WARNING "Eigen3 not found. Example will use mock Eigen types.")
    add_executable(example main.cpp)
    target_compile_definitions(example PRIVATE MOCK_EIGEN)
endif()

# Custom target to run clang-tidy on the example
add_custom_target(check-example
    COMMAND clang-tidy
        -load ${CMAKE_BINARY_DIR}/clang-tidy-eigen.so
        -checks='-*,eigen-avoid-auto'
        -header-filter='.*'
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        --
        -std=c++17
        -I${CMAKE_CURRENT_SOURCE_DIR}
        $<$<BOOL:${Eigen3_FOUND}>:-I${EIGEN3_INCLUDE_DIR}>
    DEPENDS clang-tidy-eigen
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-tidy eigen-avoid-auto check on example"
)
