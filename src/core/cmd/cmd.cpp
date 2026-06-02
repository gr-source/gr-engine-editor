#include "core/cmd/cmd.hpp"

#include <gr-base/core/console.hpp>

#include <stdio.h>

bool Command::exec(const std::string &cmd, std::function<void(std::string)> out)
{
    FILE *pipe = popen(std::string(cmd + " 2>&1").c_str(), "r");

    if (pipe == nullptr)
        return false;

    char line[256] = {0};
    while (fgets(line, 256, pipe))
        out(line);

    int status = pclose(pipe);

    return status == -1 ? false : WIFEXITED(status) && (WEXITSTATUS(status) == 0);
}

#include <filesystem>
#include <iostream>
#include <fstream>
bool Command::exec(const std::vector<std::string> &list, std::function<void(std::string)> out) {
    std::string temp_file = std::filesystem::temp_directory_path() / "command.sh";

    std::ofstream os(temp_file);
    for (auto &cmd : list) {
        os << cmd << std::endl;
    }
    os.close();

    auto file = popen(std::string("sh " + temp_file + " 2>&1").c_str(), "r");
    if (!file) {
        return false;
    }

    if (out) {
        char line[1024] = {0};
        while (fgets(line, 1024, file)) {
            out(line);
        }
    }

    auto status = pclose(file);

    std::filesystem::remove(temp_file);
    
    return status != 0 ? false : true;
}


