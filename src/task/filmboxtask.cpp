#include "task/filmboxtask.hpp"

#include <gr-base/core/scene.hpp>

#include <gr-base/externals/hash.hpp>

#include "filmbox/filmbox.hpp"

#include "manager/viewManager.hpp"

#include "view/popup/filmboximport.hpp"

#include <cstring>
#include <gr-base/core/file/memoryStream.hpp>
#include <gr-base/core/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>

#include <gr-base/managers/assetsManager.hpp>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) : m_file(file), m_scene(nullptr), m_shader("shader/simple_model.glsl")
{}

void FilmboxLoadTask::onPreExecute()
{
    std::string filename = gfile::get_file_name(m_file);

    Console::LogInfo("[ FilmboxTask ] Loading: %s", filename.c_str());
}

int FilmboxLoadTask::doInBackground()
{
    Filmbox filmbox;
    try
    {
        filmbox.load(m_file);
    }
    catch (const std::exception &e)
    {
        return 1;
    }

    m_scene = filmbox.getScene();

    for (auto& fbxMaterial : m_scene->materials)
    {
        if (m_materialCacheList.find(fbxMaterial.name) != m_materialCacheList.end())
            continue;

        std::string MaterialKey = "material/" + fbxMaterial.name;

        MaterialAsset asset;
        asset.name = fbxMaterial.name;
        asset.hash = hash::fnv1_str(MaterialKey.c_str());

        asset.SetShader(m_shader);

        asset.Set("albedo", fbxMaterial.albedo);
        asset.Set("metallic", fbxMaterial.metallic);
        asset.Set("roughness", fbxMaterial.roughness);
        asset.Set("ambientocclusion", fbxMaterial.ao);

        m_materialCacheList.emplace(fbxMaterial.name, asset);
    }

    std::cout << m_scene->meshes.size() << std::endl;

    size_t vertexOffset = 0;
    for (auto& fbxMesh : m_scene->meshes)
    {
        if (m_meshCacheList.find(fbxMesh.GetName()) == m_meshCacheList.end())
        {
            std::string MeshKey = "mesh/" + fbxMesh.GetName();

            MeshAsset asset;
            asset.name = fbxMesh.GetName();
            asset.hash = hash::fnv1_str(MeshKey.c_str());

            m_meshCacheList.emplace(fbxMesh.GetName(), asset);
        }

        MeshAsset& asset = m_meshCacheList[fbxMesh.GetName()];

        for (const auto& v : fbxMesh.vertices)
        {
            asset.vertices.push_back({v.position, v.normal, v.texCoord});
        }

        auto& submesh = asset.submeshes.emplace_back();
        submesh.materialName = m_scene->materials[fbxMesh.GetMaterialID()].name;

        for (auto& idx : fbxMesh.indices)
        {
            submesh.indices.push_back(idx + static_cast<uint32_t>(vertexOffset));
        }

        vertexOffset += fbxMesh.vertices.size();
    }

    auto& root = m_scene->getRootNode();
    for (auto& fbxNode : root.getChildren())
        m_objectCacheList.push_back(PackingObject(&fbxNode));

    return 0;
}

void FilmboxLoadTask::onPostExecute(Result<int> result)
{
    if (result.value != 0)
        return Console::LogError("[FilmboxLoadTask] fail in to load '%s' error code: %s", m_file.c_str(), result.error.c_str());

    FilmboxImport* view = ViewManager::GetView<FilmboxImport>();
    if (view == nullptr)
        view = ViewManager::AddSubView<FilmboxImport>();

    auto scene = Scene::GetCurrent();

    auto filename = gfile::get_file_name(m_file);

    ModelPack pack;
    pack.materialsAsset = m_materialCacheList;
    pack.meshesAsset = m_meshCacheList;

    ModelAsset& asset = pack.modelAsset;
    asset.childrent = m_objectCacheList;
    asset.name = filename;
    asset.isStatic = false;

    view->addFile(filename, pack);
}

ModelAsset FilmboxLoadTask::PackingObject(const FbxNode* node)
{
    ModelAsset asset;
    asset.name = node->getName();
    asset.isStatic = false;
    asset.mesh = INVALID_HASH64;

    for (auto& meshID : node->GetMesheIDs())
    {
        auto& fbxMesh = m_scene->meshes[meshID];
        const auto& meshName = fbxMesh.GetName();

        std::string MeshKey = "mesh/" + meshName;

        asset.mesh = hash::fnv1_str(MeshKey.c_str());

        auto& fbxMaterial = m_scene->materials[fbxMesh.GetMaterialID()];
        std::string MaterialKey = "material/" + fbxMaterial.name;

        asset.materials.push_back(hash::fnv1_str(MaterialKey.c_str()));
    }

    Transform transform;
    transform.position = node->getPosition();
    transform.rotation = node->getRotation();
    transform.scale = node->getScale();

    asset.AddComponent(transform);

    // if (asset.mesh != INVALID_HASH64)
    // {
        /*
        MeshRef meshRef;
        meshRef.hash = asset.mesh;

        MaterialRef materialRef;
        materialRef.materials = (MaterialHandle*)malloc(asset.materials.size() * sizeof(MaterialHandle));
        materialRef.numMaterials = asset.materials.size();

        memcpy(materialRef.materials, asset.materials.data(), asset.materials.size() * sizeof(MaterialHandle));

        asset.AddComponent(meshRef);
        asset.AddComponent(materialRef);
        */
    // }

    for (auto& child : node->getChildren())
        asset.childrent.push_back(PackingObject(&child));

    return asset;
}





