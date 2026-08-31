#pragma once

#include "view/iwindowView.hpp"

#include "gr-base/core/console.hpp"

#include <cstring>
#include <vector>
#include <mutex>

class ConsoleWindowView : public IWindowView
{
public:
    ConsoleWindowView();

private:
    struct TextBuffer
    {
        std::vector<size_t> lineOffsets = { 0 };
        std::vector<char> buffer;

        template <typename... Args>
        void append(const char* fmt, Args&&...args)
        {
            size_t offset = buffer.size() > 0 ? buffer.size() : 1;

            if constexpr (sizeof...(Args) > 0)
            {
                int length = snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
                if (length <= 0)
                    return;
                buffer.resize(offset + length);

                snprintf(&buffer[offset - 1], length + 1, fmt, std::forward<Args>(args)...);
            } else
            {
                int length = strlen(fmt);

                buffer.resize(offset + length);

                memcpy(&buffer[offset - 1], fmt, length + 1);
            }

            for (size_t i = offset; i < buffer.size(); i++)
                if (buffer[i] == '\n')
                    lineOffsets.push_back(i + 1);
        }

        const char* begin() const
        {
            return buffer.data();
        }

        const char* end() const
        {
            return buffer.size() > 0 ? (buffer.data() + buffer.size() - 1) : buffer.data();
        }
    };

    // std::deque<std::string> m_messages;

    TextBuffer gTextBuffer;

    bool gAutoScrool = true;

    std::mutex m_mutex;

    void Draw() override;

    void draw_menu();

    void draw_child();

    void show(const char* line_start, const char* line_end);

    void onAddMessage(Console_onAddMessage& event);
};

