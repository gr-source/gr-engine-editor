#include "view/browserFile.hpp"

// #include "gr-base/asset/assetDB.hpp"

#include <gr-base/core/assetResolver.hpp>
#include <gr-base/core/gameObject.hpp>
#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/file/file.hpp>

#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/taskmanager.hpp"

#include "gr-base/managers/eventManager.hpp"
#include "gr-base/task/assetSaveTask.hpp"
#include "gr-base/task/modelsavetask.hpp"
#include "imgui.h"
#include "view/imguiWidget.hpp"

#include <gr-render/texture.hpp>

#include <algorithm>

#define GETNAME(n) #n

BrowserFile::BrowserFile() :
    m_imguiIO(&ImGui::GetIO())
{
    open = true;

    entryRootID =
        m_spaseSet.emplace(FileInfo{
                    .name = "project://",
                    .path = "/",
                    .hash = engine::hash::fnv1_str("/")
                });

    auto root = m_spaseSet.get(entryRootID);

    m_hashmap.emplace(root->hash, entryRootID);

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
    // auto avail = ImGui::GetContentRegionAvail();

    ImGui::Columns(2, nullptr, true);
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("browserFile_render_directories"))
    {
        render_virtual_directories(entryRootID);

        ImGui::EndChild();
    }

    ImGui::NextColumn();
    if (ImGui::BeginChild("browserFile_render_entries"))
    {
        if (entrySelectedDir != INVALID_SPARSE_ID)
            render_virtual_entries(entrySelectedDir);

        ImGui::EndChild();
    }

    create_call_popup();

    create_popup();
}

void BrowserFile::render_virtual_directories(EntryID entryID)
{
    auto info =
        m_spaseSet.get(entryID);

    ImGuiTreeNodeFlags flags = 0;
    if (info->children.empty())
        return;

    auto& name = info->name;

    if (entrySelectedDir == entryID)
        flags |= ImGuiTreeNodeFlags_Selected;

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        entrySelectedDir = entryID;

        m_selected_assets.clear();
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        entrySelectedDir = entryID;

        popupFlags |= PopupFlags_ShowDirOption;
        /*
        Hash64 hash =
            engine::hash::fnv1_str(info->path);

        auto it =
            m_hashmap.find(hash);
        if (it != m_hashmap.end())
        {
            std::cout << "Delete: " << name << ", " << info->path << std::endl;

            for (auto& child : it->second->children)
            {
                std::cout << child->path << std::endl;
            }
        }
        */
    }

    drop_source_in_directories();

    if (isOpen)
    {
        for (auto& childEntryID : info->children)
            render_virtual_directories(childEntryID);

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

    Entity entity =
        *static_cast<Entity*>(payload->Data);

    GameObject gameobject(scene, entity);
    if (gameobject.IsAlive())
    {
        URI uri = URI::parse("project://model/" + gameobject.GetName());

        AssetID id = AssetDB::FindByHash(engine::hash::fnv1_str(uri.toString()));
        if (id == INVALID_ASSET)
            id = AssetDB::CreateID();

        AssetSaveInfo info;
        info.type = AssetType_Model;
        info.id = id;
        info.uri = uri;

        info.asset = std::make_shared<ModelAsset>(ModelSaveTask::ConvertToAssetModel(gameobject));

        TaskManager::Add<AssetSaveTask>(info);

        // TaskManager::Add<ModelSaveTask>(uri, gameobject);
    }

    ImGui::EndDragDropTarget();
}

