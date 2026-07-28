#pragma once

#include "gr-base/core/console.hpp"
#include "view/windowView.hpp"

#include <list>
#include <mutex>

class ConsoleWindowView : public WindowView
{
public:
    ConsoleWindowView();

private:
    std::list<Log> m_logs;

    std::mutex m_mutex;

    bool refresh = false;

    void Draw() override;

    void render();

    void show(const Log* log);

    void onAddMessage(Console_onAddMessage* event);
};

