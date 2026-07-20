#include "view/browserFile.hpp"

#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/eventManager.hpp"
#include "gr-base/task/modelsavetask.hpp"
#include "view/imguiWidget.hpp"

#include <algorithm>

BrowserFile::BrowserFile() :
    m_selected_dir(nullptr),
    m_imguiIO(&ImGui::GetIO())
{
    open = true;

    m_root = std::make_shared<FileInfo>();
    m_root->name = "project://";
    m_root->path = "/";

    m_hashmap.emplace(engine::hash::fnv1_str(m_root->path), m_root.get());

    EventManager::Attach<AssetEventOnLoaded>(
            &BrowserFile::onAssetLoadedDB,
            this
        );

    EventManager::Attach<AssetEventOnCreated>(
            &BrowserFile::onAssetCreated,
            this
        );

    EventManager::Attach<AssetEventOnRemove>(
            &BrowserFile::onAssetRemove,
            this
        );
}

void BrowserFile::Draw()
{
    if (ImGui::Begin("Browser File"))
        renderer();
    ImGui::End();
}

void BrowserFile::renderer()
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("browserFile_render_directories", ImVec2(avail.x / 3.0f, 0), ImGuiChildFlags_AlwaysUseWindowPadding))
        render_virtual_directories(m_root.get());

    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::BeginChild("browserFile_render_entries", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding))
        render_virtual_entries();

    create_render_popup_asset_info();

    ImGui::EndChild();
}

void BrowserFile::render_virtual_directories(FileInfo* info)
{
    ImGuiTreeNodeFlags flags = 0;
    if (info->children.empty())
        return;

    auto& name = info->name;

    if (m_selected_dir == info)
        flags |= ImGuiTreeNodeFlags_Selected;

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);
    if (ImGui::IsItemClicked())
    {
        m_selected_dir = info;

        m_selected_assets.clear();
    }

    drop_source_in_directories();

    if (isOpen)
    {
        for (auto& child : info->children)
            render_virtual_directories(child.get());

        ImGui::TreePop();
    }
}

void BrowserFile::drop_source_in_directories()
{
    if (!ImGui::BeginDragDropTarget())
        return;

    auto scene = Scene::GetCurrent();
    
    auto payload = ImGui::AcceptDragDropPayload(".gameobject");
    if (!payload || payload->DataSize <= 0 || scene == nullptr)
        return ImGui::EndDragDropTarget();

    auto entity = *reinterpret_cast<Entity*>(payload->Data);

    GameObject gameobject(scene, entity);
    if (gameobject.IsAlive())
    {
        URI uri = URI::parse("project://model/" + gameobject.GetName());

        TaskManager::Add<ModelSaveTask>(uri, gameobject);
    }

    ImGui::EndDragDropTarget();
}

void BrowserFile::render_virtual_entries()
{
    if (m_selected_dir == nullptr)
        return;

    auto& name = m_selected_dir->name;
    ImGui::SeparatorText(name.c_str());

    ImGui::Columns(4, nullptr, false);
    for (auto& entry : m_selected_dir->children)
    {
        if (!entry->leaf())
            continue;

        auto& name = entry->name;

        bool bSelected = m_selected_assets.find(entry->assetID) != m_selected_assets.end();

        if (m_fileTexture.get() != nullptr)
        {
            ImguiWidget::ImageAndTextButton(
                    reinterpret_cast<ImTextureID>(m_fileTexture->id),
                    name.c_str(),
                    bSelected);
        } else
        {
            ImGui::Text("%s", name.c_str());
        }

        if (
            ImGui::IsItemClicked(ImGuiMouseButton_Right) &&
            (m_imguiIO->KeyShift &&
                ImGui::IsItemClicked(ImGuiMouseButton_Left))
        )
        {
            if (bSelected)
                m_selected_assets.erase(entry->assetID);
            else
                m_selected_assets.emplace(entry->assetID);
        } else if (
            ImGui::IsItemClicked(ImGuiMouseButton_Right)
        )
        {
            if (!bSelected)
                m_selected_assets.emplace(entry->assetID);
        }

        if (entry->assetID != INVALID_ASSET)
        {
            if (
                    ImGui::IsItemHovered(ImGuiMouseButton_Left) &&
                    ImGui::IsMouseDragging(ImGuiMouseButton_Left)
                )
            {
                if (!bSelected)
                    m_selected_assets.emplace(entry->assetID);
            }

            drag_selected_entries();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !m_selected_assets.empty())
            ImGui::OpenPopup("create_render_popup_asset_info");

        ImGui::NextColumn();
    }

    if (ImGui::IsWindowHovered())
    {
        if (m_imguiIO->KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
        {
            m_selected_assets.clear();

            for (auto &entry : m_selected_dir->children)
            {
                if (!entry->leaf())
                    continue;

                m_selected_assets.emplace(entry->assetID);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            m_selected_assets.clear();
    }
}

void BrowserFile::drag_selected_entries()
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceAllowNullID))
    {
        auto count =
            m_selected_assets.size() > 10 ? 10 : m_selected_assets.size();

        auto pos = ImGui::GetCursorPos();
        auto after = pos;

        for (std::size_t i={};i<count;i++)
        {
            int id = i % 2;

            ImGui::SetCursorPos(pos);
            pos.y -= 5.5f;
            pos.x = after.x + (id * 5.0f);


            ImGui::Image(reinterpret_cast<ImTextureID>(m_fileTexture->id), {32, 32});
        }

        std::vector<AssetID> assets;
        for (auto& assetID : m_selected_assets)
        {
            if (assetID != INVALID_ASSET)
                assets.push_back(assetID);
        }

        ImGui::SetDragDropPayload(
                "AssetID",
                assets.data(),
                assets.size() * sizeof(AssetID)
            );

        ImGui::EndDragDropSource();
    }
}

