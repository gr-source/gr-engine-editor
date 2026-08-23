#pragma once

#include "view/windowView.hpp"

#include <gr-base/pool/sparseSet.hpp>

#include "gr-base/core/assetDB.hpp"
#include <gr-base/core/assetPtr.hpp>

#include "gr-base/asset/default.hpp"

#include "gr-base/asset/default.hpp"

#include "imgui.h"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

struct EditorContext;

class BrowserFile : public WindowView
{
public:
    BrowserFile(EditorContext* context);

private:

    typedef SparseID EntryID;

    enum PopupFlags
    {
        PopupFlags_ShowAssetInfo = 1 << 1,
        PopupFlags_ShowDirOption = 1 << 2
    };

    struct EntryInfo
    {
        std::string name;
        std::string path;
    };

    struct FileInfo
    {
        std::string name;
        std::string path;

        AssetID assetID = INVALID_ASSET;
        Hash64 hash = INVALID_HASH64;

        EntryID parent
            = INVALID_SPARSE_ID;

        std::vector<EntryID> children;

        bool leaf() const
        {
            return children.empty();
        }
    };

    SparseSet2<FileInfo> m_spaseSet;

    std::unordered_map<Hash64, EntryID> m_hashmap;

    EntryID entryRootID =
        INVALID_SPARSE_ID;

    EntryID entrySelectedDir =
        INVALID_SPARSE_ID;

    AssetPtr<gr::texture> m_fileTexture;

    std::unordered_set<AssetID> m_selected_assets;

    uint32_t popupFlags =
        0;

    EditorContext* context = nullptr;

    size_t startSelected = -1;

    void Draw() override;

    void renderer();

    void render_virtual_directories(EntryID entryID);

    void drop_source_in_directories();

    void render_virtual_entries(EntryID entryID);

    bool drag_selected_entry(AssetID assetID);

    void onAssetLoadedDB(AssetEventOnLoaded& event);

    void onAssetCreated(AssetEventOnCreated& event);

    void onAssetRemove(AssetEventOnRemove& event);

    std::vector<EntryInfo> splitPath(const std::string& path);

    void create_hierarchy(AssetInfo* info);

    void create_call_popup();

    void create_popup();

    void create_popup_asset_info();

    void create_popup_dir_option();

    void remove_all_assets_on_dir(EntryID entryID);
};


