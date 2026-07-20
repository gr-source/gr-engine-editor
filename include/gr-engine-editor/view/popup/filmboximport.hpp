#pragma once

#include "view/windowView.hpp"

#include "task/model/modelCache.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/ecs/default.hpp>

#include <string>

/*
class FilmboxImport : public WindowView
{
public:
    FilmboxImport();

    inline const char* GetTitle() const override
    {
        return "Filmbox Import";
    }

    void addFile(const std::string& filename, const ModelPack& pack);

    void SetTitle(const std::string& title);
    void SetMessage(const std::string& message);
    void Show();

private:
    std::unordered_map<std::string, ModelPack> m_modelPack;

    void Draw() override;

    void render_tree_node(const ModelNode* node);
};
*/

