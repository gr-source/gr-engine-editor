#pragma once

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

    const char* GetTitle() const override
    {
        return "SpawnO bject";
    }

    void Draw() override;

private:
    void renderer(Scene* scene);

    void SetPrefab(Entity prefabID);
};


