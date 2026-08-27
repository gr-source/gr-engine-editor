#pragma once

#include <gr-base/core/default.hpp>

#include <gr-render/texture.hpp>

#include <unordered_set>
#include <algorithm>

typedef uint64_t Entity;

struct ImGuiIO;
class Scene;

enum IconType
{
    ICON_FOLDER         = 0,
    ICON_FILE           = 1,
    ICON_BACK_FILE      = 2,
    ICON_GAMEOBJECT     = 3,
	ICON_PENCIL         = 4,
	ICON_LIGHT          = 5,
	ICON_TEXTURE        = 6,
	ICON_PNG            = 7,
	ICON_PLAY_GAME      = 8,
	ICON_STOP_GAME      = 9,
	ICON_BIN            = 10,
	ICON_IMPORT         = 11,
	ICON_ASCENDENTE     = 12,
	ICON_DESCENDENTE    = 13,
	ICON_EDIT           = 14,
	ICON_ANIMATION      = 15,
	ICON_SHADER         = 16,
	ICON_SCENE          = 17,
	ICON_SCRIPT         = 18,
    ICON_COUNT
};

#include <unordered_set>
#include <vector>

template <typename T>
struct Selector
{
    std::unordered_set<T> map;
    std::vector<T> list;

    void select(T value)
    {
        if (map.find(value) != map.end())
            return;

        map.emplace(value);
        list.push_back(value);
    }

    void deselect(T value)
    {
        list.erase(std::remove(list.begin(), list.end(), value), list.end());
        map.erase(value);
    }

    void deselect_all()
    {
        list.clear();
        map.clear();
    }

    bool empty() const
    {
        return list.empty();
    }

    std::vector<T> getSelected() const
    {
        return list;
    }

    bool has(T value) const
    {
        return map.find(value) != map.end();
    }
};

struct EditorContext
{
    TextureRef iconsUI[IconType::ICON_COUNT];

    Selector<Entity> selector;

    Scene* scene = nullptr;
    ImGuiIO* io = nullptr;
};


