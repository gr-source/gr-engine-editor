#pragma once

#include "filmbox/default.hpp"

#include "filmbox/fbxScene.hpp"

#include "task/model/modelCache.hpp"

#include <gr-base/core/thread/asynctask.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/core/default.hpp>

#include <gr-base/core/geometry/material.hpp>
#include <gr-base/core/geometry/mesh.hpp>

#include <gr-base/ecs/default.hpp>

#include <gr-base/asset/materialasset.hpp>
#include <gr-base/asset/modelasset.hpp>
#include <gr-base/asset/meshasset.hpp>

#include <unordered_map>

class FilmboxLoadTask : public IAsyncTask<FbxScene>
{
public:
    FilmboxLoadTask(const std::string& file);

private:
    Handle m_dialog;

    std::string m_file;

    std::string m_shader;

    std::unordered_map<std::string, MaterialAsset> m_materialCacheList;

    std::unordered_map<std::string, MeshAsset> m_meshCacheList;

    std::vector<ModelNode> m_objectCacheList;

    void onPreExecute() override;

    FbxScene doInBackground() override;

    void onPostExecute(Result<FbxScene> result) override;

    // ModelNode PackingObject(const FbxNode* node);
};

