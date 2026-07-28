#include "view/workspace/editorWorkspace.hpp"

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

#include <gr-base/module/moduleloader.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/managers/worldmanager.hpp>
#include <gr-base/managers/eventManager.hpp>

#include <gr-base/ecs/world.hpp>

#include "core/sandbox.hpp"
#include "editorApplication.hpp"

#include "event/editorApplicationEvent.hpp"
#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/string.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-base/script/relationship.hpp>
#include <gr-base/script/name.hpp>

#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/task/modelsavetask.hpp"
#include "Global.h"
#include "core/time.hpp"

#include "script/cameraEditor.hpp"

#include "system/cameraEditorSystem.hpp"

#include <gr-base/application.hpp>

#include "gr-base/core/mainthreaddispatcher.hpp"
// #include "gr-base/core/debugdraw.hpp"
//
#include <gr-base/asset/assetDB.hpp>

#include "gr-base/core/clock.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "manager/viewManager.hpp"
#include "settings.h"

#include "task/filmboxLoadTask.hpp"

#include "view/editor/consoleWindowView.hpp"
#include "view/engineStatusView.hpp"
#include "view/loadingTaskPopup.hpp"
// #include "view/popup/filmboximport.hpp"
#include "view/sceneInspectorView.hpp"
#include "view/spawnObject.hpp"
// #include "view/worldManagerView.hpp"
#include "view/imguiWidget.hpp"


#include <gr-base/core/window.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/managers/assetsManager.hpp>
#include <gr-base/managers/sceneManager.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/display.hpp>
#include <gr-base/core/scene.hpp>

#include "plugin/pluginManager.hpp"

#include "core/screen.hpp"

#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include <gr-base/core/gameObject.hpp>

#include "guizmo/ImGuizmo.h"

#include "core/project.hpp"

#include <gr-render/texture.h>

#include <gr-base/core/rendertarget.hpp>

#include <gr-base/prefab/shaderprefab.hpp>

#include <gr-base/system/transformsystem.hpp>
#include <gr-base/system/renderSystem.hpp>
#include <gr-base/system/camerasystem.hpp>


#include <gr-base/task/textureBuildTask.hpp>
#include <gr-base/task/shaderbuildtask.hpp>

constexpr Hash64 kBuiltinSystems[] = {
    engine::type_traits::hash<CameraEditorSystem>,
};

#define GET_N_ARRAY(c) sizeof(c) / sizeof(c[0])

enum class DragDropFlags : uint32_t {
    GMDL,
    SCN,
    SCRIPT,
    SHADER,
    GAMEOBJECT,
    ANIMATION,
    TEXTURE,
    MESH
};

#include <gr-base/core/binary/gHeader.hpp>

static std::unordered_map<DragDropFlags, const char *> m_extensionDragDrop =
{
    {DragDropFlags::GMDL,       HEADER_MODEL_EXTENSION},
    {DragDropFlags::SCN,        ".scn"},
    {DragDropFlags::SCRIPT,     ".script"},
    {DragDropFlags::SHADER,     ".shader"},
    {DragDropFlags::GAMEOBJECT, ".gameobject"},
    {DragDropFlags::ANIMATION,  ".anm"},
    {DragDropFlags::TEXTURE,    ".png"},
    {DragDropFlags::MESH,       ".gmsh"},
};

static std::unordered_map<std::string, DragDropFlags> m_extensionNameToFlag =
{
    {HEADER_MODEL_EXTENSION,  DragDropFlags::GMDL},
    {".scn",        DragDropFlags::SCN},
    {".cpp",        DragDropFlags::SCRIPT},
    {".hpp",        DragDropFlags::SCRIPT},
    {".glsl",       DragDropFlags::SHADER},
    {".gameobject", DragDropFlags::GAMEOBJECT},
    {".anm",        DragDropFlags::ANIMATION},
    {".png",        DragDropFlags::TEXTURE},
    {".gmsh",       DragDropFlags::MESH},
};

#include <gr-base/task/shaderbuildtask.hpp>

