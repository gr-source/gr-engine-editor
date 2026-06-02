#pragma once

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/meshasset.hpp"

#include "gr-base/asset/modelasset.hpp"
#include "view/viewlayout.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/ecs/default.hpp>

#include <string>

#include <unordered_map>

struct ModelPack
{
    std::unordered_map<std::string, MaterialAsset> materialsAsset;
    std::unordered_map<std::string, MeshAsset> meshesAsset;

    ModelAsset modelAsset;
};

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport();

    inline const char* getName() const override
    {
        return "Filmbox Import";
    }

    void addFile(const std::string& filename, const ModelPack& pack);

private:
    std::unordered_map<std::string, ModelPack> m_modelPack;

    void onRenderUI() override;

    void render_tree_node(const ModelAsset *asset);
};


