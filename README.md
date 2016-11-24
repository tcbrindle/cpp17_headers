
# C++17 Headers for C++11/14 #

This repository features self-contained single-header implementations of some of
the new facilities which will be added to the standard library in C++17, namely
`std::any`, `std::optional`, `std::string_view` and `std::variant`.

The intention is to make it easy for people to get started with the new libraries
*now*, without having to wait for newer compiler versions. The headers are all
backwards-compatible to C++11, except for `variant.hpp` which is C++14 only.

I am not the original author of any of these headers; my involvement extends to
putting them together in one place, changing namespaces and adding a little bit
of preprocessor code to prefer standard library versions when available. All credit
should go to the  original authors, whose details can be found below.

## Usage ##

The recommended way to use these headers is to simply copy the ones you need
into your own sources. (If you find this distasteful, and instead wish to add a dependency
on this repo in your project, please consider whether you should add a dependency
on Boost instead.) Each header is self-contained, and they can all be used
independently. 

These headers all use namespace `stx` ("standard extensions")
by default. This can be customised by defining the symbol `STX_NAMESPACE NAME` to
a suitable string before including the header, for example

```cpp
#define STX_NAMESPACE_NAME myns
#include <stx/any.hpp>

myns::any i = 3;
```

### Forward compatibility ###

Eventually, and hopefully in the not-too-distant future, all of these headers will
be available as part of every standard library implementation. Many of them are
already available in the `std::experimental` namespace in some compilers. For
forward compatibility, these headers will try to use standard library versions
if possible, and only fall back to the included implementations of this fails.

On compilers which implement `__has_include()` (only GCC and Clang at the time of
writing), each header `N.hpp` will first look for the standard library header
`<N>`; if this is found, the names will be made available in the configured
namespace via `using` declarations.  If `<N>` cannot be found, it will look for
`<experimental/N>`, before finally back to the included implementation. So
for example, GCC 6.1 includes an implementation of `std::experimental::any`;
therefore

```cpp
#include <stx/any.hpp>

static_assert(std::is_same<stx::any, std::experimental::any>::value, "");
```

will pass on that system. The objective is to provide a seamless upgrade path
to standard library facilities as they are included with compiler releases,
eventually rendering this repo redundant.

If, for ABI reasons, you'd rather stick with the versions defined in this header
even if a newer standard library implementation provides a particular header, then
define the preprocessor symbol `STX_NO_STD_N`, where `N` is `ANY`, `OPTIONAL` etc
before including the header. This will short-circuit the `__has_include()` check.

*As an aside: if anybody knows of a way to emulate `__has_include()` on other
compilers, particularly MSVC, please let me know.*

## ...or use Boost ##
 
All of these headers (with the exception of `variant.hpp`, which is quite
different from Boost.Variant) are available in (near-)identical form as part of
Boost. The intention of this repo is to provide these facilities to people who are
unwilling to (or cannot) add a dependency on Boost. If this situation does not
describe you, *please* consider using the Boost versions instead.

## Headers ##

### any.hpp ###

 * Author: Denilson das Mercês Amorim
 * Repository: https://github.com/thelink2012/any
 * Licence: Boost 
 * Compatibility: C++11
 
 
### optional.hpp ###

 * Author: Andrzej Krzemieński
 * Repository: https://github.com/akrzemi1/Optional
 * Licence: Boost
 * Compatibility: C++11
 
 ### string_view.hpp ###
 
 * Authors: Marshall Clow and Beman Dawes (Boost)
 * Repository: https://github.com/boostorg/utility
 * Licence: Boost
 * Compatibility: C++11
 
### variant.hpp ###

 * Author: Anthony Williams
 * Repository: https://bitbucket.org/anthonyw/variant (Mercurial)
 * Licence: BSD
 * Compatibility: C++14
