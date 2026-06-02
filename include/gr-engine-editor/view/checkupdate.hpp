#pragma once

#include "view/viewlayout.hpp"

#include <functional>
#include <string>

class CheckUpdate : public ViewLayout
{
public:
    template <typename... Args>
    CheckUpdate(Args&&... args);

private:
    std::function<void ()> m_function;

    inline const char* getName() const override
    {
        return "Check Update";
    }

    void init();

    void onRenderUI() override;

    void doInBackground();

    bool copydata(const std::string& src_dir);

    uint64_t GetHashFromFile(const std::string& file);
};


template <typename... Args>
inline CheckUpdate::CheckUpdate(Args&&... args) : m_function(std::bind(std::forward<Args>(args)...))
{
    init();
}


