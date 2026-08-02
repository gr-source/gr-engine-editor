#include "view/spawnObject.hpp"

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/script/transform.hpp>
#include <memory>

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include "gr-base/asset/assetDB.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/script/name.hpp"
#include "imgui.h"
#include "view/imguiRenderer.hpp"

SpawnObject::SpawnObject() :
    numInstances(100),
    position({0.0f, 0.0f}),
    m_info(nullptr)
{}

void SpawnObject::Draw()
{
    auto scene = Scene::GetCurrent();

    if (ImGui::Begin("Spawn bject", &open) && scene != nullptr)
        renderer(scene);

    ImGui::End();
}

void SpawnObject::renderer(Scene* scene)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    if (ImGui::BeginDragDropTargetCustom(window->InnerRect, window->ID))
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
        {
            AssetID* ids =
                (AssetID*)payload->Data;

            m_info = AssetDB::GetAsset(ids[0]);
            if (m_info != nullptr && m_info->type != AssetType_Model)
                m_info = nullptr;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SeparatorText(m_info != nullptr ? m_info->uri : "Not selected file");

    ImGui::InputInt("Num", &numInstances);

    if (ImGui::Button("Spawn") && m_info != nullptr)
        AssetsManager::LoadGameObjectAsync(m_info->uri, &SpawnObject::SetPrefab, this);

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

    // Cria uma shape pra box
    // std::shared_ptr<btCollisionShape> shape =
    //     std::make_shared<btBoxShape>(btVector3());
    // shape->setLocalScaling(btVector3());

    for (int x=-distance;x<distance;x++)
    {
        for (int z=-distance;z<distance;z++)
        {
            int targetX = (currentX + x);
            int targetZ = (currentZ + z);

            std:: string str = "Test[" + std::to_string(targetX) + "][" + std::to_string(targetZ) + "]";

            GameObject gameObject(scene, scene->create(prefabID));

            auto name = gameObject.GetOrAddComponent<Name>();
            name->value = strdup(str.c_str());

            auto transform = gameObject.GetComponent<TransformLocal>();
            transform->position = {(float)targetX * 5.5f, 0.0f, (float)targetZ * 4.5f};

            gameObject.AddComponentByName("CubeMovement");

            scene->MarkChanged<TransformWorld>(gameObject.entity);

            m_instances.push_back(gameObject.entity);
        }
    }

    scene->destroy(prefabID);
}


