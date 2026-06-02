#pragma once

#include "view/viewlayout.hpp"

#include <gr-base/ecs/default.hpp>

#include <gr-math/gmath.hpp>

#include <vector>

class SpawnObject : public ViewLayout
{
    int numInstances;

    Vector2 position;

    std::vector<Entity> m_instances;

public:
    SpawnObject();

    inline virtual const char* getName() const override
    {
        return "SpawnObject";
    }

    virtual void onRenderUI() override;

private:
    void SetPrefab(Entity prefabID);
};


