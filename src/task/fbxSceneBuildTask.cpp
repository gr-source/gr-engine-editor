#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/meshasset.hpp"

#include "gr-base/asset/textureAsset.hpp"

#include <gr-base/core/assetDB.hpp>
#include <gr-base/core/console.hpp>

#include <gr-base/file/file.hpp>

#include "gr-base/managers/taskmanager.hpp"

#include "gr-base/script/transform.hpp"

#include "task/fbxSceneBuildTask.hpp.hpp"

#include "view/loadingTaskPopup.hpp"

#include "gr-base/task/assetSaveTask.hpp"
#include <stdexcept>

FbxSceneBuildTask::FbxSceneBuildTask(const FbxScene& scene) : m_fbxScene(scene)
{}

void FbxSceneBuildTask::onPreExecute()
{
    m_dialog =
        LoadingTaskPopup::Create(
            "Building",
            String::format("Formatando a cena '%s' para o sistema interno...", m_fbxScene.name.c_str())
        );
}

ModelAsset FbxSceneBuildTask::doInBackground()
{
    AssetID shader =
        AssetDB::FindByName("engine://shader/simple_model");
    if (shader == INVALID_ASSET)
        throw std::runtime_error("Shader default not found");

    for (const auto& path : m_fbxScene.textures)
    {
        std::string filename = gfile::get_file_name(path);

        try
        {
            TextureAsset asset = buildToTextureAsset(path);

            std::string uri = "project://texture/" + m_fbxScene.name + "/" + filename;

            AssetID id = AssetDB::FindByName(uri);
            if (id == INVALID_ASSET)
                id = AssetDB::CreateID();

            AssetWrapper<TextureAsset> wrapper;
            wrapper.asset = asset;
            wrapper.id = id;
            wrapper.uri = uri;

            m_textureMapWrapper.emplace(path, wrapper);
        } catch (const std::exception& e)
        {
            Console::LogDefault("{FFA500}[ FbxSceneBuildTask ] {FFFFFF}%s.", e.what());
        }
    }

    for (const auto& fbxMaterial : m_fbxScene.materials)
    {
        MaterialAsset asset;
        asset.shader = shader;
        asset.name = fbxMaterial.name;
        asset.Set("albedo", fbxMaterial.albedo);
        asset.Set("metallic", fbxMaterial.metallic);
        asset.Set("roughness", fbxMaterial.roughness);
        asset.Set("ambientocclusion", fbxMaterial.ao);

        for (auto&& [type, id] : fbxMaterial.textures)
        {
            const auto& path =
                m_fbxScene.textures[id];

            auto it =
                m_textureMapWrapper.find(path);
            if (it == m_textureMapWrapper.end())
                continue;

            switch (type)
            {
                case TextureType_Albedo:
                    asset.SetTexture("albedoTex", it->second.id);
                    break;
                default:
                    break;
            }
        }

        std::string uri = "project://material/" + m_fbxScene.name + "/" + fbxMaterial.name;

        AssetID id = AssetDB::FindByName(uri);
        if (id == INVALID_ASSET)
            id = AssetDB::CreateID();

        AssetWrapper<MaterialAsset> wrapper;
        wrapper.uri = uri;
        wrapper.asset = asset;
        wrapper.id = id;

        m_materialMapWrapper.emplace(fbxMaterial.name, wrapper);
    }

    for (auto& fbxMesh : m_fbxScene.meshes)
    {
        auto& fbxVertices = fbxMesh.vertices;
        auto& fbxIndices = fbxMesh.indices;
        auto& fbxSubmesh = fbxMesh.submeshes;

        std::vector<Vertex3D> vertices(fbxVertices.size());
        for (size_t i=0;i<fbxVertices.size();i++)
            vertices[i] =
                {
                    .position = fbxVertices[i].position,
                    .normal = fbxVertices[i].normal,
                    .texCoords = fbxVertices[i].texCoord
                };

        std::vector<uint32_t> indices(fbxIndices.size());
        for (size_t i=0;i<fbxIndices.size();i++)
            indices[i] =
                fbxIndices[i];

        std::vector<SubMeshAsset> submeshes;
        for (auto& fbxSubmesh : fbxMesh.submeshes)
        {
            auto& fbxMaterial = m_fbxScene.materials[fbxSubmesh.materialID];

            SubMeshAsset submesh;
            submesh.indexOffset = fbxSubmesh.indexOffset;
            submesh.indexCount = fbxSubmesh.indexCount;

            submeshes.push_back(submesh);
        }

        MeshAsset asset;
        asset.name = fbxMesh.name;
        asset.vertices = vertices;
        asset.indices = indices;
        asset.submeshes = submeshes;

        std::string uri = "project://mesh/" + m_fbxScene.name + "/" + fbxMesh.name;

        AssetID id = AssetDB::FindByName(uri);
        if (id == INVALID_ASSET)
            id = AssetDB::CreateID();

        AssetWrapper<MeshAsset> wrapper;
        wrapper.uri = uri;
        wrapper.asset = asset;
        wrapper.id = id;

        m_meshMapWrapper.emplace(fbxMesh.name, wrapper);
    }

    return readNode(&m_fbxScene.root);
}

