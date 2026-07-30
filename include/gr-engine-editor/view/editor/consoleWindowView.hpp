#pragma once

#include "gr-base/core/console.hpp"
#include "view/windowView.hpp"

#include <atomic>
#include <deque>
#include <string>
#include <mutex>
#include <list>

class ConsoleWindowView : public WindowView
{
public:
    ConsoleWindowView();

private:
    std::deque<std::string> m_messages;

    std::atomic<bool> bScroolConsole = false;

    std::mutex m_mutex;

    void Draw() override;

    void render();

    void show(const char* message);

    void onAddMessage(Console_onAddMessage* event);
};

