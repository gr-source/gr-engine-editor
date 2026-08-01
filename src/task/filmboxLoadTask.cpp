#include "task/filmboxLoadTask.hpp"

#include <gr-base/core/scene.hpp>

#include <gr-base/externals/hash.hpp>

#include "event/importAssetViewPopupEvent.hpp"

#include "filmbox/filmbox.hpp"

#include "gr-base/managers/eventManager.hpp"

#include "view/loadingTaskPopup.hpp"

#include <gr-base/core/file/memoryStream.hpp>
#include <gr-base/core/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>

#include <gr-base/managers/assetsManager.hpp>
#include <stdexcept>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) :
    m_file(file),
    m_shader("engine://shader/simple_model"),
    m_dialog(HANDLE_INVALID)
{}

void FilmboxLoadTask::onPreExecute()
{
    auto filename = gfile::get_file_name(m_file);

    m_dialog =
        LoadingTaskPopup::Create(
            "Filmbox Task",
            String::format("Loading file '%s'", filename.c_str())
        );

    /*
    m_dialog =
        m_workspace->CreateTaskView(
            "Filmbox Task",
            String::format("Loading file '%s'", filename.c_str())
        );
    */
}

FbxScene FilmboxLoadTask::doInBackground()
{
    Filmbox filmbox;

    FbxScene fbxScene;
    
    std::string error;
    if (!filmbox.load(m_file, error, &fbxScene))
        throw std::runtime_error(error);

    if (fbxScene.name.empty())
        fbxScene.name = gfile::get_file_name(m_file);

    fbxScene.root.name = gfile::get_file_name(m_file);

    return fbxScene;

    /*
    m_scene = filmbox.getScene();

    for (auto& fbxMaterial : m_scene->materials)
    {
        if (m_materialCacheList.find(fbxMaterial.name) != m_materialCacheList.end())
            continue;

        MaterialAsset asset;
        asset.name = fbxMaterial.name;
        asset.shader = m_shader;

        asset.Set("albedo", fbxMaterial.albedo);
        asset.Set("metallic", fbxMaterial.metallic);
        asset.Set("roughness", fbxMaterial.roughness);
        asset.Set("ambientocclusion", fbxMaterial.ao);

        m_materialCacheList.emplace(fbxMaterial.name, asset);
    }

    for (auto& fbxMesh : m_scene->meshes)
    {
        if (m_meshCacheList.find(fbxMesh.name) == m_meshCacheList.end())
        {
            MeshAsset asset;
            asset.name = fbxMesh.name;

            m_meshCacheList.emplace(fbxMesh.name, asset);
        }

        const auto& vertices = fbxMesh.vertices;
        const auto& indices = fbxMesh.indices;

        MeshAsset& asset = m_meshCacheList[fbxMesh.name];

        size_t vertexOffset = asset.vertices.size();
        size_t indexOffset = asset.indices.size();

        for (const auto& v : vertices)
        {
            asset.vertices.push_back({v.position, v.normal, v.texCoord});
        }

        for (auto& idx : indices)
            asset.indices.push_back(idx + static_cast<uint32_t>(vertexOffset));

        auto& submesh = asset.submeshes.emplace_back();
        submesh.materialName = m_scene->materials[fbxMesh.materialID].name;
        submesh.indexCount = static_cast<uint32_t>(indices.size());
        submesh.indexOffset = static_cast<uint32_t>(indexOffset);
    }

    // auto filename = gfile::get_file_name(m_file);

    const auto& root = m_scene->root;
    for (const auto& fbxNode : root.children)
        m_objectCacheList.push_back(PackingObject(&fbxNode));

    return 0;
    */
}

void FilmboxLoadTask::onPostExecute(Result<FbxScene> result)
{
    LoadingTaskPopup::Destroy(m_dialog);

    if (!result.success)
        return Console::LogError("[FilmboxLoadTask] fail in to load '%s' error= '%s'.", m_file.c_str(), result.error.c_str());

    ImportAssetViewPopup_onAddFbxScene event;
    event.fbxScene = result.value;

    EventManager::Notify<ImportAssetViewPopup_onAddFbxScene>(&event);

    /*
    auto& viewManager = m_workspace->GetViewManager();

    auto view = viewManager.getOrAddView<ImportAssetViewPopup>();
    view->AddFbxScene(result.value);
    */

    /*
    FilmboxImport* view = viewManager.getOrAddView<FilmboxImport>();
    view->visible = true;

    auto scene = Scene::GetCurrent();

    auto filename = gfile::get_file_name(m_file);

    ModelPack pack;
    pack.materialsAsset = m_materialCacheList;
    pack.meshesAsset = m_meshCacheList;

    auto& root = pack.root;
    root.children = m_objectCacheList;
    root.name = filename;

    view->addFile(filename, pack);
    */
}

/*
ModelNode FilmboxLoadTask::PackingObject(const FbxNode* fbxNode)
{
    ModelNode node;
    node.name = fbxNode->name;

    node.position =
        fbxNode->position;
    node.rotation =
        fbxNode->rotation;
    node.scale =
        fbxNode->scale;

    for (auto& meshID : fbxNode->meshes)
    {
        // auto& fbxMesh = m_scene->meshes[meshID];
        // const auto& meshName = fbxMesh.name;

        {
            auto it = m_meshCacheList.find(meshName);
            if (it != m_meshCacheList.end())
                asset.mesh = it->second.hash;
        }

        // auto& fbxMaterial = m_scene->materials[fbxMesh.materialID];
        {
            auto it = m_materialCacheList.find(fbxMaterial.name);
            if (it != m_materialCacheList.end())
                asset.materials.push_back(it->second.hash);
        }
    }

    // asset.AddComponent(transform);

    // if (asset.mesh != INVALID_HASH64)
    // {
        MeshRef meshRef;
        meshRef.hash = asset.mesh;

        MaterialRef materialRef;
        materialRef.materials = (MaterialHandle*)malloc(asset.materials.size() * sizeof(MaterialHandle));
        materialRef.numMaterials = asset.materials.size();

        memcpy(materialRef.materials, asset.materials.data(), asset.materials.size() * sizeof(MaterialHandle));

        asset.AddComponent(meshRef);
        asset.AddComponent(materialRef);
    // }

    for (auto& child : fbxNode->children)
        node.children.push_back(PackingObject(&child));

    return node;
}*/





