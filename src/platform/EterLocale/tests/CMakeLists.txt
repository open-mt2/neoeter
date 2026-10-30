# src/EterLocale/tests/CMakeLists.txt

add_executable(test_locale test_locale.cpp)
target_link_libraries(test_locale PRIVATE EterLocale)
add_test(NAME test_locale COMMAND test_locale)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_locale)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
