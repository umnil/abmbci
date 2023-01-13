#ifndef INCLUDE_ELECTRODES_INFO_HPP_
#define INCLUDE_ELECTRODES_INFO_HPP_

#define LIST_STRING_GETTER(cls, prop, T) \
[](cls const& x) { \
    std::vector<T> strings_list; \
    for (auto str : x.prop) { \
        strings_list.push_back(str); \
    } \
    return strings_list; \
}

#define LIST_STRING_SETTER(cls, prop, max, T) \
[](cls& x, std::vector<T> const& val) { \
    for (uint32_t i = 0; i < max; i++) { \
        std::memset(x.prop[i], 0, max); \
        if (i < val.size()) { \
            auto end = (val[i].size() < max) ? val[i].end() : val[i].begin() + (max - 1); \
            std::copy(val[i].begin(), end, x.prop[i]); \
        } \
    } \
}

#define LIST_GETTER(cls, prop, max, T) [](cls const& x) { return std::vector<T>(&x.prop[0], &x.prop[max]); }
#define LIST_SETTER(cls, prop, max, T) [](cls& x, std::vector<T> const& val) {\
    std::memset(x.prop, 0, max);\
    auto end = val.end(); \
    if (val.size() < max) {\
        end = val.begin() + (max - 1);\
    }\
    std::copy(val.begin(), end, x.prop);\
}

#endif  /* INCLUDE_ELECTRODES_INFO_HPP_ */