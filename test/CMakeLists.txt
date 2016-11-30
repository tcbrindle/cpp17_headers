
add_executable(test_any test_any.cpp)
add_executable(test_optional test_optional.cpp)
add_executable(test_string_view string_view_test1.cpp)
if (NOT MSVC)
    set(CMAKE_CXX_STANDARD 14)
    add_executable(test_variant test_variant.cpp)
    set(CMAKE_CXX_STANDARD 11)
endif()