EditorWorkspace::EditorWorkspace()
    : m_viewport(ImGui::GetMainViewport()),
    mImGuiIO(&ImGui::GetIO()),
    m_plugin(nullptr),
    m_state(EngineState::Edit)
{
    m_cameraObjectSelected = nullptr;

    m_scene = nullptr;

    s_texturePlayGame = HANDLE_INVALID;

    g_treefile_dir_selected = nullptr;
    
    popupFlags = 0;

    FPSColor = {1.0f, 1.0f, 1.0f, 1.0f};

    fpsFrameCount = 0;
    fpsTimer = 0;

    fps = 0;

    m_guizmoOperation = ImGuizmo::TRANSLATE;

    m_guizmoManipulate = ImGuizmo::WORLD;

    m_backbuffer = std::make_shared<RenderTarget>(1024, 1024);
    Renderer::SetRenderTarget(m_backbuffer.get());

    EditorApplication* engine = static_cast<EditorApplication*>(Application::application);

    m_project = engine->getProject();


#define INSERT_SCRIPT(object) \
    {#object, [this](GameObject *go) \
        { \
            RenderProperty<object>(&EditorWorkspace::OnProperty##object, this); \
        } \
    }

    m_guiscriptmap = {
        {"Transform", [this](GameObject *go)
            {
                RenderProperty<Transform>(&EditorWorkspace::OnPropertyTransform, this);
            }
        },
        INSERT_SCRIPT(Camera),
        INSERT_SCRIPT(Light),
        INSERT_SCRIPT(MeshRender),
        // INSERT_SCRIPT(SkinnedMeshRender),
        INSERT_SCRIPT(MeshFilter),
        // INSERT_SCRIPT(LODComponent),
        // INSERT_SCRIPT(MeshCollider),
        // INSERT_SCRIPT(BoxCollider),
        // INSERT_SCRIPT(Rigidbody),
        // INSERT_SCRIPT(Animator),
    };
#undef INSERT_SCRIPT
    
    m_viewport = ImGui::GetMainViewport();

    SystemRegistry::Register<CameraEditorSystem>();
    ComponentRegistry::Register<CameraEditor>(false);

    initialize();

    AssetDB::Load(m_project->path + "/asset.db");

    /*
    {
        Entity gameObjectID = GameObject::Create("Camera");

        GameObject* gameObject = GameObject::Get(gameObjectID);
        gameObject->addComponent<Camera>();

        m_cameraID = gameObject->GetID<Camera>();
    }
    */

    bBuildDock = gfile::exists("imgui.ini");

    LoadReources();

    try
    {
        auto libdir = m_project->path + "/.build/lib/libscript.so";
        if (gfile::exists(libdir))
            ModuleLoader::Load("script", libdir);
    } catch (const std::exception &e)
    {
        Console::LogError("Fatal error in script: %s", e.what());
    }

    root_tree_file.path = m_project->path + "/assets";
    root_tree_file.name = "Assets";
    root_tree_file.parent = nullptr;
    root_tree_file.bdirectory = true;

    updateFileTree(&root_tree_file);

    Window::SetWindowTitle(String::format("GR - %s", m_project->name.c_str()).c_str());

    // EventManager::Attach("delete_file_selected", &EditorWorkspace::delete_file_selected, this);
    // EventManager::Attach("editor_update_tree_file", &EditorWorkspace::updateFileTree, this, &root_tree_file);

    TaskManager::Add<ShaderBuildTask>(
        "/home/NoirVoid/.local/share/gr-engine/assets/shader/line.glsl",
        URI::parse("engine://shader/line")
    );

    TaskManager::Add<ShaderBuildTask>(
            "/home/NoirVoid/.local/share/gr-engine/assets/shader/model/simple_model.glsl",
            URI::parse("engine://shader/simple_model")
        );
}

EditorWorkspace::~EditorWorkspace()
{
    AssetDB::Save(m_project->path + "/asset.db");

    for (auto& scene : SceneManager::GetScenes())
    {
        for (auto systemID : scene->GetSystemIDs())
            scene->DelSystemByID(systemID);
    }

    ModuleLoader::Unload("script");

    shutdown();
}

void* GetVTable(const SystemInfo* info)
{
    ISystem* system = (ISystem*)malloc(info->stride);

    ((void (*)(ISystem*, const ISystem*))(info->constructor))(system, nullptr);
    void* vtable = *(void**)system;

    ((void (*)(ISystem*))(info->constructor))(system);

    free(system);
    return vtable;
}

void EditorWorkspace::loadScript(const std::string& dir)
{
    std::string file = dir + "/libscript.so";
    if (!gfile::exists(file))
        return;

    Sandbox sandbox(file);
    if (!sandbox.Run())
        return;

    std::unordered_map<Hash64, const SystemInfo*> newSystemMap;
    for (auto& systems : kBuiltinSystems)
        newSystemMap.emplace(systems, nullptr);

    for (auto& info : sandbox.systems)
        newSystemMap.emplace(info.hash, &info);

    std::unordered_map<TypeID, void*> systemAliveMap;
    std::unordered_set<TypeID> systemReload;

    for (size_t i=0;i<SystemRegistry::GetCount();i++)
    {
        auto& info = SystemRegistry::GetSystems()[i];

        void* vtable = nullptr;

        bool alive = newSystemMap.find(info.hash) != newSystemMap.end();
        if (alive)
        {
            auto new_info = newSystemMap[info.hash];

            vtable = GetVTable(&info);
            if (new_info != nullptr && info.stride != new_info->stride)
            {
                systemReload.emplace(info.index);

                std::cout << "REINSTANTIATE: " << info.name << std::endl;

                vtable = nullptr;
            }
        } else
        {
            Console::LogWarning("[ Script ] Removed system: %s", info.name);
        }

        systemAliveMap.emplace(info.index, vtable);

        newSystemMap.erase(info.hash);
    }

    // Reinserção dos novos systemas
    std::unordered_map<Scene*, std::vector<TypeID>> systemsToInstantiate;
    for (auto& systemID : systemReload)
    {
        for (auto& scene : SceneManager::GetScenes())
        {
            auto system = scene->GetSystem(systemID);
            if (system == nullptr)
                continue;

            systemsToInstantiate[scene].push_back(systemID);
        }
    }

    // Remo das cenas os systemas que não existe na nova build
    for (auto &&[id, vtable] : systemAliveMap)
    {
        if (vtable == nullptr)
        {
            for (auto& scene : SceneManager::GetScenes())
                scene->DelSystemByID(id);
        }
    }

    // liberar o modulo da memoria
    ModuleLoader::Unload("script");

    // Verica se ainda é valido na memoria
    if (ModuleLoader::CheckModuleHasInMemory("libscript.so"))
        return Console::LogError("[ Script ] Fatal error data not unloaded.");

    // Recarrega o modulo com novos dados
    ModuleLoader::Load("script", file);

    // Apenas para log, imprime novos modulos se houver
    for (auto &&[hash, info] : newSystemMap)
        Console::LogInfo("[ Script ] Added system: %s", info->name);

    // Faz checagem de vtable
    for (auto &&[systemID, oldVTable] : systemAliveMap)
    {
        if (oldVTable != nullptr)
        {
            auto info = SystemRegistry::GetSystemByID(systemID);

            void* newVTable = GetVTable(info);
            if (oldVTable != newVTable)
            {
                for (auto& scene : SceneManager::GetScenes())
                {
                    ISystem* system = scene->GetSystem(systemID);

                    void** vtable = (void**)(system);
                    *vtable = newVTable;
                }

                Console::LogWarning("[ Script ] VTable realocated for system: %s", info->name);
            }
        }
    }

    for (auto&& [scene, systemIDs] : systemsToInstantiate)
    {
        for (auto& systemID : systemIDs)
            scene->AddSystemByID(systemID);
    }
}

// ===================================== END =========================================================
void EditorWorkspace::Update()
{
    if (m_scene != nullptr)
        m_scene->Update();
}

/*
void EditorWorkspace::onRender3D()
{
}
*/

void EditorWorkspace::DrawWorkspace()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    create_dock();

    viewManager.renderer();

    create_window_viewport();

    create_window_hierarchy();

    create_window_property();

    create_call_popup();

    create_popup();
}

/*
void EditorWorkspace::onFocus()
{
    // m_scene = Scene::GetCurrentScene();
}

void EditorWorkspace::onBlur()
{}
*/
void EditorWorkspace::dropPath(const std::vector<std::string>& entries)
{
    for (auto& file : entries)
    {
        if (gfile::is_directory(file))
            continue;

        std::string filename = gfile::get_file_name(file);
        if (filename.find(".fbx") != std::string::npos)
            TaskManager::Add<FilmboxLoadTask>(file);
        else if (
                filename.find(".png") != std::string::npos ||
                filename.find(".jpg") != std::string::npos
            )
        {
            std::string parent = gfile::parent(file);

            std::string path =
                "texture/" +
                gfile::get_file_name(parent) +
                "/" +
                gfile::get_file_name(file);

            URI uri = URI::parse("project://" + path);

            TaskManager::Add<TextureBuildTask>(file, uri);
        }
    }
}

// void EditorWorkspace::drop(void *data)
// {
//     updateFileTree(&root_tree_file);
// }

#include <gr-base/core/renderer.hpp>

#include "view/importAssetViewPopup.hpp"
#include "view/browserFile.hpp"

void EditorWorkspace::initialize()
{
    viewManager.getOrAddView<LoadingTaskPopup>();
    viewManager.getOrAddView<ImportAssetViewPopup>();

    viewManager.show<ConsoleWindowView>();
    viewManager.show<BrowserFile>();
}

void EditorWorkspace::shutdown()
{
    // AssetDB::Shutdown();
}

void EditorWorkspace::create_dock()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Close"))
                EventManager::NotifyAsync<EditorApplicationEvent_onCloseProject>(nullptr);

                // MainThreadDispatcher::Add(Priority::LOW, &Application::init_project, &Application::GetInstance(), "");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("Bake"))
            {
                /*
                for (auto& gameObjectID : m_scene->getObjectList())
                {
                    auto gameobject = GameObject::Get(gameObjectID);

                    auto transform = gameobject->GetTransform();
                    auto position = transform->getPosition();

                    int chunkIdX = floor(position.x / CHUNK_SIZE);
                    int chunkIdZ = floor(position.z / CHUNK_SIZE);

                    std::string key = std::to_string(chunkIdX) + "_" + std::to_string(chunkIdZ);

                    ChunkID chunkID = ChunkManager::Find(key);
                    if (chunkID == INVALID_CHUNK)
                    {
                        chunkID = ChunkManager::Create(key);

                        StreamChunk* chunk = ChunkManager::Get(chunkID);
                        chunk->position = Vector2{(float)chunkIdX, (float)chunkIdZ};
                        chunk->objects = nullptr;
                        chunk->dirty = false;
                    }

                    StreamChunk* chunk = ChunkManager::Get(chunkID);
                    chunk->dirty = true;
                    vector_push(chunk->objects, gameObjectID);
                }
                */
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            {
                bool visible = viewManager.isVisible<SceneInspectorView>();
                if (ImGui::MenuItem("Scene Inspector", nullptr, &visible))
                    viewManager.setVisible<SceneInspectorView>(visible);
            }

            {
                bool visible = viewManager.isVisible<BrowserFile>();
                if (ImGui::MenuItem("Browser File", nullptr, &visible))
                    viewManager.setVisible<BrowserFile>(visible);
            }

            {
                bool visible = viewManager.isVisible<SpawnObject>();
                if (ImGui::MenuItem("Spawn Object", nullptr, &visible))
                    viewManager.setVisible<SpawnObject>(visible);
            }

            {
                bool visible = viewManager.isVisible<EngineStatusView>();
                if (ImGui::MenuItem("Status", nullptr, &visible))
                    viewManager.setVisible<EngineStatusView>(visible);
            }

            {
                bool visible = viewManager.isVisible<ConsoleWindowView>();
                if (ImGui::MenuItem("Console", nullptr, &visible))
                    viewManager.setVisible<ConsoleWindowView>(visible);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // PopupMenuProperty();
    
    if (bBuildDock)
        return;

    // BuildDock(dockID, 0);

    bBuildDock = true;
}

void EditorWorkspace::BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags)
{
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, m_viewport->Size);

	// auto dock_id_center = ImGui::DockBuilderGetCentralNode(dockspace_id);
	auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);
	auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
	auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.15f, nullptr, &dockspace_id);
	auto dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_down, ImGuiDir_Down, 0.2f, nullptr, &dock_id_down);
	auto dock_id_left_down_right = ImGui::DockBuilderSplitNode(dock_id_left_down, ImGuiDir_Right, 0.2f, nullptr, &dock_id_left_down);

    // Right
	ImGui::DockBuilderDockWindow("Propriedades", dock_id_right);

    // Left
	ImGui::DockBuilderDockWindow("Cenario", dock_id_left);
	ImGui::DockBuilderDockWindow("Navegador", dock_id_left_down);
	ImGui::DockBuilderDockWindow("Console", dock_id_left_down_right);

	ImGui::DockBuilderDockWindow("Viewport", dockspace_id);

	ImGui::DockBuilderFinish(dockspace_id);
}

