
#include <stx/optional.hpp>
#include <cassert>

/*
 * This is a very basic test to ensure we can instantiate the type and link
 * correctly. It is used to ensure that the std overrides are working correctly.
 */

int main()
{
    constexpr stx::optional<int> empty{};

    static_assert(!empty, "");

    bool caught_exception = false;

    try {
        empty.value();
    } catch (stx::bad_optional_access e) { // catch by value to test copy constructor
        caught_exception = true;
    }

    assert(caught_exception);

    constexpr stx::optional<int> opt{3};
    assert(opt);
    static_assert(opt.value() == 3, "");
}