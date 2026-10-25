# src/EterImageLib/tests/CMakeLists.txt

add_executable(test_image test_image.cpp)
target_link_libraries(test_image PRIVATE EterImageLib)
add_test(NAME test_image COMMAND test_image)
