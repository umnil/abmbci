#ifndef INCLUDE_CHANNEL_INFO_HPP_
#define INCLUDE_CHANNEL_INFO_HPP_
#define UNICODE
#include <windows.h>
#include "sdk/sdk.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

#define STRING_GETTER(cls, prop, T) [](cls const& x){ return T(x.prop); }
#define STRING_SETTER(cls, prop, T, max) [](cls& x, T const& val){ \
    std::memset(x.prop, 0, max);\
    auto end = val.end(); \
    if (val.size() > max) end = val.begin() + (max - 1);\
    std::copy(val.begin(), end, x.prop);\
}
#define STRING_GETTER_SETTER(cls, prop, T, max) STRING_GETTER(cls, prop, T), STRING_SETTER(cls, prop, T, max)

#endif  /* INCLUDE_CHANNEL_INFO_HPP_ */