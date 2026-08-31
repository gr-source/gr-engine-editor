#pragma once

#include "view/iwindowView.hpp"

#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/default.hpp"

#include <gr-base/ecs/default.hpp>

#include <gr-math/gmath.hpp>

#include <vector>

struct EditorContext;

class SpawnObject : public IWindowView
{
    int numInstances;

    Vector2 position;

    std::vector<Entity> m_instances;

public:
    SpawnObject(EditorContext* context);

    void Draw() override;

private:
    EditorContext* context =
        nullptr;

    AssetInfo* m_info =
        nullptr;

    void renderer(Scene* scene);

    void SetPrefab(Entity prefabID);

    void drop_asset();
};


