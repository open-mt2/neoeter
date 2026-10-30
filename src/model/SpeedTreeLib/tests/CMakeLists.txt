add_executable(test_speedtreelib test_speedtreelib.cpp)

target_link_libraries(test_speedtreelib
    PRIVATE
        SpeedTreeLib
)

add_test(NAME SpeedTreeLib_SmokeTests COMMAND test_speedtreelib)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_speedtreelib)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
