#include "view/spawnObject.hpp"

#include "view/imguiRenderer.hpp"

#include "view/editor/editorContext.hpp"

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/mainthreaddispatcher.hpp>
#include <gr-base/core/gameObject.hpp>

#include <gr-base/script/transform.hpp>

#include <gr-base/script/name.hpp>
#include <iostream>

#include "imgui.h"

SpawnObject::SpawnObject(EditorContext* context) :
    numInstances(100),
    position({0.0f, 0.0f}),
    m_info(nullptr),
    context(context)
{}

void SpawnObject::Draw()
{
    if (ImGui::Begin("Spawn bject", &open) && context->scene != nullptr)
        renderer(context->scene);

    ImGui::End();
}

void SpawnObject::renderer(Scene* scene)
{
    drop_asset();

    ImGui::SeparatorText(m_info != nullptr ? m_info->uri : "Not selected file");

    ImGui::InputInt("Instances", &numInstances);

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

    const int start = -(numInstances / 2);

    for (int x = 0; x < numInstances; ++x)
    {
        for (int z = 0; z < numInstances; ++z)
        {
            const float targetX = (start + x) * 5.0f;
            const float targetZ = (start + z) * 5.0f;

            std:: string str = "Test[" + std::to_string(targetX) + "][" + std::to_string(targetZ) + "]";

            GameObject gameObject(scene, scene->create(prefabID));

            auto& name = gameObject.GetOrAddComponent<Name>();
            name.value = strdup(str.c_str());

            auto& transform = gameObject.GetComponent<TransformLocal>();
            transform.position = {(float)targetX, 0.0f, (float)targetZ };

            gameObject.AddComponentByName("CubeMovement");

            int index = x * numInstances + z;

            MainThreadDispatcher::Queue(index * 0.8f, [gameObject = std::forward<GameObject>(gameObject)]() mutable
            {
                if (gameObject.IsAlive())
                {
                    // gameObject.TryAddComponent<Static>();

                    std::cout << gameObject.GetName() << std::endl;
                }
            });

            m_instances.push_back(gameObject.entity);
        }
    }

    scene->destroy(prefabID);
}

void SpawnObject::drop_asset()
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
}