void BrowserFile::onAssetLoadedDB(IEvent* data)
{
    try
    {
        m_fileTexture =
            AssetsManager::LoadTexture("engine://icon/icons8-file-30.png");
    } catch (const std::exception& e)
    {
        Console::LogError("[ BrowserFile ] %s", e.what());
    }

    std::cout << "Loaded DB" << std::endl;
}

void BrowserFile::onAssetCreated(IEvent* data)
{
    auto event = static_cast<AssetEventOnCreated*>(data);

    auto info = AssetDB::GetAsset(event->value);
    if (info == nullptr)
        return;

    URI uri = URI::parse(info->path);
    if (uri.scheme != Scheme::Project)
        return;

    create_hierarchy(info);
}

void BrowserFile::onAssetRemove(IEvent* data)
{
    auto event = static_cast<AssetEventOnRemove*>(data);

    auto info = AssetDB::GetAsset(event->value);
    if (info == nullptr)
        return;

    URI uri = URI::parse(info->path);

    std::string path = "/" + uri.path;

    auto hash = engine::hash::fnv1_str(path);

    auto node = m_hashmap[hash];

    {
        std::string parentPath = gfile::parent(path);
        auto parentHash = engine::hash::fnv1_str(parentPath);

        auto parentNode = m_hashmap[parentHash];

        parentNode->children.erase(
            std::remove_if(parentNode->children.begin(), 
                           parentNode->children.end(), 
                           [&](auto& n) { return n.get() == node; }),
            parentNode->children.end() // Este argumento estava faltando/fora do lugar
        );   
    }

    m_hashmap.erase(hash);
}

#include <filesystem>
std::vector<BrowserFile::EntryInfo> BrowserFile::splitPath(const std::string& path)
{
    std::vector<EntryInfo> entries;

    std::string current = path;
    while (!current.empty())
    {
        std::string name = gfile::get_file_name(current);
        // std::string name = std::filesystem::path(current).filename();

        EntryInfo entry;
        entry.path = current;
        entry.name = name;

        entries.push_back(entry);

        if (name.empty())
            break;

        current = gfile::parent(current);
        // current = std::filesystem::path(current).parent_path();
    }

    std::reverse(entries.begin(), entries.end());

    return entries;
}

void BrowserFile::create_hierarchy(AssetInfo* info)
{
    URI uri = URI::parse(info->path);

    std::string path = "/" + uri.path;

    auto entries =
        splitPath(path);

    auto parent = m_root.get();

    // std::cout << path << std::endl;

    for (const auto& entry : entries)
    {
        auto hash = engine::hash::fnv1_str(entry.path);

        auto it = m_hashmap.find(hash);
        if (it != m_hashmap.end())
        {
            parent = it->second;
            continue;
        }

        auto node = std::make_shared<FileInfo>();
        node->name = entry.name;
        node->path = entry.path;

        parent->children.push_back(node);

        m_hashmap.emplace(hash, node.get());

        parent = node.get();
    }

    if (
            parent != nullptr &&
            parent->assetID == INVALID_ASSET
        )
    {
        parent->assetID = info->id;
    }
}

void BrowserFile::create_render_popup_asset_info()
{
    if (!ImGui::BeginPopup("create_render_popup_asset_info"))
        return;

    ImGui::SeparatorText("Asset Menu");

    ImGui::Text("Selected: %zu", m_selected_assets.size());
    ImGui::Spacing();

    if (ImGui::BeginChild("AssetList", ImVec2(350.0f, 150.0f), true))
    {
        for (AssetID id : m_selected_assets)
        {
            if (auto* info = AssetDB::GetAsset(id))
            {
                ImGui::BulletText("%s - %d", info->path, info->id);
            }
        }

        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Remove Asset"))
    {
        for (auto& id : m_selected_assets)
        {
            auto info = AssetDB::GetAsset(id);
            if (info == nullptr)
                continue;

            AssetDB::Unregistry(info->hash);
        }

        m_selected_assets.clear();

        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Copy Id"))
    {
        std::string str;
        for (auto& id : m_selected_assets)
            str += std::to_string(id) + "\n";

        ImGui::SetClipboardText(str.c_str());
    }

    if (ImGui::Button("Copy Path"))
    {
        std::string str;
        for (auto& id : m_selected_assets)
        {
            auto info = AssetDB::GetAsset(id);
            if (info == nullptr)
                continue;

            str += info->path;
            str += '\n';
        }

        ImGui::SetClipboardText(str.c_str());
    }

    /*
    ImGui::SameLine();

    if (ImGui::Button("Reload"))
    {
    }
    */

    ImGui::Separator();

    if (ImGui::Button("Close"))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}




