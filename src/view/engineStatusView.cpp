#include "view/engineStatusView.hpp"

#include <gr-base/core/status.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/ecs/archetypeStorage.hpp>
#include <gr-base/ecs/world.hpp>

#include <imgui.h>

void EngineStatusView::onRenderUI()
{
    Scene* scene = Scene::GetCurrent();

    ImGui::SeparatorText("World");
    if (scene != nullptr)
    {
        ImGui::Text("Entities: %ld", scene->GetEntityCount());

        auto world = scene->GetWorld();

        ImGui::Text("Archetypes: %u", world->GetArchetypeCount());

        auto array = world->GetRawArchetype();

        for (ArchetypeID i = 0; i < world->GetArchetypeCount(); i++)
        {
            ArchetypeStorage& arche = array[i];

            if (ImGui::TreeNode((void*)(intptr_t)i, "Archetype %u", i))
            {
                // =========================
                // COMPONENT TYPES
                // =========================
                ImGui::Text("Component Types: %zu", arche.GetTypes().size());

                if (ImGui::TreeNode("Types"))
                {
                    for (auto typeID : arche.GetTypes())
                    {
                        auto type = Type::GetTypeInfoByID(typeID);
                        ImGui::BulletText("%s (ID: %u)", type->name, typeID);
                    }
                    ImGui::TreePop();
                }

                // =========================
                // STORAGE STATS
                // =========================
                uint32_t entityCount = arche.entityCount;
                // uint32_t chunkCount = arche.GetChunkCount();
                uint32_t capacity = arche.m_capacity;

                ImGui::Text("Entities: %u", entityCount);
                // ImGui::Text("Chunks: %u", chunkCount);
                ImGui::Text("Capacity: %u", capacity);

                // densidade
                float density = capacity > 0
                    ? (float)entityCount / (float)capacity
                    : 0.0f;

                ImGui::Text("Density: %.2f%%", density * 100.0f);

                // =========================
                // VISUAL CHUNKS
                // =========================
                /*
                if (ImGui::TreeNode("Chunks"))
                {
                    for (uint32_t c = 0; c < chunkCount; c++)
                    {
                        const ChunkStats& stats = arche.GetChunkStats(c);

                        ImGui::Text(
                            "Chunk %u -> %u / %u",
                            c,
                            stats.count,
                            stats.capacity
                        );

                        float fill = stats.capacity > 0
                            ? (float)stats.count / stats.capacity
                            : 0.0f;

                        ImGui::ProgressBar(fill, ImVec2(0, 0));
                    }

                    ImGui::TreePop();
                }
                */

                ImGui::TreePop();
            }
        }
    }

    ImGui::SeparatorText("AssetManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Material: %ld", AssetsManager::GetMaterialCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetCount());

    ImGui::SeparatorText("Renderização");

    /*
    Scene* scene = Scene::GetCurrent();
    if (scene != nullptr)
    {
        ImGui::Text("Luzes: %ld",       scene->GetLights().size());
        ImGui::Text("Cameras: %ld",     scene->GetCameras().size());
        ImGui::Text("Objects: %ld",     scene->getObjectList().size());
    }
    */

    ImGui::Text("DrawCall Count: %ld", Status::BatchDrawCallCount);
    ImGui::Text("Render Count: %ld", Status::BatchRenderCount);
}

