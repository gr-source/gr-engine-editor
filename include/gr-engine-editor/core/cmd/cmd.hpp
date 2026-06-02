#pragma once

#include <functional>
#include <vector>
#include <string>

namespace Command
{
    bool exec(const std::string &cmd, std::function<void(std::string)> out);

    bool exec(const std::vector<std::string> &list, std::function<void(std::string)> out = nullptr);
};




