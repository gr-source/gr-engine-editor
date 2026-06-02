#include "core/string.hpp"

std::string String::RemoveNumber(const std::string &text) {
    std:: string result;
    for (auto &ch : text) {
        if (std::isdigit(ch)) {
            continue;
        }
        result += ch;
    }
    return result;
}

std::string String::replace_all(std::string text, const std::string &lhs, const std::string &rhs){
    std::size_t pos = 0;
    while (true) {
        pos = text.find(lhs, pos);
        if (pos == std::string::npos) {
            break;
        }

        text = text.replace(pos, lhs.size(), rhs);
        pos += rhs.size();
    }

    return text;
}

bool String::contains(const std::string &text, const std::string &value) {
    return text.find(value) != std::string::npos;
}
