#pragma once

#include "view/iwindowView.hpp"

#include <cstdint>
#include <functional>
#include <string>

class CheckUpdate : public IWindowView
{
public:
    CheckUpdate();

    void addFile(const std::string& file);

    void taskFinished();

private:
    std::vector<std::string> m_files;

    void Draw() override;
};



