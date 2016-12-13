
function(stx_test_case name file)
    add_executable(${name} ${file})
    add_test(${name} ${name})
endfunction()

function(stx_test_case_override name file)
    add_executable(${name} ${file})
    target_compile_definitions(${name} PRIVATE
        -DSTX_NO_STD_ANY
        -DSTX_NO_STD_OPTIONAL
        -DSTX_NO_STD_STRING_VIEW
        -DSTX_NO_STD_VARIANT
        )
    add_test(${name} ${name})
endfunction()

# C++11 Tests
set(CMAKE_CXX_STANDARD 11)
stx_test_case(test_any_basic11 test_any_basic.cpp)
stx_test_case_override(test_any_basic11o test_any_basic.cpp)

stx_test_case_override(test_any11o test_any.cpp)

stx_test_case(test_optional_basic11 test_optional_basic.cpp)
stx_test_case_override(test_optional_basic11o test_optional_basic.cpp)

stx_test_case_override(test_optional11o test_optional.cpp)

stx_test_case(test_string_view11 string_view_test1.cpp)
stx_test_case_override(test_string_view11o string_view_test1.cpp)

# C++14 tests
set(CMAKE_CXX_STANDARD 14)
stx_test_case(test_any_basic14 test_any_basic.cpp)
stx_test_case_override(test_any_basic14o test_any_basic.cpp)

stx_test_case_override(test_any14o test_any.cpp)

stx_test_case(test_optional_basic14 test_optional_basic.cpp)
stx_test_case_override(test_optional_basic14o test_optional_basic.cpp)

stx_test_case_override(test_optional14o test_optional.cpp)

stx_test_case(test_string_view14 string_view_test1.cpp)
stx_test_case_override(test_string_view14o string_view_test1.cpp)

if (NOT MSVC)
    stx_test_case_override(test_variant14o test_variant.cpp)
    #stx_test_case_override(test_all_includes14o test_all_includes.cpp)
endif()