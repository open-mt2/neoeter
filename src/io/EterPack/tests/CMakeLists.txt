# src/EterPack/tests/CMakeLists.txt

add_executable(test_eterpack test_eterpack.cpp)
target_link_libraries(test_eterpack PRIVATE EterPack)
add_test(NAME test_eterpack COMMAND test_eterpack)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_eterpack)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
