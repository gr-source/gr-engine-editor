#pragma once

#include "view/ipopupView.hpp"

#include "gr-base/pool/objectPool.hpp"

#include <string>

struct LoadingTaskInfo
{
    std::string name;
    std::string message;
};

class LoadingTaskPopup : public IPopupView
{
public:
    static Handle Create(const std::string& title, const std::string& message);

    static void Destroy(Handle handle);

    LoadingTaskPopup();
    ~LoadingTaskPopup() override;

private:
    static LoadingTaskPopup* s_LoadingTaskPopup;

    ObjectPool<LoadingTaskInfo> m_tasks;

    Handle create(const std::string& title, const std::string& message);

    void destroy(Handle handle);

    void Draw() override;
};


