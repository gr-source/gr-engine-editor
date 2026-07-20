#pragma once

#include "view/windowView.hpp"
#include "view/ipopupView.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>
#include <vector>

typedef uint32_t ViewID;

class ViewManager
{
public:
    ViewManager();
    ~ViewManager();

    void renderer();

    template <typename T>
    void show();

    template <typename T>
    void hide();

    template <typename T>
    bool isVisible() const;

    template <typename T>
    void setVisible(bool value);

    void clear();

private:
    std::vector<std::shared_ptr<WindowView>> m_windows;

    std::vector<std::shared_ptr<IPopupView>> m_popups;

    inline static ViewID GenViewID() noexcept
    {
        static ViewID nextID = 0;
        return nextID++;
    }

    template <typename T>
    inline static ViewID GetViewID() noexcept
    {
        static ViewID id = GenViewID();
        return id;
    }
};

template <typename T>
inline void ViewManager::show()
{
    static_assert(std::is_base_of_v<WindowView, T> || std::is_base_of_v<IPopupView, T>, 
              "This object is not valid.");   

    ViewID id = GetViewID<T>();
    if constexpr (std::is_base_of_v<IPopupView, T>)
    {
        if (id >= m_popups.size())
            m_popups.resize(id + 1, nullptr);

        if (m_popups[id] == nullptr)
            m_popups[id] = std::make_shared<T>();

        m_popups[id]->requestOpen = true;
    } else if constexpr (std::is_base_of_v<WindowView, T>)
    {
        if (id >= m_windows.size())
            m_windows.resize(id + 1, nullptr);

        if (m_windows[id] == nullptr)
            m_windows[id] = std::make_shared<T>();

        m_windows[id]->open = true;
    }
}

template <typename T>
inline void ViewManager::hide()
{
    static_assert(std::is_base_of_v<WindowView, T> || std::is_base_of_v<IPopupView, T>, 
              "This object is not valid.");   

    ViewID id = GetViewID<T>();
    if constexpr (std::is_base_of_v<IPopupView, T>)
    {
        if (id >= m_popups.size())
            m_popups.resize(id + 1, nullptr);

        if (m_popups[id] != nullptr)
            m_popups[id]->requestOpen = false;

    } else if constexpr (std::is_base_of_v<WindowView, T>)
    {
        if (id >= m_windows.size())
            m_windows.resize(id + 1, nullptr);

        if (m_windows[id] != nullptr)
            m_windows[id]->open = false;
    }
}

template <typename T>
inline bool ViewManager::isVisible() const
{
    static_assert(std::is_base_of_v<WindowView, T> || std::is_base_of_v<IPopupView, T>, 
              "This object is not valid.");   

    ViewID id = GetViewID<T>();
    if constexpr (std::is_base_of_v<IPopupView, T>)
    {
        if (id >= m_popups.size())
            return false;
        if (m_popups[id] == nullptr)
            return false;
        return m_popups[id]->requestOpen;
    } else if constexpr (std::is_base_of_v<WindowView, T>)
    {
        if (id >= m_windows.size())
            return false;
        if (m_windows[id] == nullptr)
            return false;
        return m_windows[id]->open;
    }
}

template <typename T>
inline void ViewManager::setVisible(bool value)
{
    static_assert(std::is_base_of_v<WindowView, T> || std::is_base_of_v<IPopupView, T>, 
              "This object is not valid.");

    if (value)
        show<T>();
    else
        hide<T>();
}
