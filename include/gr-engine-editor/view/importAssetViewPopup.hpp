#pragma once

#include "event/importAssetViewPopupEvent.hpp"

#include "view/ipopupView.hpp"

class ImportAssetViewPopup : public IPopupView
{
public:
    ImportAssetViewPopup();

private:
    std::vector<FbxScene> m_scenes;

    void Draw() override;

    void onAddFbxScene(ImportAssetViewPopup_onAddFbxScene* event);
};


