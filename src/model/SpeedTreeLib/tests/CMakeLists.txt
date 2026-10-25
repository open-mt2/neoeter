add_executable(test_speedtreelib test_speedtreelib.cpp)

target_link_libraries(test_speedtreelib
    PRIVATE
        SpeedTreeLib
)

add_test(NAME SpeedTreeLib_SmokeTests COMMAND test_speedtreelib)
