# src/EterLocale/tests/CMakeLists.txt

add_executable(test_locale test_locale.cpp)
target_link_libraries(test_locale PRIVATE EterLocale)
add_test(NAME test_locale COMMAND test_locale)
