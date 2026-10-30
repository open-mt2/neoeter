# src/EterImageLib/tests/CMakeLists.txt

add_executable(test_image test_image.cpp)
target_link_libraries(test_image PRIVATE EterImageLib)
add_test(NAME test_image COMMAND test_image)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_image)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
