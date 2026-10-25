add_executable(test_etergrnlib test_etergrnlib.cpp)

target_link_libraries(test_etergrnlib
    PRIVATE
        EterGrnLib
)

add_test(NAME EterGrnLib_SmokeTests COMMAND test_etergrnlib)
