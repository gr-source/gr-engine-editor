#include "view/popup/filmboximport.hpp"

#include "editorApplication.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/eventManager.hpp"

#include "manager/viewManager.hpp"

#include <gr-base/script/transform.hpp>

#include <gr-base/core/gameObject.hpp>

#include "gr-base/task/materialsavetask.hpp"
#include "gr-base/task/modelsavetask.hpp"
#include "gr-base/task/meshsavetask.hpp"

#include "imgui.h"

#include <imgui.h>

FilmboxImport::FilmboxImport()
{}

void FilmboxImport::addFile(const std::string& filename, const ModelPack& pack)
{
    m_modelPack.emplace(filename, pack);
}

void FilmboxImport::onRenderUI()
{
    EditorApplication* engine = static_cast<EditorApplication*>(Application::application);

    for (auto &&[filename, pack] : m_modelPack)
    {
        if (ImGui::CollapsingHeader(filename.c_str()))
        {
            auto& materialsCache = pack.materialsAsset;
            if (!materialsCache.empty() && ImGui::TreeNode("Material"))
            {
                for (auto&&[name, asset] : materialsCache)
                {
                    ImGui::Text("%s", name.c_str());
                }
                ImGui::TreePop();
            }

            auto& meshesCache = pack.meshesAsset;
            if (!meshesCache.empty() && ImGui::TreeNode("Mesh"))
            {
                for (auto &&[name, mesh] : meshesCache)
                    ImGui::Text("%s sub: %ld", name.c_str(), mesh.submeshes.size());

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("GameObject"))
            {
                render_tree_node(&pack.modelAsset);
                ImGui::TreePop();
            }
        }
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (engine->getProject() != nullptr && ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
    {
        for (auto &&[filename, cache] : m_modelPack)
        {
            // material
            {
                for (auto&&[asset_name, asset] : cache.materialsAsset)
                {
                    auto path = "material/" + asset_name;

                    TaskManager::Add<MaterialSaveTask>(path, asset);
                }
            }

            // mesh
            {
                for (auto&&[asset_name, asset] : cache.meshesAsset)
                {
                    auto path = "mesh/" + asset_name;

                    TaskManager::Add<MeshSaveTask>(path, asset);
                }
            }

            // gameObject
            {
                auto& asset = cache.modelAsset;

                auto path = "model/" + asset.name;

                TaskManager::Add<ModelSaveTask>(path, asset);
            }
        }

        visible = false;
    }
}

void FilmboxImport::render_tree_node(const ModelAsset* asset)
{
    auto& childrent = asset->childrent;

    auto isOpen = ImGui::TreeNodeEx(asset->name.c_str(), childrent.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
        return;

    for (auto &child : childrent)
        render_tree_node(&child);

    ImGui::TreePop();
}



