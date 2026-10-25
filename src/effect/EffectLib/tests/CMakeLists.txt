add_executable(test_effectlib test_effectlib.cpp)

target_link_libraries(test_effectlib
    PRIVATE
        EffectLib
)

add_test(NAME EffectLib_SmokeTests COMMAND test_effectlib)
