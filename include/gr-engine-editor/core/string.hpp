#pragma once

#include <string>

class String {
public:
    static std::string RemoveNumber(const std::string &text);

    static std::string replace_all(std::string text, const std::string &lhs, const std::string &rhs);

    static bool contains(const std::string &text, const std::string &value);

    template <typename ...Args>
    static std::string format(const std::string &fmt, Args &&...args);
};

template <typename... Args>
inline std::string String::format(const std::string &fmt, Args &&...args) {
    int retVal = snprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...);
    if (retVal <= 0) {
        return "";
    }
    std::string buffer(retVal, '\0');

    snprintf(buffer.data(), buffer.size() + 1, fmt.c_str(), std::forward<Args>(args)...);
    return buffer;
}
