# src/EterPack/tests/CMakeLists.txt

add_executable(test_eterpack test_eterpack.cpp)
target_link_libraries(test_eterpack PRIVATE EterPack)
add_test(NAME test_eterpack COMMAND test_eterpack)
