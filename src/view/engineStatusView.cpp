#include "view/engineStatusView.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/systemRegistry.hpp"

#include <gr-base/core/threadpool.hpp>
#include <gr-base/core/status.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/profiler.hpp>

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/ecs/world.hpp>

#include <imgui.h>

void EngineStatusView::Draw()
{
    Scene* scene = Scene::GetCurrent();

    if (ImGui::Begin("Status"))
        renderer(scene);
    ImGui::End();
}

void EngineStatusView::renderer(Scene* scene)
{
    ImGui::SeparatorText("World");
    if (ImGui::CollapsingHeader("Components"))
    {
        const auto count = ComponentRegistry::GetComponentCount();
        const auto* components = ComponentRegistry::GetComponents();

        ImGui::Text("Registered: %zu", count);
        ImGui::Separator();

        for (size_t i = 0; i < count; ++i)
        {
            const auto& info = components[i];

            ImGui::Text(
                "[%zu] %s | TypeID=%u | Hash=0x%llX",
                i,
                info.name,
                static_cast<unsigned>(info.index),
                static_cast<unsigned long long>(info.hash)
            );
        }
    }

    if (ImGui::CollapsingHeader("Systems"))
    {
        const auto count = SystemRegistry::GetCount();
        const auto* systems = SystemRegistry::GetSystems();

        ImGui::Text("Registered: %zu", count);
        ImGui::Separator();

        for (size_t i = 0; i < count; ++i)
        {
            const auto& info = systems[i];

            ImGui::Text(
                "[%zu] %s | TypeID=%u | Hash=0x%llX",
                i,
                info.name,
                static_cast<unsigned>(info.index),
                static_cast<unsigned long long>(info.hash)
            );
        }
    }

    if (ImGui::CollapsingHeader("ECS") && scene != nullptr)
    {
        auto& storage = scene->world.GetStorage();

        ImGui::Text("Entities: %ld", scene->GetEntityCount());

        ImGui::Text("Archetypes: %zu", storage.count);

        for (size_t i = 0; i < storage.count; i++)
        {
            const auto& arche = storage.archetypes[i];

            char archename[128];
            sprintf(archename, "Archetype %zu", i);

            ImGui::PushID(archename);

            if (ImGui::CollapsingHeader(archename))
            {
                // =========================
                // COMPONENT TYPES
                // =========================
                ImGui::Text("Component Types: %zu", arche.mask);

                if (ImGui::TreeNode("Types"))
                {
                    Signature mask = arche.mask;
                    while (mask)
                    {
                        TypeID typeID = __builtin_ctzll(mask);

                        if (auto info = ComponentRegistry::GetComponentByID(typeID))
                            ImGui::BulletText("%s (ID: %u)", info->name, typeID);

                        mask &= mask - 1;
                    }

                    ImGui::TreePop();
                }

                // =========================
                // STORAGE STATS
                // =========================

                size_t entityCount =
                    arche.entityCount;

                size_t chunk_capacity =
                    arche.chunkCapacity;

                size_t chunks_count =
                    arche.chunkCount;

                ImGui::Text("Entities: %lu", arche.entityCount);
                ImGui::Text("Chunks: %lu / %lu", chunks_count, chunk_capacity);

                // densidade
                float density = chunk_capacity > 0
                    ? (float)chunks_count / (float)chunk_capacity
                    : 0.0f;

                ImGui::Text("Density: %.2f%%", density * 100.0f);

                // =========================
                // VISUAL CHUNKS
                // =========================
                if (ImGui::TreeNode("Chunks"))
                {
                    for (uint32_t c = 0; c < chunks_count; c++)
                    {
                        auto& chunk = arche.chunks[c];

                        ImGui::Text(
                            "Chunk %u -> %lu / %lu",
                            c,
                            chunk->count,
                            arche.layout->capacity
                        );

                        float fill = arche.layout->capacity > 0
                            ? (float)chunk->count / arche.layout->capacity
                            : 0.0f;

                        ImGui::ProgressBar(fill, ImVec2(0, 0));
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Profiler"))
    {
        for (auto&& [name, timeMs] : Profiler::GetEntries())
        {
            ImGui::Text("%-20s %.3f ms",
                name.c_str(),
                timeMs);
        }
    }

    ImGui::SeparatorText("AssetManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Material: %ld", AssetsManager::GetMaterialCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetCount());
    ImGui::Text("Jobs: %ld", Threadpool::GetJobCount());

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

    ImGui::Text("Draw Calls: %ld", RendererStats::DrawCallCount);
    ImGui::Text("Triangles: %ld", RendererStats::TriangleCount);

    ImGui::Text("Instances: %ld", RendererStats::InstanceCount);
    ImGui::Text("Batches: %ld", RendererStats::BatchCount);
}