void BrowserFile::render_virtual_entries(EntryID entryID)
{
    auto info =
        m_spaseSet.get(entryID);

    auto& name = info->name;
    ImGui::SeparatorText(name.c_str());

    ImGui::Columns(4, nullptr, false);
    for (auto& entryID : info->children)
    {
        auto entry =
            m_spaseSet.get(entryID);

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
                m_imguiIO->KeyShift &&
                ImGui::IsItemClicked(ImGuiMouseButton_Left)
            )
        {
            if (bSelected)
                m_selected_assets.erase(entry->assetID);
            else
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
                {
                    m_selected_assets.clear();
                    m_selected_assets.emplace(entry->assetID);
                }
            }

            drag_selected_entries();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && m_selected_assets.find(entry->assetID) != m_selected_assets.end())
        {
            popupFlags |= PopupFlags_ShowAssetInfo;
        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            popupFlags |= PopupFlags_ShowAssetInfo;

            m_selected_assets.clear();

            m_selected_assets.emplace(entry->assetID);
        }

        ImGui::NextColumn();
    }

    if (ImGui::IsWindowHovered())
    {
        if (m_imguiIO->KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
        {
            m_selected_assets.clear();

            for (auto &entryID : info->children)
            {
                auto entry =
                    m_spaseSet.get(entryID);

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

void BrowserFile::onAssetLoadedDB(AssetEventOnLoaded* event)
{
    try
    {
        m_fileTexture =
            AssetsManager::LoadTexture("engine://icon/icons8-file-30.png");
    } catch (const std::exception& e)
    {
        Console::LogError("[ BrowserFile ] %s", e.what());
    }
}

void BrowserFile::onAssetCreated(AssetEventOnCreated* event)
{
    auto info = AssetDB::GetAsset(event->value);
    if (info == nullptr)
        return;

    URI uri = URI::parse(info->uri);
    if (uri.scheme != Scheme_Project)
        return;

    create_hierarchy(info);
}

void BrowserFile::onAssetRemove(AssetEventOnRemove* event)
{
    auto info = AssetDB::GetAsset(event->value);
    if (info == nullptr)
        return;

    auto relPath = AssetResolver::Resolve(info->uri);
    if (gfile::exists(relPath))
        gfile::remove(relPath);

    /*
    URI uri = URI::parse(info->uri);

    std::string path = "/" + uri.path;

    auto hash = engine::hash::fnv1_str(path);

    auto node = m_hashmap[hash];

    {
        std::string parentPath = gfile::parent(path);
        auto parentHash = engine::hash::fnv1_str(parentPath);

        auto parentEntryID = m_hashmap[parentHash];

        auto parentEntry =
            m_spaseSet.get(parentEntryID);

        parentEntry->children.erase(
            std::remove_if(parentEntry->children.begin(), 
                           parentEntry->children.end(), 
                           [&](EntryID entryID) { return entryID == node; }),
            parentEntry->children.end() // Este argumento estava faltando/fora do lugar
        );   
    }

    m_hashmap.erase(hash);
    */
}

std::vector<BrowserFile::EntryInfo> BrowserFile::splitPath(const std::string& path)
{
    std::vector<EntryInfo> entries;

    std::string current = path;
    while (!current.empty())
    {
        std::string name = gfile::get_file_name(current);

        EntryInfo entry;
        entry.path = current;
        entry.name = name;

        entries.push_back(entry);

        if (name.empty())
            break;

        current = gfile::parent(current);
    }

    std::reverse(entries.begin(), entries.end());

    return entries;
}

void BrowserFile::create_hierarchy(AssetInfo* info)
{
    URI uri = URI::parse(info->uri);

    std::string path = "/" + uri.path;

    auto entries =
        splitPath(path);

    auto parentID = entryRootID;

    for (const auto& entry : entries)
    {
        auto hash = engine::hash::fnv1_str(entry.path);

        auto it = m_hashmap.find(hash);
        if (it != m_hashmap.end())
        {
            parentID = it->second;
            continue;
        }

        auto newEntryID =
            m_spaseSet.emplace(FileInfo{
                        .name = entry.name,
                        .path = entry.path,
                        .hash = hash,
                        .parent = parentID
                    });

        auto parentPtr =
            m_spaseSet.get(parentID);

        parentPtr->children.push_back(newEntryID);

        m_hashmap.emplace(hash, newEntryID);

        parentID = newEntryID;
    }

    auto parentPtr =
        m_spaseSet.get(parentID);

    if (parentPtr->assetID == INVALID_ASSET)
        parentPtr->assetID = info->id;
}

void BrowserFile::create_call_popup()
{
    switch (popupFlags)
    {
        case PopupFlags_ShowAssetInfo:
            ImGui::OpenPopup(GETNAME(PopupFlags_ShowAssetInfo));
            break;
        case PopupFlags_ShowDirOption:
            ImGui::OpenPopup(GETNAME(PopupFlags_ShowDirOption));
            break;
        default:
            break;
    }

    popupFlags = 0;
}

void BrowserFile::create_popup()
{
    create_popup_asset_info();

    create_popup_dir_option();
}

void BrowserFile::create_popup_asset_info()
{
    if (!ImGui::BeginPopup(GETNAME(PopupFlags_ShowAssetInfo)))
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
                ImGui::BulletText("%s - %d", info->uri, info->id);
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

            // AssetDB::Unregistry(info->hash);
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

            str += info->uri;
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

void BrowserFile::create_popup_dir_option()
{
    if (!ImGui::BeginPopup(GETNAME(PopupFlags_ShowDirOption)))
        return;

    auto info =
        m_spaseSet.get(entrySelectedDir);

    ImGui::SeparatorText(info->name.c_str());

    if (ImGui::Button("Delete") && entrySelectedDir != entryRootID)
    {
        remove_all_assets_on_dir(entrySelectedDir);
        entrySelectedDir =
            INVALID_SPARSE_ID;

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}


void BrowserFile::remove_all_assets_on_dir(EntryID entryID)
{
    auto info =
        m_spaseSet.get(entryID);

    if (info->assetID != INVALID_ASSET)
        AssetDB::Unregistry(info->assetID);

    if (info->parent != INVALID_SPARSE_ID)
    {
        auto parent = m_spaseSet.get(info->parent);
        parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), entryID), parent->children.end());
    }

    auto temp = info->children;
    for (auto& childEntryID : temp)
        remove_all_assets_on_dir(childEntryID);

    m_hashmap.erase(info->hash);

    m_spaseSet.destroy(entryID);
}




