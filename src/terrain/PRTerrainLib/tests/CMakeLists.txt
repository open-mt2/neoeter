add_executable(test_prterrainlib test_prterrainlib.cpp)

target_link_libraries(test_prterrainlib
    PRIVATE
        PRTerrainLib
)

add_test(NAME PRTerrainLib_SmokeTests COMMAND test_prterrainlib)