void EditorWorkspace::create_window_hierarchy()
{
    if (ImGui::Begin("hierarchy") && m_scene != NULL)
        create_render_hierarchy(m_scene);

    ImGui::End();
}

void EditorWorkspace::create_render_hierarchy(Scene* scene)
{
    std::string name = SceneManager::GetSceneName(m_scene);

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
    {
        drop_gameobject([&](Entity entity)
        {
            scene->Unlink(entity);
            scene->SetParent(scene->GetRoot(), entity);
        });

        RenderSceneObject(scene);

        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && scene->has_object_selected())
    {
        for (auto go : scene->get_object_selected())
            scene->destroy(go);

        scene->deselect_all();


        GameObject root(scene, scene->GetRoot());

        auto rel = root.GetComponent<Relationship>();
        if (rel != nullptr)
        {
            Entity child = rel->firstChild;

            if (child != INVALID_ENTITY)
                scene->select(child);
        }
    }
}

void EditorWorkspace::drop_gameobject(std::function<void (Entity)> func)
{
    if (!ImGui::BeginDragDropTarget())
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
        return ImGui::EndDragDropTarget();

    auto entity = *reinterpret_cast<Entity*>(payload->Data);
    
    func(entity);

    ImGui::EndDragDropTarget();
}

void EditorWorkspace::RenderSceneObject(Scene *scene)
{
    if (scene->hierarchyDirty)
    {
        RebuildHierarchyVector();

        scene->hierarchyDirty = false;
    }

    ImGui::BeginChild("SceneObjectsScroll");

    ImGuiListClipper clipper;
    clipper.Begin((int)uiList.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            GameObject go(scene, uiList[i]);
            ShowGameObjectHierarchy(go);
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        popupFlags |= EditorPopupFlags_SceneAddOption;

    if (ImGui::IsKeyReleased(ImGuiKey_F2) && ImGui::IsWindowHovered() && scene->has_object_selected())
        popupFlags |= EditorPopupFlags_RenameGameObject;

    if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered())
        scene->deselect_all();

    ImGui::EndChild();
}

void EditorWorkspace::ShowGameObjectHierarchy(GameObject& go)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto name = go.GetName();

    // auto rel = ;

    if (go.scene->object_has_been_selected(go.entity))
        flags |= ImGuiTreeNodeFlags_Selected;

    if (go.HasComponent<Relationship>() && go.GetComponent<Relationship>()->firstChild != INVALID_ENTITY)
        flags &= ~ImGuiTreeNodeFlags_Leaf;

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        if (go.scene->object_has_been_selected(go.entity))
        {
            go.scene->deselect(go.entity);
        } else
        {
            go.scene->select(go.entity);
        }
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        go.scene->deselect_all();

        go.scene->select(go.entity);
    }

    dragDropGameObject(go.entity, name);
    
    if (isOpen)
    {
        if (go.HasComponent<Relationship>())
        {
            auto rel = go.GetComponent<Relationship>();

            GameObject child(go.scene, rel->firstChild);

            while (child.IsAlive())
            {
                ImGui::PushID(go.entity);
                ShowGameObjectHierarchy(child);
                ImGui::PopID();

                if (!child.HasComponent<Relationship>())
                    break;

                child.entity = child.GetComponent<Relationship>()->nextSibling;
            }
        }

        ImGui::TreePop();
    }
}

bool EditorWorkspace::dragDropGameObject(Entity entity, const std::string &name)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
    {
        ImGui::Text("Moving \"%s\"", name.c_str());
        ImGui::SetDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT], &entity, sizeof(Entity));
        ImGui::EndDragDropSource();
    }

    if (!ImGui::BeginDragDropTarget())
        return false;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
    {
        ImGui::EndDragDropTarget();
        return false;
    }

    Entity child = *reinterpret_cast<Entity*>(payload->Data);
    if (entity != child)
    {
        m_scene->Unlink(child);

        m_scene->SetParent(entity, child);
    }

    ImGui::EndDragDropTarget();

    return true;
}

void EditorWorkspace::PopupMenuAdd()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
    if (!isOpen)
        return;

    static std::vector<const char*> items {"Empty GameObject"};
    int id = 0;

    ImGui::Text("Add Object");
    if (ImGui::ListBox("##popup_menu_menu_add_list_box", &id, items.data(), items.size()))
    {
        switch (id)
        {
        case 0:
            PostLoadGameObject(m_scene->create("Empty "));
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void EditorWorkspace::create_window_viewport()
{
    if (ImGui::Begin("viewport"))
        create_render_viewport();
    ImGui::End();
}

void EditorWorkspace::create_render_viewport()
{
    if (!ImGui::BeginTabBar("##create_render_viewport_tabbar_0"))
        return;

    for (auto &scene : SceneManager::GetScenes())
    {
        auto name = SceneManager::GetSceneName(scene);

        bool bactive = true;
        if (ImGui::BeginTabItem(name, &bactive))
        {
            if (scene != m_scene)
            {
                if (m_scene != nullptr)
                    m_scene->deselect_all();

                m_scene = scene;
            }

            ImGui::EndTabItem();
        }

        if (!bactive)
        {
            m_scene = nullptr;
            
            SceneManager::SetActive(nullptr);

            SceneManager::Remove(scene);
        }
    }

    if (ImGui::TabItemButton("+"))
        popupFlags |= EditorPopupFlags_CreateScene;

    ImGui::EndTabBar();
    
    drop_scene_file_in_viewport();

    if (m_scene != nullptr)
        render_scene(m_scene);
}

void EditorWorkspace::drop_scene_file_in_viewport()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SCN]);
    if (payload == nullptr)
        return ImGui::EndDragDropTarget();

    std::vector<gtreefile *> files(payload->DataSize / sizeof(gtreefile *), nullptr);
    memcpy(files.data(), payload->Data, payload->DataSize);

    // for (auto file : files)
    //     AssetsManager::LoadScene(file->path, &EditorWorkspace::set_scene, this);

    ImGui::EndDragDropTarget();
}