void FbxSceneBuildTask::onPostExecute(Result<ModelAsset> result)
{
    LoadingTaskPopup::Destroy(m_dialog);

    if (!result.success)
        return Console::LogDefault("{FF0000}[ FbxSceneBuildTask ] {FFFFFF}%s.", result.error.c_str());

    for (auto&& [name, wrapper] : m_textureMapWrapper)
    {
        AssetSaveInfo info;
        info.type =
            AssetType_Texture;

        info.id =
            wrapper.id;

        info.uri =
            URI::parse(wrapper.uri);

        info.asset =
            std::make_shared<TextureAsset>(wrapper.asset);

        TaskManager::Add<AssetSaveTask>(info);
    }

    for (auto&& [name, wrapper] : m_materialMapWrapper)
    {
        AssetSaveInfo info;
        info.type =
            AssetType_Material;

        info.id =
            wrapper.id;

        info.uri =
            URI::parse(wrapper.uri);

        info.asset =
            std::make_shared<MaterialAsset>(wrapper.asset);

        TaskManager::Add<AssetSaveTask>(info);
    }

    for (auto&& [name, wrapper] : m_meshMapWrapper)
    {
        AssetSaveInfo info;
        info.type =
            AssetType_Mesh;

        info.id =
            wrapper.id;

        info.uri =
            URI::parse(wrapper.uri);

        info.asset =
            std::make_shared<MeshAsset>(wrapper.asset);

        TaskManager::Add<AssetSaveTask>(info);
    }

    auto& asset = result.value;

    std::string uri =
        "project://model/" + m_fbxScene.name + "/" + asset.name;

    AssetID id = AssetDB::FindByName(uri);
    if (id == INVALID_ASSET)
        id = AssetDB::CreateID();

    AssetSaveInfo info;
    info.type =
        AssetType_Model;

    info.id =
        id;

    info.uri =
        URI::parse(uri);

    info.asset =
        std::make_shared<ModelAsset>(asset);

    TaskManager::Add<AssetSaveTask>(info);
}

ModelAsset FbxSceneBuildTask::readNode(const FbxNode* fbxNode)
{
    auto& name = fbxNode->name;

    std::vector<AssetID> materials;
    AssetID mesh = INVALID_ASSET;

    if (fbxNode->mesh != -1)
    {
        auto& fbxMesh = m_fbxScene.meshes[fbxNode->mesh];
        materials.resize(fbxMesh.submeshes.size());

        for (size_t i=0;i<fbxMesh.submeshes.size();i++)
        {
            auto& fbxMaterial = m_fbxScene.materials[fbxMesh.submeshes[i].materialID];

            materials[i] = m_materialMapWrapper[fbxMaterial.name].id;
        }

        mesh = m_meshMapWrapper[fbxMesh.name].id;
    }

    std::vector<ModelAsset> children;
    for (auto& child : fbxNode->children)
        children.push_back(readNode(&child));

    TransformLocal transform;
    transform.position = fbxNode->position;
    transform.rotation = fbxNode->rotation;
    transform.scale = fbxNode->scale;

    ModelAsset asset;
    asset.name = name;
    asset.children = children;

    asset.materials = materials;
    asset.mesh = mesh;

    asset.AddComponent<TransformLocal>(transform);

    return asset;
}

#define STB_IMAGE_IMPLEMENTATION
#include "gr-base/core/stb/stb_image.h"

TextureAsset FbxSceneBuildTask::buildToTextureAsset(const std::string& path)
{
    if (!gfile::exists(path))
        throw std::runtime_error(std::string(strerror(ENOENT)) + ": " + path);

    bool isHDR = stbi_is_hdr(path.c_str());

    int width;
    int height;
    int ch;

    void* raw = isHDR ?
        static_cast<void*>(stbi_loadf(path.c_str(), &width, &height, &ch, 0)) :
        static_cast<void*>(stbi_load(path.c_str(), &width, &height, &ch, 0));

    if (raw == nullptr)
        throw std::runtime_error("failed to load texture: " + path);

    size_t pixelCount = width * height * ch;
    size_t byteSize = isHDR
        ? pixelCount * sizeof(float)
        : pixelCount * sizeof(unsigned char);

    std::vector<uint8_t> pixels(byteSize);
    memcpy(pixels.data(), raw, byteSize);

    stbi_image_free(raw);

    TextureAsset asset;
    asset.width = width;
    asset.height = height;
    asset.channels = ch;
    asset.hdr = isHDR;

    asset.raw = pixels;

    return asset;
}

