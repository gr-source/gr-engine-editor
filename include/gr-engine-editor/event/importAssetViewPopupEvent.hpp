#pragma once

#include "gr-base/core/ievent.hpp"

#include "filmbox/fbxScene.hpp"

struct ImportAssetViewPopup_onAddFbxScene : IEvent
{
    FbxScene fbxScene;
};

