add_executable(test_mileslib
    test_mileslib.cpp
)

target_link_libraries(test_mileslib
    PRIVATE
        MilesLib
)

add_test(NAME MilesLib_SmokeTests COMMAND test_mileslib)

# TA.1: shared test helpers and always-on asserts (tests/support/TestAssert.h)
foreach(_test_target test_mileslib)
    target_include_directories(${_test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${_test_target} PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-UNDEBUG>)
endforeach()
