#pragma once

#include <string>
#include <stdexcept>

class Exception : public std::exception {
public:
    explicit Exception(const std::string& message) : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};

