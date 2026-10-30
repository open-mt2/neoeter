# src/SphereLib/tests/CMakeLists.txt

add_executable(test_sphere test_sphere.cpp)
target_link_libraries(test_sphere PRIVATE SphereLib)
add_test(NAME test_sphere COMMAND test_sphere)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_sphere)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