bool gluInvertMatrix(const float m[16], float invOut[16]) {
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

void EditorWorkspace::render_scene(Scene *scene)
{
    UpdateResizeSceneFramebuffer();

    // EditorController::bCanMove = ImGui::IsWindowHovered();

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

    // Play button
    if (s_texturePlayGame.get() != nullptr)
    {
        s_texturePlayGame = iconsUI[ICON_PLAY_GAME];
        if (Settings::bGameRunning) {
            if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
                OnStopButton();
            } else {
                s_texturePlayGame = iconsUI[ICON_STOP_GAME];
            }
        }
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorPos.y + 0.5f});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->id), ImVec2(16, 16)))
        {
            if (!Settings::bGameRunning)
            {
                OnPlayButton();
            } else {
                OnStopButton();
            }
        }
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, regionAvail.x, regionAvail.y);

    GameObject obj(m_scene, m_scene->GetCamera());
    if (obj.IsAlive())
    {
        regionAvail = ImGui::GetContentRegionAvail();
        cursorPos = ImGui::GetCursorPos();

        Camera* camera = obj.GetComponent<Camera>();
        if (camera != nullptr)
        {
            RenderTarget* rendertarget = camera->rendertarget;

            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

            drop_gameobject_in_scene();

            if (scene->has_object_selected() && !Settings::bGameRunning)
                create_render_guizmo(scene, &obj);
        }
    }

    // Fps Draw
    {
        fpsTimer += static_cast<float>(Clock::deltaTime);
        fpsFrameCount++;

        if (fpsTimer >= 1.0f)
        {
            fps = fpsFrameCount;
            fpsFrameCount = 0;
            fpsTimer = 0.0f;
        }

        char fps_text[256] = {0};
        sprintf(fps_text, "FPS: %d", fps);

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos(ImVec2(regionAvail.x - textSize.x, cursorPos.y + textSize.y));

        ImGui::TextColored(FPSColor, "%s", fps_text);
    }

    ImGui::SetCursorPos({8.0f, cursorPos.y});

    ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 1.0f, 1.0f});

    ImGui::BeginGroup();
    if (ImGui::RadioButton("Local", m_guizmoManipulate == ImGuizmo::LOCAL))
        m_guizmoManipulate = ImGuizmo::LOCAL;
    
    ImGui::SameLine();
    if (ImGui::RadioButton("World", m_guizmoManipulate == ImGuizmo::WORLD))
        m_guizmoManipulate = ImGuizmo::WORLD;

    ImGui::EndGroup();

    ImGui::PopStyleColor();

    if (m_scene->has_object_selected() && ImGui::IsWindowFocused() && mImGuiIO->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D))
    {
        auto temp = m_scene->get_object_selected();
        m_scene->deselect_all();

        for (auto go : temp)
            m_scene->select(m_scene->create(go));
    }

    if (ImGui::IsWindowFocused() && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S))
    {
        // std::string path = m_project->path + "/assets/scene";

        // for (auto &scene : SceneManager::GetScenes())
            // TaskManager::Add<SceneSaveTask>(path + "/" + SceneManager::GetSceneName(scene) + ".scn", scene);
    }
}

void EditorWorkspace::UpdateResizeSceneFramebuffer()
{
    GameObject obj(m_scene, m_scene->GetCamera());
    if (!obj.IsAlive())
        return;

    auto camera = obj.GetComponent<Camera>();

    auto rendertarget = camera->rendertarget;
    float width = static_cast<float>(rendertarget->GetWidth());
    float height = static_cast<float>(rendertarget->GetHeight());

    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (camera != nullptr && (width != regionAvail.x || height != regionAvail.y && regionAvail.x > 5.0f && regionAvail.y > 5.0f))
    {
        if (regionAvail.x > 0 && regionAvail.y > 0)
        {
            Display::SetDisplayRect(Rect{0.0f, 0.0f, regionAvail.x, regionAvail.y});

            rendertarget->resize(static_cast<int>(regionAvail.x), static_cast<int>(regionAvail.y));
        }
    }
}

