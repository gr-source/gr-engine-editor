#pragma once

#include "view/windowView.hpp"

#include "gr-base/asset/URI.hpp"

#include "gr-base/asset/assetDB.hpp"
#include "gr-base/asset/assetPtr.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/asset/default.hpp"

#include "imgui.h"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <unordered_set>
#include <memory>

struct FileInfo
{
    std::string name;
    std::string path;

    AssetID assetID = INVALID_ASSET;

    std::vector<std::shared_ptr<FileInfo>> children;

    bool leaf() const
    {
        return children.empty();
    }
};

class BrowserFile : public WindowView
{
public:
    BrowserFile();

    inline const char* GetTitle() const override
    {
        return "Browser File";
    }

private:
    struct EntryInfo
    {
        std::string name;
        std::string path;
    };

    std::unordered_map<uint64_t, FileInfo*> m_hashmap;

    std::shared_ptr<FileInfo> m_root;

    FileInfo* m_selected_dir;

    AssetPtr<gr::texture> m_fileTexture;

    std::unordered_set<AssetID> m_selected_assets;

    ImGuiIO* m_imguiIO;

    void Draw() override;

    void renderer();

    void render_virtual_directories(FileInfo* info);

    void drop_source_in_directories();

    void render_virtual_entries();

    void drag_selected_entries();

    void onAssetLoadedDB(IEvent* event);

    void onAssetCreated(IEvent* data);

    void onAssetRemove(IEvent* data);

    std::vector<EntryInfo> splitPath(const std::string& path);

    void create_hierarchy(AssetInfo* info);

    void create_render_popup_asset_info();
};


