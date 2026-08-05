#pragma once

#include "filmbox/fbxNode.hpp"
#include "filmbox/fbxScene.hpp"

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/meshasset.hpp"
#include "gr-base/asset/modelasset.hpp"
#include "gr-base/asset/textureAsset.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/externals/handle.h"

class FbxSceneBuildTask : public IAsyncTask<ModelAsset>
{
public:
    FbxSceneBuildTask(const FbxScene& scene);

private:
    const FbxScene m_fbxScene;

    template <typename T>
    struct AssetWrapper
    {
        T asset;
        std::string uri;
        AssetID id;
    };

    std::unordered_map<std::string, AssetWrapper<MaterialAsset>> m_materialMapWrapper;
    std::unordered_map<std::string, AssetWrapper<MeshAsset>> m_meshMapWrapper;

    Handle m_dialog;

    void onPreExecute() override;

    ModelAsset doInBackground() override;

    void onPostExecute(Result<ModelAsset> result) override;

    ModelAsset readNode(const FbxNode* fbxNode);

    TextureAsset loadTextureAsset(const std::string& path);
};


