project(Lab1)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})

# ========== БИБЛИОТЕКИ ==========
set(QUADTREE_LIB quadtree_lib)

add_library(${QUADTREE_LIB} src/quadtree.cpp)

# ========== ТЕСТЫ ==========
add_executable(quadtree_tests
        tests/quadtree_tests.cpp
)
target_link_libraries(quadtree_tests
        ${QUADTREE_LIB}
        GTest::gtest
        GTest::gtest_main
)

add_executable(quadtree_bench_tests
        tests/quadtree_bench_tests.cpp
)
