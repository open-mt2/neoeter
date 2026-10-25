# src/SphereLib/tests/CMakeLists.txt

add_executable(test_sphere test_sphere.cpp)
target_link_libraries(test_sphere PRIVATE SphereLib)
add_test(NAME test_sphere COMMAND test_sphere)
