
#include <stx/any.hpp>

#include <cassert>

/* This is just a basic test ensuring that we can instantiate the type and
 * that it behaves as expected. It is used to ensure that the std overrides
 * are working correctly */

int main()
{
    stx::any a{3};
    assert(a.type() == typeid(int));
    assert(stx::any_cast<int>(a) == 3);

    a = 3.0f;
    assert(a.type() == typeid(float));
    bool exception_caught = false;
    try {
        stx::any_cast<int>(a);
    } catch (const stx::bad_any_cast& e) {
        exception_caught = true;
    }
    assert(exception_caught);
}