void EditorWorkspace::create_render_guizmo(Scene *scene, const GameObject* go)
{
    if (ImGui::IsWindowHovered() && !mImGuiIO->KeyShift && !mImGuiIO->KeyCtrl)
    {
        if (ImGui::IsKeyReleased(ImGuiKey_G))
        {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
    
        if (ImGui::IsKeyReleased(ImGuiKey_R))
        {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
    
        if (ImGui::IsKeyReleased(ImGuiKey_S))
        {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
    }

    auto camera = go->GetComponent<CameraData>();

    auto viewmatrix = camera->view;
    auto projection = camera->projection;

    // static bool IsMoving = false;
    // static std::unordered_set<Transform *> nodeup;
    for (auto entity : scene->get_object_selected())
    {
        GameObject entitySelected(scene, entity);
        if (!entitySelected.IsAlive())
            continue;

        ImGuizmo::SetID(entity);
        auto transform = entitySelected.GetComponent<Transform>();

        auto manipulatedWorld = transform->worldMatrix;

        bool changed = ImGuizmo::Manipulate(&viewmatrix[0][0], &projection[0][0], m_guizmoOperation, m_guizmoManipulate, &manipulatedWorld[0][0]);
        if (changed)
        {
            scene->MarkChanged<Transform>(entity);

            Matrix4x4 localMatrix = manipulatedWorld;

            Relationship* rel = scene->GetWorldDynamic()->GetComponent<Relationship>(entity);

            if (rel != nullptr && rel->parent != INVALID_ENTITY)
            {
                auto* parentTransform =
                    scene->GetWorldDynamic()->GetComponent<Transform>(rel->parent);

                if (parentTransform != nullptr)
                {
                    Matrix4x4 invParent =
                        type_traits<Matrix4x4>::identity;

                    if (gluInvertMatrix(
                            &parentTransform->worldMatrix[0][0],
                            &invParent[0][0]))
                    {
                        localMatrix = invParent * manipulatedWorld;
                    }
                }
            }

            Math::mat4_decompose(
                    localMatrix,
                    transform->scale,
                    transform->rotation,
                    transform->position
                );

            m_scene->MarkChildrenDirty(entity);
        }

        /*
        static bool isDragging = false;
        if (ImGuizmo::IsUsing() && !isDragging) 
        {
            isDragging = true;
        }
        if (isDragging && !ImGuizmo::IsUsing()) 
        {
            isDragging = false;
        }
        */
    }
}

void EditorWorkspace::OnPlayButton()
{
    Threadpool::AddTask(&EditorWorkspace::OnBuildScript, this);
}

#include "core/cmd/cmd.hpp"
void EditorWorkspace::OnBuildScript()
{
    /*
    Console::LogInfo("[ Build ] Starting pre-build scripting...");

    auto builddir = m_project->path + "/.build";
    if (!gfile::exists(builddir))
    {
        if (!gfile::create_directory(builddir))
            return Console::LogError("[ Build ] fail to create directory: %s", builddir.c_str());

        auto templatedir = gfile::get_data_directory() + "/assets/script";

        gfile::copy(templatedir.c_str(), builddir.c_str());
    }

    auto cmakebuild = builddir + "/build";
    if (!gfile::exists(cmakebuild))
    {
        if (!gfile::create_directory(cmakebuild))
            return Console::LogError("[ Build ] fail to create directory: %s", cmakebuild.c_str());

        std::string cmd = "cd " + cmakebuild + " && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..";

        bool retVal = Command::exec(cmd, [](std::string line)
        {
            Console::LogWarning(line);
        });

        if (!retVal)
            return Console::LogError("[ Build ] Falied in to configure build");
    }

    std::string cmd = "cmake --build " + cmakebuild;

    bool retVal = Command::exec(cmd, [](std::string line)
    {
        Console::LogWarning(line);
    });

    if (!retVal)
        return Console::LogError("[ Build ] Falied in to build script");

    Console::LogInfo("[ Build ] Script compiled sucess fully.");

    MainThreadDispatcher::Add(Priority::NORMAL, &EditorWorkspace::loadScript, this, builddir + "/lib");
    */
}

void EditorWorkspace::OnPostSaveSnapshot(std::string file)
{
    std::cout << "PlayGame" << std::endl;

    // m_playMode = std::make_unique<EditorRuntime>();
    // m_runtime = m_playMode.get();
    // m_runtime->initialize();

    m_scene = nullptr;
}

void EditorWorkspace::OnPostLoadSnapshot(Scene* scene)
{
    m_scene = scene;

    Settings::bGameRunning = true;

    TaskManager::Add<PhysicsTask>();

    // ComponentStorage::Start();

    // set_scene(scene);
}

void EditorWorkspace::OnStopButton()
{
    /*
    MainThreadDispatcher::Add(Priority::HIGHT, [&]
    {

        if (m_scene != nullptr)
            m_scene->deselect_all();

        m_state = EngineState::Edit;

        m_playMode = nullptr;

        m_runtime = m_editMode.get();
        m_runtime->initialize();

        m_scene = SceneManager::GetCurrent();
        m_scene->SetCamera(m_cameraID);

        Settings::bGameRunning = false;

        Console::LogDefault("Game stoped.");
    });
    */
}

void EditorWorkspace::drop_gameobject_in_scene()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
        return;

    auto payload = ImGui::AcceptDragDropPayload("AssetID");
    if (payload == nullptr)
        return ImGui::EndDragDropTarget();

    std::vector<AssetID> assets(payload->DataSize / sizeof(AssetID), INVALID_ASSET);
    memcpy(assets.data(), payload->Data, payload->DataSize);

    for (auto &assetID : assets)
    {
        auto info = AssetDB::GetAsset(assetID);
        if (info == nullptr)
            continue;

        if (info->type== AssetType_Model)
            AssetsManager::LoadGameObjectAsync(info->uri, &EditorWorkspace::PostLoadGameObject, this);

        /*
        if (file->extension == HEADER_MODEL_EXTENSION)

        */
    }

    unselect_all_file();

    ImGui::EndDragDropTarget();
}

void EditorWorkspace::create_window_property()
{
    if (ImGui::Begin("property", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar) &&
            m_scene != nullptr &&
            m_scene->has_object_selected()
        )
        create_render_property();

    ImGui::End();
}

void EditorWorkspace::create_render_property()
{
    GameObject go(m_scene, m_scene->get_object_selected().back());
    if (!go.IsAlive())
        return;

    auto objName = go.GetName();

    ImGui::SeparatorText(objName.c_str());

    bool isStatic = false; // go->GetIsStatic();
    if (ImGui::Checkbox("static", &isStatic))
    {
        // go->SetIsStatic(isStatic);

        // RenderManager::SwitchTo(go->getCacheID(), isStatic);
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Definir como Estático");

    for (auto typeID : m_scene->GetWorldDynamic()->GetAllComponentType(go.entity))
        ShowComponentInfoUI(go, ComponentRegistry::GetComponentByID(typeID));

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_AddComponent;
}

void EditorWorkspace::create_call_popup()
{
    switch (popupFlags)
    {
    case EditorPopupFlags_LoadPlugin:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_LoadPlugin));
        break;
    case EditorPopupFlags_AddComponent:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
        break;
    case EditorPopupFlags_PluginEditor:
        ImGui::OpenPopup("##popup_plugin_editor");
        break;
    case EditorPopupFlags_CreateMaterial:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
        break;
        /*
    case EditorPopupFlags_FileManager | EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager));
        popupFlags &= ~(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager);
        break;
        */
    case EditorPopupFlags_FileManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager));
        break;
    case EditorPopupFlags_CurrentDirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CurrentDirManager));
        break;
    case EditorPopupFlags_RenameFileOrDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameFileOrDir));
        break;
    case EditorPopupFlags_CreateNewDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateNewDir));
        break;
    case EditorPopupFlags_CreateScript:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScript));
        break;
    case EditorPopupFlags_CreateShader:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateShader));
        break;
    case EditorPopupFlags_RenameGameObject:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
        break;
    case EditorPopupFlags_SceneAddOption:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
        break;
    case EditorPopupFlags_CreateScene:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
        break;
    case EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
        break;
    default:
        break;
    }

    popupFlags = 0;
}

void EditorWorkspace::create_popup()
{
    create_popup_create_scene();

    create_popup_add_component();

    create_popup_rename_object();

    create_popup_create_script();

    create_popup_rename_filedir();

    create_popup_info_dir();

    create_popup_dir_manager();

    create_popup_new_material();

    PopupMenuAdd();
}

