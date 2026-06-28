#pragma once

#include "view/viewlayout.hpp"

#include <unordered_map>
#include <typeinfo>
#include <vector>
#include <memory>

class ViewManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T>
    static T* Register();

    static ViewLayout* GetCurrentView()
    {
        return s_ViewManager->s_current;
    }

    static void Update();

    static void Renderer3D();

    static void RendererUI();

    static void RemoveAllView();

    template <typename T, typename... Args>
    static void AddView(Args&&... args);

    template <typename T>
    static T* GetView();

    template <typename T>
    static void RemoveView();

    template <typename T>
    static bool IsVisible();

    template <typename T>
    static void SetVisible(bool value);

private:
    static ViewManager* s_ViewManager;

    std::unordered_map<ViewHash, std::shared_ptr<ViewLayout>> s_ViewMap;

    std::vector<ViewLayout*> s_viewRegistryPopup;

    std::vector<ViewLayout*> s_viewRegistry;

    ViewLayout* s_current;

    ViewManager();
    ~ViewManager();

    ViewLayout* addView(ViewHash hash, std::shared_ptr<ViewLayout> view);

    ViewLayout* addSubView(ViewHash hash, std::shared_ptr<ViewLayout> view);

    ViewLayout* getView(ViewHash hash);

    void remove(ViewHash hash);

    void removeAllView();

    void update();

    void renderer3D();

    void rendererUI();

    void render_tabbar();
};


template <typename T>
inline T* ViewManager::Register()
{
    return static_cast<T*>(s_ViewManager->addSubView(typeid(T).hash_code(), std::make_shared<T>()));
}

template <typename T, typename... Args>
inline void ViewManager::AddView(Args&&... args)
{
    if (s_ViewManager->GetView<T>() != nullptr)
        return;
    s_ViewManager->addView(typeid(T).hash_code(), std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T>
inline T* ViewManager::GetView()
{
    return static_cast<T*>(s_ViewManager->getView(typeid(T).hash_code()));
}

template <typename T>
inline void ViewManager::RemoveView()
{
    s_ViewManager->remove(typeid(T).hash_code());
}

template <typename T>
inline bool ViewManager::IsVisible()
{
    auto view = s_ViewManager->getView(typeid(T).hash_code());

    return view != nullptr ? view->visible : false;
}

template <typename T>
void ViewManager::SetVisible(bool value)
{
    auto view = s_ViewManager->getView(typeid(T).hash_code());
    if (view != nullptr)
        view->visible = value;
}


