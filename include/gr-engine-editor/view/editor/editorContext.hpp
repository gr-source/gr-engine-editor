#pragma once

#include <gr-base/core/assetPtr.hpp>

#include <gr-render/texture.hpp>

#include <unordered_map>

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
	ICON_SCRIPT         = 18
};

struct EditorContext
{
    std::unordered_map<IconType, AssetPtr<gr::texture>> iconsUI;

    Scene* scene = nullptr;
    ImGuiIO* io = nullptr;
};