void EditorWorkspace::create_popup_create_scene()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
    if (!isOpen)
        return;

    ImGui::Text("Input scene name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    std::vector<char> buffer(256, '\0');
    if (ImGui::InputText("##popup_create_scene_inputtext", buffer.data(), buffer.size() - 1, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        std::string name = buffer.data();

        m_scene = SceneManager::Create(name);
        SceneManager::SetActive(m_scene);

        m_scene->AddSystem<CameraEditorSystem>();
        m_scene->AddSystem<TransformSystem>();
        m_scene->AddSystem<CameraSystem>();

        m_scene->AddSystem<RenderSystem>();

        Entity camera = m_scene->create("MainCamera");
        {
            GameObject go(m_scene, camera);
            go.AddComponent<CameraData>();
            go.GetOrAddComponent<Camera>()->rendertarget = m_backbuffer.get();
        }

        m_scene->SetCamera(camera);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_add_component()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
    if (!isOpen)
        return;

    auto go = m_scene->get_object_selected().back();

    std::vector<std::string> m_componentName;

    auto infos = ComponentRegistry::GetComponents();

    for (size_t id=0;id<ComponentRegistry::GetComponentCount();id++)
    {
        auto& info = infos[id];

        m_componentName.push_back(info.name);
    }

    std::sort(m_componentName.begin(), m_componentName.end(), [](const auto &lhs, const auto &rhs) {
        return lhs < rhs;
    });

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    static char componentName[256] = {'\0'};
    auto bSelect = ImGui::InputText("##popup_add_componnet_inputtext_filter", componentName, sizeof(componentName), ImGuiInputTextFlags_EnterReturnsTrue);

    m_componentName.erase(std::remove_if(m_componentName.begin(), m_componentName.end(), [] (auto &name) {
        return std::strstr(name.c_str(), componentName) == nullptr;
    }), m_componentName.end());

    auto regionAvail = ImGui::GetContentRegionAvail();

    for (auto it = m_componentName.begin(); it != m_componentName.end(); ++it)
    {
        bool isFirst = (it == m_componentName.begin());
        if (ImGui::Selectable(it->c_str(), isFirst) || bSelect && go != HANDLE_INVALID)
        {
            GameObject gameObject(m_scene, go);

            gameObject.AddComponentByName(it->c_str());

            std::memset(componentName, '\0', sizeof(componentName));

            ImGui::CloseCurrentPopup();
            
            break;
        }
    }
    
    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_rename_object()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
    if (!isopen)
        return;

    ImGui::SeparatorText("[ Rename ]");

    Entity entity = m_scene->get_object_selected().back();

    GameObject go(m_scene, entity);

    // auto object = GameObject::Get();
    auto entityName = go.GetName();

    char newName[256] = {0};
    strcpy(newName, entityName.c_str());

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    if (ImGui::InputText("##create_popup_rename_object_inputtext_0", newName, sizeof(newName), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        auto n = go.GetOrAddComponent<Name>();
        if (n->value != nullptr)
            free(n->value);

        n->value = strdup(newName);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_create_script()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScript));
    if (!isOpen)
    {
        return;
    }

    auto &file_tree = g_tree_file_selected.back();

    ImGui::Text("Input your script name");

    char name[256] = {0};
    if (ImGui::InputText("##input_text_popup_create_script_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        updateFileTree(file_tree);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_rename_filedir()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_RenameFileOrDir));
    if (!isOpen)
    {
        return;
    }

    auto &tree_file = g_tree_file_selected.back();

    ImGui::Text("Rename:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere(0);

    auto filename = tree_file->name;
    char name[256] = {0};
    strcpy(name, filename.c_str());

    if (ImGui::InputText("##popup_rename_file", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        // File::rename(tree_file->path, tree_file->parent->path + "/" + name);

        updateFileTree(tree_file->parent);
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_info_dir()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CurrentDirManager));
    if (!isopen)
        return;

    if (ImGui::Selectable("Create directory"))
        popupFlags |= EditorPopupFlags_CreateNewDir;

    if (ImGui::Selectable("Create script"))
        popupFlags |= EditorPopupFlags_CreateScript;

    if (ImGui::Selectable("Create material"))
        popupFlags |= EditorPopupFlags_CreateMaterial;

    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_dir_manager()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
    if (!isopen)
        return;

    if (ImGui::Selectable("Create directory"))
    {
        popupFlags |= EditorPopupFlags_CreateNewDir;
    }

    if (ImGui::Selectable("Remove"))
    {
        gfile::remove(g_treefile_dir_selected->path);

        g_treefile_dir_selected = g_treefile_dir_selected->parent;

        updateFileTree(g_treefile_dir_selected);
    }

    ImGui::EndPopup();
}

void EditorWorkspace::create_popup_new_material()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
    if (!isOpen)
        return;

    ImGui::SeparatorText("Create Material");

    ImGui::Text("Name");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};

    if (ImGui::InputText("##popup_create_material_inputtext", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        std::string path = m_project->path + "/assets/material/" + name;
        /*
        auto baseRender = s_objectSelected->hasComponent<MeshRender>() ? static_cast<BaseRender *>(s_objectSelected->getComponent<MeshRender>()) : static_cast<BaseRender *>(s_objectSelected->getComponent<SkinnedMeshRender>());
        if (baseRender != nullptr)
            baseRender->createMaterial(name);
        */
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void EditorWorkspace::dropScriptInAddComponent()
{
    /*if (s_objectSelected == nullptr || !ImGui::BeginDragDropTarget())
    {
        return;
    }
    
    auto data = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SCRIPT], ImGuiDragDropFlags_None);
    if (!data)
    {
        return ImGui::EndDragDropTarget();
    }

    std::filesystem::path file = std::string((char*)data->Data, data->DataSize);

    auto entity = s_objectSelected->getEntity();
    if (entity != nullptr)
    {
        entity->addComponent(file.filename().replace_extension().string());
    }
    ImGui::EndDragDropTarget();
    */
}

void EditorWorkspace::ShowComponentInfoUI(GameObject& go, const ComponentInfo* info)
{
    char name[512] = {0};
    sprintf(name, "%s 0x%lX", info->name, info->hash);

    ImGui::PushID(info->name);

    bool bVisible = true;

    bool bShowCollapsed = strcmp(info->name, "Transform") == 0 ? ImGui::CollapsingHeader(name) : ImGui::CollapsingHeader(name, &bVisible);
    if (bShowCollapsed)
    {
        auto it = m_guiscriptmap.find(info->name);
        if (it != m_guiscriptmap.end())
            it->second(&go);
        else
            ShowUserComponentUI(go.GetComponentByID(info->index));
    }

    // if (!bVisible)
        // go->DelComponent(info->index);

    ImGui::PopID();
}

void EditorWorkspace::ShowUserComponentUI(void *behavior)
{}

/*
void EditorWorkspace::RenderField(Field &field, Component *component) {
    auto name = field.getName();

    ImGui::Text("%s:", name.c_str());
    name = "##" + name;
    
    switch (field.getType()) {
    case FieldType_GameObject: {
        renderDropGameObject(Field::GetFieldData<GameObject *>(component, field));
        break;
    }
    case FieldType_float: {
        auto data = Field::GetFieldData<float>(component, field);

        ImGui::DragFloat(name.c_str(), data);
        break;
    }
    case FieldType_int: {
        auto data = Field::GetFieldData<int>(component, field);

        ImGui::DragInt(name.c_str(), data);
        break;
    }
    case FieldType_bool: {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::Checkbox(name.c_str(), data);
        break;
    }
    case FieldType_Vector2: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat2(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    case FieldType_Vector3: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat3(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    case FieldType_Vector4: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat4(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    default:
        break;
    }
}
*/

void EditorWorkspace::renderDropGameObject(GameObject **ref)
{
    /*
    if (*ref) {
        ImGui::Text("%s", (*ref)->GetName().c_str());
    }

    if (!ImGui::BeginDragDropTarget()) {
        return;
    }
    
    auto data = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GMDL], ImGuiDragDropFlags_None);
    if (!data) {
        return ImGui::EndDragDropTarget();
    }

    std::cout << __FILE__ << ": " << __LINE__ << std::endl;
    
    // auto path = std::string((char*)data->Data, data->DataSize);

    // AssetsManager::LoadGameObject(path, &EditorWorkspace::OutputGameObject, ref);

    ImGui::EndDragDropTarget();
    */
}

/* ********** Propriedades ********** */
void EditorWorkspace::OnPropertyTransform(GameObject& go, Transform *transform)
{
    static std::vector<const char*> textItemsToVector = {"X:", "Y:", "Z:"};

    bool dirty = false;

    ImGui::Text("Position");
    dirty |= ImGui::DragFloat3("##engine_ui_transform_position", transform->position.data);
    
    ImGui::Text("Scale");
    dirty |= ImGui::DragFloat3("##engine_ui_transform_scale", transform->scale.data);

    auto euler = Math::eulerAngles(transform->rotation);
    ImGui::Text("Rotation");
    if (ImGui::DragFloat3("##engine_ui_transform_rotation", (float*)&euler))
    {
        euler.y = std::clamp(euler.y, -89.0f, 89.0f);

        transform->rotation = Math::normalize(Math::euler(Vector3{Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));
        dirty = true;
    }

    if (dirty)
        m_scene->MarkChildrenDirty(go.entity);
}

void EditorWorkspace::OnPropertyCamera(GameObject& go, Camera *camera)
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::Text("Far plane");
    ImGui::DragFloat("##camera_farplane", &camera->farPlane, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("Near plane");
    ImGui::DragFloat("##camera_nearplane", &camera->nearPlane, 0.1f, 0.0f, 10.0f);

    // bool isChanged = false;

    // ImGui::Text("Rect");
    // auto rect = camera->getRect();

    // ImGui::Text("x");
    // ImGui::SameLine();

    // isChanged |= ImGui::DragFloat("##camera_rect_x", &rect.x);

    // ImGui::Text("y");
    // ImGui::SameLine();

    // isChanged |= ImGui::DragFloat("##camera_rect_y", &rect.y);

    // ImGui::Text("w");
    // ImGui::SameLine();

    // isChanged |= ImGui::DragFloat("##camera_rect_w", &rect.w);

    // ImGui::Text("h");
    // ImGui::SameLine();

    // isChanged |= ImGui::DragFloat("##camera_rect_h", &rect.h);

    // if (isChanged)
        // camera->setRect(rect);

    /*
    ImGui::Text("Background");
    if (ImGui::ColorButton("##camera_background", (*(ImVec4*)&camera->background), ImGuiColorEditFlags_NoLabel, ImVec2(avail.x, 0.0f)))
        ImGui::OpenPopup("##camera_background_popup");

    if (ImGui::BeginPopup("##camera_background_popup"))
    {
        ImGui::ColorPicker4("##camera_background_colorpicker", camera->background.data);
        ImGui::EndPopup();
    }
    */

    m_cameraObjectSelected = ImGui::CollapsingHeader("Render") ? camera : nullptr;

    if (m_cameraObjectSelected != nullptr)
    {
        // camera->setRect(Rect{0.0f, 0.0f, 64.0f, 64.0f});

        auto rendertarget = camera->rendertarget;

        if (rendertarget != nullptr)
            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), {64.0f, 64.0f}, {0.0f, 1.0f}, {1.0f, 0.0f});
    }
}

void EditorWorkspace::OnPropertyMeshFilter(GameObject& go, MeshFilter *meshFilter)
{
    auto mesh = AssetsManager::GetMesh(meshFilter->mesh);
    if (mesh == nullptr)
        return ImGui::Text("No mesh");

    ImGui::Text("Mesh Handle: %ld", meshFilter->mesh);
    ImGui::Text("Vertices: %ld", mesh->numVertices);
    ImGui::Text("Indices: %ld", mesh->numIndices);
    ImGui::Text("Sub: %ld", mesh->numSubmeshes);

    for (uint32_t i=0;i<mesh->numSubmeshes;i++)
    {
        auto& submesh = mesh->submeshes[i];

        ImGui::Text("Submesh %u | Count %lu | Offset %lu", i, submesh.indexCount, submesh.indexOffset);
    }
}

void EditorWorkspace::OnPropertyMeshRender(GameObject& go, MeshRender *meshRender)
{
    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_CreateMaterial;

    ImGui::Indent();
    for (size_t i=0;i<meshRender->numMaterials;i++)
    {
        auto& material = meshRender->materials[i];

        ImGui::Text("Material: %ld", material);
    }
    ImGui::Unindent();
}

void EditorWorkspace::OnPropertyLight(GameObject& go, Light *light)
{
    int elem = static_cast<int>(light->type);
    
    const char* elems_names[3] = {"Directional", "Point", "Spot"};
    
    const char* elem_name = (elem >= 0 && elem < 3) ? elems_names[elem] : "Unknown";

    ImGui::Text("Type");
    if (ImGui::SliderInt("##light_type", &elem, 0, 3 - 1, elem_name))
        light->type = static_cast<LightType>(elem);
    
    if (light->type != LightType::eDirection)
    {
        ImGui::Text("Constant");
        ImGui::DragFloat("##light_constant", &light->attenuationConstant, 0.01f, 0.0f, 10.0f);

        ImGui::Text("Linear");
        ImGui::DragFloat("##light_linear", &light->attenuationLinear, 0.001f, 0.0f, 1.0f);

        ImGui::Text("Quadract");
        ImGui::DragFloat("##light_quadract", &light->attenuationQuadratic, 0.001f, 0.0f, 2.0f);

        if (light->type == LightType::eSpot)
        {
            ImGui::Text("Inner angle:");
            ImGui::DragFloat("##light_inconer", &light->angleInnerCone, 0.1f, 0.0f, 180.0f);

            ImGui::Text("Outer angle:");
            ImGui::DragFloat("##light_outerconner", &light->angleOuterCone, 0.1f, 0.0f, 180.0f);
        }
    }

    ImGui::Text("Color");
    
    if (ImGui::ColorButton("##light_color", *reinterpret_cast<ImVec4 *>(&light->color)))
        ImGui::OpenPopup("##light_color_popup");
    
    if (ImGui::BeginPopup("##light_color_popup"))
    {
        ImGui::ColorPicker4("#light_color_pick", reinterpret_cast<float *>(&light->color));

        ImGui::EndPopup();
    }
}

/*
bool EditorWorkspace::render_material(MaterialID id)
{
    auto name = AssetsManager::GetMaterialName(id);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        m_materialToRename = id;

    auto material = AssetsManager::GetMaterial(id);

    auto shaderPrefab = material->GetShaderPrefab();
    if (shaderPrefab == nullptr)
    {
        dropShaderInMaterial(material);
        return false;
    }

    if (!ImGui::TreeNode(name.c_str()))
        return false;

    bool bCanEnableInstancing = material->supportsInstancing();
    if (ImGui::Checkbox("Instancing", &bCanEnableInstancing))
    {
        if (bCanEnableInstancing)
            material->SetEnableInstancing();
        else
            material->SetDisableInstancing();
    }

    // auto shaderParams = GetShaderParamsFromName(AssetsManager::GetShaderName(shader));

    // auto shadername = gfile::get_file_name(shaderParams.base);

    render_shader_property(shaderPrefab->GetName(), material);

    dropShaderInMaterial(material);

    ImGui::TreePop();

    return false;
}
*/

void EditorWorkspace::render_shader_property(const std::string &name, Material *material)
{
    auto isopen = ImGui::TreeNode(name.c_str());
    if (!isopen)
        return;

    // RenderShaderUniform(material, material->GetShaderPrefab());

    ImGui::TreePop();
}

void EditorWorkspace::RenderShaderUniform(Material *material, gr::Shader *shader)
{
    auto shaderUniforms = shader->GetUniforms();

    for (UniformID id=0;id<shader->GetUniformCount();id++)
    {
        gr::ShaderUniform& uniform = shaderUniforms[id];
        if (strstr(uniform.name, "u_mat") != nullptr)
            continue;

        switch (uniform.type)
        {
        case UniformType::MAT3:
        case UniformType::MAT4:
            continue;
        default:
            break;
        }

        ImGui::SeparatorText(uniform.name);

        switch (uniform.type)
        {
            case UniformType::VEC3:
            {
                auto value = material->Get<Vector3>(uniform.name);
                if (value == nullptr)
                    material->Set<Vector3>(uniform.name, type_traits<Vector3>::zero);

                if (value != nullptr)
                    ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)value);
                break;
            }
            case UniformType::FLOAT:
            {
                auto value = material->Get<float>(uniform.name);
                if (value == nullptr)
                    material->Set<float>(uniform.name, 0.0f);

                if (value != nullptr)
                    ImGui::DragFloat(("##" + std::string(uniform.name)).c_str(), (float *)value);
                break;
            }
            /*case UniformType::int: {
            int value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<int *>(uniform.data);

            if (ImGui::DragInt(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::float: {
            float value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<float *>(uniform.data);

            if (ImGui::DragFloat(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::vec3: {
            Vector3 value = Vector3::zero;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<Vector3 *>(uniform.data);

            if (ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)&value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::vec4: {
            Vector4 value = Vector4::zero;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<Vector4 *>(uniform.data);

            if (ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)&value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        */
        default:
            break;
        }
        
    }
}

void EditorWorkspace::dropShaderInMaterial(Material *material) {
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SHADER]);
    if (!payload)
    {
        return ImGui::EndDragDropTarget();
    }
    
    std::string file((char*)payload->Data, payload->DataSize);
    if (file.empty() || file.find(".glsl") == std::string::npos)
    {
        return ImGui::EndDragDropTarget();
    }

    std::cout << "Wrong: " << file << std::endl;

    // AssetsManager::LoadShader(AssetShader_Shader3D, file, &Material::setShader, material);

    ImGui::EndDragDropTarget();
}

void EditorWorkspace::LoadReources()
{
    std::string internal_assets_path = gfile::get_data_directory() + "/assets";

#define ICON_BUILD(name) \
    do \
    { \
        TaskManager::Add<TextureBuildTask>(internal_assets_path + "/" + name, URI::parse("engine://" + std::string(name))); \
    } while (0) \

    #define ICON_LOAD(name, icon) \
    do \
    { \
        try \
        { \
            SetupTexture(icon, AssetsManager::LoadTexture("engine://" + std::string(name))); \
        } catch (const std::exception& e) \
        { \
            Console::LogError("[ Editor ] [ LoadReources ] error='%s'.", e.what()); \
        } \
    } while (0)

    // #define LOAD_ICON(n, t) ICON_BUILD(n)
    #define LOAD_ICON(n, t) ICON_LOAD(n, t)

        // AssetsManager::LoadTexture(internal_assets_path + path, &EditorWorkspace::SetupTexture, this, icon_type)

    LOAD_ICON("icon/icons8-file-30.png",                    IconType::ICON_FILE);
    LOAD_ICON("icon/icons8-folder-50.png",                  IconType::ICON_FOLDER);
    LOAD_ICON("icon/icons8-go-back-50.png",                 IconType::ICON_BACK_FILE);
    LOAD_ICON("icon/icons8-3d-object-50.png",               IconType::ICON_GAMEOBJECT);
    LOAD_ICON("icon/icons8-lápis-30.png",                   IconType::ICON_PENCIL);
    LOAD_ICON("icon/icons8-lâmpada-globular-64.png",        IconType::ICON_LIGHT);
    LOAD_ICON("icon/icons8-textura-de-renderização-32.png", IconType::ICON_TEXTURE);
    LOAD_ICON("icon/icons8-png-24.png",                     IconType::ICON_PNG);
    LOAD_ICON("icon/icons8-reproduzir-30.png",              IconType::ICON_PLAY_GAME);
    LOAD_ICON("icon/icons8-parar-50.png",                   IconType::ICON_STOP_GAME);
    LOAD_ICON("icon/icons8-resíduos-50.png",                IconType::ICON_BIN);
    LOAD_ICON("icon/icons8-importar-50.png",                IconType::ICON_IMPORT);
    LOAD_ICON("icon/icons8-ordenar-para-a-direita-50.png",  IconType::ICON_ASCENDENTE);
    LOAD_ICON("icon/icons8-ordem-descendente-50.png",       IconType::ICON_DESCENDENTE);
    LOAD_ICON("icon/icons8-editar-24.png",                  IconType::ICON_EDIT);
    LOAD_ICON("icon/motion-graphics.png",                   IconType::ICON_ANIMATION);
    LOAD_ICON("icon/shade.png",                             IconType::ICON_SHADER);
    LOAD_ICON("icon/studio-set.png",                        IconType::ICON_SCENE);

    #undef LOAD_ICON
}

void EditorWorkspace::SetupTexture(IconType type, TextureHandle h)
{
    iconsUI[type] = h;

    if (type == IconType::ICON_PLAY_GAME)
    {
        s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
    }
}

void EditorWorkspace::PostLoadGameObject(Entity prefabID)
{
    if (m_scene == nullptr)
        return;

    m_scene->deselect_all();
    m_scene->select(prefabID);
}

void EditorWorkspace::set_scene(Scene *scene)
{
    // m_scene = scene;

    // scene->SetCamera(m_cameraID);
}

void EditorWorkspace::OutputGameObject(GameObject **ref, GameObject *go)
{
    // if ((*ref) != nullptr)
        // GameObject::Destroy((*ref)->GetCacheID());

    // *ref = go;
}

void EditorWorkspace::generateConfig(const std::string &file)
{
    /*
    auto scenes = SceneManager::GetScenes();
    if (!scenes.empty())
    {
        auto section = config_find_section(m_settings, "Scenes");
        if (section != nullptr)
        {
            config_remove_section(m_settings, section);
        }

        section = config_create_section(m_settings, "Scenes", nullptr);
        
        for (auto &scene : scenes)
        {
            auto name = SceneManager::GetSceneName(scene);

            auto subsection = config_create_sub_section(section, "Scene", nullptr);
            config_set_string(subsection, "name", name);
        }
    }

    {
        auto section = config_find_section(m_settings, "Script");
        if (section != nullptr)
        {
            config_remove_section(m_settings, section);
        }

        section = config_create_section(m_settings, "Script", nullptr);

        if (config_get_string(section, "libname") == nullptr)
        {
            config_set_string(section, "libname", "libgr-component.so");
        }
        
        if (config_get_string(section, "dir") == nullptr)
        {
            config_set_string(section, "dir", "/.build/component/lib/");
        }
    }
    */
}

void EditorWorkspace::updateFileTree(gtreefile *tree_file)
{
    if (!tree_file->bdirectory)
        return;

    auto &children = tree_file->children;

    std::vector<std::filesystem::directory_entry> files;
    for (auto &dir : std::filesystem::directory_iterator(tree_file->path))
    {
        files.push_back(dir);
    }

    std::sort(files.begin(), files.end(), [](auto& lhs, auto& rhs) {
        if (lhs.is_directory() != rhs.is_directory()) {
            return lhs.is_directory() > rhs.is_directory();
        }
        return lhs.path().filename() < rhs.path().filename();
    });

    children.clear();

    children.resize(files.size());
    for (std::size_t i=0;i<files.size();i++)
    {
        auto &child = children[i];

        child.name          = files[i].path().filename().string();
        child.path          = files[i].path().string();
        child.parent        = tree_file;
        child.extension     = files[i].path().extension();
        child.bdirectory    = files[i].is_directory();

        auto extension = files[i].path().extension().string();

        auto icon = IconType::ICON_FILE;
        if (extension == HEADER_MODEL_EXTENSION)
        {
            icon = IconType::ICON_GAMEOBJECT;
        } else if (extension == ".anm")
        {
            icon = IconType::ICON_ANIMATION;
        } else if (extension == ".scn")
        {
            icon = IconType::ICON_SCENE;
        } else if (extension == HEADER_SHADER_EXTENSION)
        {
            icon = IconType::ICON_SHADER;
        } else if (extension == ".hpp" || extension == ".cpp")
        {
            icon = IconType::ICON_SCRIPT;
        } else if (extension == ".png")
        {
            icon = IconType::ICON_TEXTURE;
        }

        child.texture = iconsUI[icon];

        updateFileTree(&child);
    }
}

void EditorWorkspace::select_file(gtreefile *file)
{
    auto &&[it, added] = g_tree_file_selected_map.emplace(file);
    if (added)
        g_tree_file_selected.push_back(file);
}

void EditorWorkspace::unselect_file(gtreefile *file)
{
    g_tree_file_selected.erase(std::remove(g_tree_file_selected.begin(), g_tree_file_selected.end(), file), g_tree_file_selected.end());
    g_tree_file_selected_map.erase(file);
}

bool EditorWorkspace::selected_file(gtreefile *file) const
{
    return g_tree_file_selected_map.find(file) != g_tree_file_selected_map.end();
}

void EditorWorkspace::unselect_all_file()
{
    g_tree_file_selected.clear();
    g_tree_file_selected_map.clear();
}

void EditorWorkspace::delete_file_selected()
{
    for (auto &selected : g_tree_file_selected)
        gfile::remove(selected->path);

    unselect_all_file();

    updateFileTree(&root_tree_file);
}

void EditorWorkspace::RebuildHierarchyVector()
{
    uiList.clear();

    Entity root = m_scene->GetRoot();

    auto rel = m_scene->GetWorldDynamic()->GetComponent<Relationship>(root);

    Entity child = rel->firstChild;
    while (child != INVALID_ENTITY)
    {
        uiList.push_back(child);

        auto cRel = m_scene->GetWorldDynamic()->GetComponent<Relationship>(child);
        child = cRel->nextSibling;
    }
}

PhysicsTask::PhysicsTask()
{}

int PhysicsTask::doInBackground()
{
    gtimer time(60.0f);

    while (Settings::bGameRunning)
    {
        time.begin_frame();

        // PhysicsScene::Update(time.get_delta());

        time.end_frame();
    }

    return 0;
}


