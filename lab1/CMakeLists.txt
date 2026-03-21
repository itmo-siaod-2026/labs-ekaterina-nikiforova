project(Lab1)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})

# ========== БИБЛИОТЕКИ ==========
set(EXTENDIBLE_LIB extendible_hash-lib)
set(PERFECT_LIB perfect_hash-lib)
set(MINLSH_LIB min_lsh_hash-lib)

add_library(${EXTENDIBLE_LIB} src/extendible_hash.cpp)
add_library(${PERFECT_LIB} src/perfect_hash.cpp)
add_library(${MINLSH_LIB} src/min_lsh_hash.cpp)

# ========== ТЕСТЫ ==========
add_executable(extendible_hash_tests
        tests/extendible_hash_tests.cpp
)
target_link_libraries(extendible_hash_tests
        ${EXTENDIBLE_LIB}
        GTest::gtest
        GTest::gtest_main
)

add_executable(extendible_bench_tests
        tests/extandible_bench_tests.cpp
)
target_link_libraries(extendible_bench_tests
        ${EXTENDIBLE_LIB}
)

add_executable(perfect_hash-test
        tests/perfect_hash_tests.cpp
)
target_link_libraries(perfect_hash-test
        ${PERFECT_LIB}
        GTest::gtest
        GTest::gtest_main
)

add_executable(perfect_bench_tests
        tests/perfect_bench_tests.cpp
)
target_link_libraries(perfect_bench_tests
        ${PERFECT_LIB}
)

add_executable(min_hash-test
        tests/min_hash_tests.cpp
)
target_link_libraries(min_hash-test
        ${MINLSH_LIB}
        GTest::gtest
        GTest::gtest_main
)

