add_executable(test_mileslib
    test_mileslib.cpp
)

target_link_libraries(test_mileslib
    PRIVATE
        MilesLib
)

add_test(NAME MilesLib_SmokeTests COMMAND test_mileslib)
