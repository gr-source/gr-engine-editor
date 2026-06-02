#include "view/spawnObject.hpp"

#include <cstring>
#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/script/transform.hpp>

#include "gr-base/core/scene.hpp"
#include "gr-base/script/name.hpp"
#include "imgui.h"

SpawnObject::SpawnObject() : numInstances(100), position({0.0f, 0.0f})
{}

void SpawnObject::onRenderUI()
{
    auto scene = Scene::GetCurrent();
    if (scene == nullptr)
        return;

    ImGui::InputInt("Num", &numInstances);

    if (ImGui::Button("Spawn"))
    {
        AssetsManager::LoadGameObject("model/Cubo", &SpawnObject::SetPrefab, this);
    }

    ImGui::SameLine();

    if (ImGui::Button("Despawn"))
    {
        for (auto& handle : m_instances)
            scene->destroy(handle);

        m_instances.clear();
    }

}

void SpawnObject::SetPrefab(Entity prefabID)
{
    auto scene = Scene::GetCurrent();
    if (scene == nullptr)
        return;

    int distance = numInstances / 2;

    int currentX = floor(position.x / 16.0f);
    int currentZ = floor(position.y / 16.0f);

    for (int x=-distance;x<distance;x++)
    {
        for (int z=-distance;z<distance;z++)
        {
            int targetX = (currentX + x);
            int targetZ = (currentZ + z);

            std:: string str = "Test[" + std::to_string(targetX) + "][" + std::to_string(targetZ) + "]";

            auto gameObject = scene->Wrap(scene->create(prefabID));
            auto name = gameObject.GetOrAddComponent<Name>();
            name->value = strdup(str.c_str());

            auto transform = gameObject.GetOrAddComponent<Transform>();
            transform->position = {(float)targetX * 5.5f, 0.0f, (float)targetZ * 4.5f};

            scene->MarkChanged<Transform>(gameObject.entity);

            m_instances.push_back(gameObject.entity);
        }
    }

    scene->destroy(prefabID);

    // position += Vector2{(float)distance * 16.0f * 2.0f, 0.0f};

    // GameObject::DestroyImediate(prefabID);

    // MainThreadDispatcher::AddDelayed(10.0f, &TestPopup::Spawn, this, prefabID);
}


