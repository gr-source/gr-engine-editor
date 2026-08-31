#pragma once

#include "view/iwindowView.hpp"

#include "event/importAssetViewPopupEvent.hpp"

class ImportAssetViewPopup : public IWindowView
{
public:
    ImportAssetViewPopup();

private:
    std::vector<FbxScene> m_scenes;

    void Draw() override;

    void onAddFbxScene(ImportAssetViewPopup_onAddFbxScene& event);
};


