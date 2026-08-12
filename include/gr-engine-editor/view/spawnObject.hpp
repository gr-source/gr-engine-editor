#pragma once

#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/default.hpp"

#include "view/windowView.hpp"

#include <gr-base/ecs/default.hpp>

#include <gr-math/gmath.hpp>

#include <vector>

class SpawnObject : public WindowView
{
    int numInstances;

    Vector2 position;

    std::vector<Entity> m_instances;

public:
    SpawnObject();

    void Draw() override;

private:
    AssetInfo* m_info;

    void renderer(Scene* scene);

    void SetPrefab(Entity prefabID);
};


