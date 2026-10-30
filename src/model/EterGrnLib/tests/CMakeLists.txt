add_executable(test_etergrnlib test_etergrnlib.cpp)

target_link_libraries(test_etergrnlib
    PRIVATE
        EterGrnLib
)

add_test(NAME EterGrnLib_SmokeTests COMMAND test_etergrnlib)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_etergrnlib)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
