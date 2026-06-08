#include "system/cubeMovementSystem.hpp"

#include <gr-base/core/scene.hpp>
#include <gr-base/core/clock.hpp>

#include <gr-base/ecs/world.hpp>

#include <gr-base/script/transform.hpp>

#include "script/cubeMovement.hpp"

void CubeMovementSystem::Update(Scene* scene)
{
    World* world = scene->GetWorld();

    world->Query<Transform, CubeMovement>([&](Entity* entities, Transform* transforms, CubeMovement* cubes, size_t count)
    {
        for (size_t i=0;i<count;i++)
        {
            Transform& transform = transforms[i];
            CubeMovement& cube = cubes[i];

            float angle = Clock::time * cube.speed;

            transform.position += 
            {
                sinf(angle) * cube.radius * (float)Clock::deltaTime,
                0.0f,
                cosf(angle) * cube.radius * (float)Clock::deltaTime
            };

            scene->MarkChanged<Transform>(entities[i]);
        }
    });
}

