#include "view/workspace/editorWorkspace.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

#include <gr-base/module/moduleloader.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/managers/eventManager.hpp>

#include <gr-base/ecs/world.hpp>

#include "core/sandbox.hpp"
// #include "editorApplication.hpp"

#include "gr-base/core/string.hpp"
#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-base/script/relationship.hpp>
#include <gr-base/script/name.hpp>

#include "gr-base/ecs/systemRegistry.hpp"
#include "Global.h"

#include "gr-base/managers/renderPipelineManager.hpp"
#include "gr-base/render/pipeline/forwardPipeline.hpp"
#include "imgui.h"
#include "manager/projectManager.hpp"
#include "script/cameraEditor.hpp"

#include "system/cameraEditorSystem.hpp"

// #include <gr-base/application.hpp>

#include "manager/viewManager.hpp"

#include "task/filmboxLoadTask.hpp"

#include "view/editor/consoleWindowView.hpp"

#include "view/engineStatusView.hpp"
#include "view/loadingTaskPopup.hpp"
#include "view/sceneInspectorView.hpp"
#include "view/spawnObject.hpp"

#include <gr-base/core/assetDB.hpp>
#include <gr-base/core/window.hpp>

#include <gr-base/file/file.hpp>

#include <gr-base/managers/assetsManager.hpp>
#include <gr-base/managers/sceneManager.hpp>
#include <gr-base/managers/taskmanager.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include <gr-base/core/gameObject.hpp>

#include "core/project.hpp"

#include <gr-base/prefab/shaderprefab.hpp>

#include <gr-base/task/textureBuildTask.hpp>
#include <gr-base/task/shaderbuildtask.hpp>

static constexpr Hash64 kBuiltinSystems[] = {
    engine::type_traits<CameraEditorSystem>::hash,
};

static constexpr Hash64 kBuiltinComponents[] = {
    // engine::type_traits::hash<CameraEditor>,
};


static constexpr const char* kLightTypeName[LightType_Count] = {
    "Directional", "Point", "Spot"
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

#include <gr-base/task/shaderbuildtask.hpp>

EditorWorkspace::EditorWorkspace()
    : m_viewport(ImGui::GetMainViewport()),
    mImGuiIO(&ImGui::GetIO()),
    m_state(EngineState::Edit)
{
    popupFlags = 0;

    context.io = mImGuiIO;

    // Renderer::SetRenderTarget(m_backbuffer.get());

    // EditorApplication* engine = static_cast<EditorApplication*>(IApplication::GetInstance());

    m_project = ProjectManager::GetInstance().GetCurrent();

#define INSERT_SCRIPT(object) \
    { \
        #object, \
        [this](GameObject* go) \
        { \
            OnProperty##object(*go, go->GetComponent<object>()); \
        } \
    }

    m_guiscriptmap = {
        INSERT_SCRIPT(TransformLocal),
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

#define REGISTER_SYSTEM(K, ...) \
    do{ \
        SystemRegistry::Register<K __VA_OPT__(,) __VA_ARGS__>(); \
        GetSystemID<K>::id = \
            TypeId<IdCategory::SYSTEM>::GetID<K>(); \
    } while (0)

    REGISTER_SYSTEM(CameraEditorSystem);

#undef REGISTER_SYSTEM
    // ComponentRegistry::Register<CameraEditor>(false);

    LoadReources();

    initialize();

    AssetDB::Load(Scheme_Project, m_project->path + "/asset.db");

    try
    {
        ModuleLoader::Load("script", m_project->path + "/.build/lib/libgr-script.so");

    } catch (const std::exception &e)
    {
        Console::LogError("Fatal error in script: %s", e.what());
    }

    Window::SetWindowTitle(String::format("GR - %s", m_project->name.c_str()).c_str());

    TaskManager::Add<ShaderBuildTask>(
        "/home/NoirVoid/.local/share/gr-engine/assets/shader/line.glsl",
        URI::parse("engine://shader/line")
    );

    TaskManager::Add<ShaderBuildTask>(
            "/home/NoirVoid/.local/share/gr-engine/assets/shader/simple_model.glsl",
            URI::parse("engine://shader/simple_model")
        );

    TaskManager::Add<ShaderBuildTask>(
            "/home/NoirVoid/.local/share/gr-engine/assets/shader/screen.glsl",
            URI::parse("engine://shader/screen")
        );

    loadComponentName();
}

EditorWorkspace::~EditorWorkspace()
{
    AssetDB::Save(Scheme_Project, m_project->path + "/asset.db");

    for (auto scene : SceneManager::GetScenes())
    {
        for (auto systemID : scene->GetSystemIDs())
            scene->DelSystemByID(systemID);

        SceneManager::Remove(scene);
    }

    ModuleLoader::Unload("script");

    shutdown();
}

void* GetVTable(const TypeInfo* info)
{
    ISystem* system = (ISystem*)malloc(info->stride);

    ((void (*)(ISystem*, const ISystem*))(info->constructor))(system, nullptr);
    void* vtable = *(void**)system;

    ((void (*)(ISystem*))(info->destructor))(system);

    free(system);
    return vtable;
}

void EditorWorkspace::loadScript(const std::string& dir)
{
    const std::string file = dir + "/libgr-script.so";

    if (!gfile::exists(file))
        return;

    //
    // Monta mapa dos sistemas disponíveis no novo módulo
    //
    std::unordered_map<Hash64, const SystemInfo*> newSystems;

    std::vector<ComponentInfo> components;
    std::vector<SystemInfo> systems;

    {
        Sandbox sandbox(file);
        if (!sandbox.Run())
            return;

        components = sandbox.components;
        systems = sandbox.systems;

        sandbox.ClosePipes();
    }

    for (auto hash : kBuiltinSystems)
        newSystems.emplace(hash, nullptr);

    for (const auto& info : systems)
        newSystems.emplace(info.hash, &info);

    //
    // Descobre o que mudou
    //
    std::unordered_map<TypeID, void*> previousVTables;
    std::unordered_set<TypeID> systemsToRecreate;
    std::unordered_set<TypeID> systemsToRemove;

    for (size_t i = 0; i < SystemRegistry::GetCount(); ++i)
    {
        auto& current = SystemRegistry::GetSystems()[i];

        auto it = newSystems.find(current.hash);

        if (it == newSystems.end())
        {
            Console::LogWarning("[ Script ] Removed system: %s", current.name);
            systemsToRemove.insert(current.index);
            continue;
        }

        const SystemInfo* updated = it->second;

        if (updated != nullptr)
        {
            if (current.stride != updated->stride)
            {
                systemsToRecreate.insert(current.index);

                std::cout
                    << "Reinstantiate: " << current.name
                    << " | Stride: "
                    << current.stride
                    << " -> "
                    << updated->stride
                    << std::endl;
            }
            else
            {
                previousVTables.emplace(current.index, GetVTable(&current));
            }
        }

        newSystems.erase(it);
    }

    //
    // Remove sistemas antigos e salva quais precisarão ser recriados
    //
    std::unordered_map<Scene*, std::vector<TypeID>> recreatePerScene;

    for (Scene* scene : SceneManager::GetScenes())
    {
        for (TypeID id : systemsToRemove)
            scene->DelSystemByID(id);

        for (TypeID id : systemsToRecreate)
        {
            if (!scene->GetSystemByID(id))
                continue;

            recreatePerScene[scene].push_back(id);
            scene->DelSystemByID(id);
        }
    }

    HotReloadPass componentPass = CheckComponentInfo(components);

    //
    // Hot reload da DLL
    //
    ModuleLoader::Unload("script");

    if (ModuleLoader::CheckModuleHasInMemory("libgr-script.so"))
    {
        Console::LogError("[ Script ] Fatal error: module still loaded.");
        return;
    }

    ModuleLoader::Load("script", file);

    //
    // Loga novos sistemas
    //
    for (auto&& [hash, info] : newSystems)
    {
        if (info)
            Console::LogInfo("[ Script ] Added system: %s", info->name);
    }

    //
    // Atualiza VTables quando necessário
    //
    for (auto&& [id, oldVTable] : previousVTables)
    {
        auto* info = SystemRegistry::GetSystemByID(id);
        void* newVTable = GetVTable(info);

        if (oldVTable == newVTable)
            continue;

        for (Scene* scene : SceneManager::GetScenes())
        {
            if (ISystem* system = scene->GetSystemByID(id))
                *(void**)system = newVTable;
        }

        Console::LogWarning(
            "[ Script ] VTable relocated for system: %s",
            info->name
        );
    }

    //
    // Recria sistemas removidos por mudança de layout
    //
    for (auto&& [scene, ids] : recreatePerScene)
    {
        for (TypeID id : ids)
        {
            if (!scene->GetSystemByID(id))
                scene->AddSystemByID(id);
        }
    }

    ReloadComponentPass(componentPass);

    loadComponentName();
}

EditorWorkspace::HotReloadPass EditorWorkspace::CheckComponentInfo(std::vector<ComponentInfo>& components)
{
    std::unordered_map<Hash64, const ComponentInfo*> newComponents;

    for (auto hash : kBuiltinComponents)
        newComponents.emplace(hash, nullptr);

    for (auto& info : components)
        newComponents.emplace(info.hash, &info);

    // std::unordered_map<TypeID, void*> previousVTables;
    std::unordered_set<TypeID> componentsToRecreate;
    std::unordered_set<TypeID> componentsToRemove;

    for (size_t i=0;i<ComponentRegistry::GetComponentCount();i++)
    {
        auto current = ComponentRegistry::GetComponents()[i];

        auto it = newComponents.find(current.hash);
        if (it == newComponents.end())
        {
            Console::LogWarning("[ Script ] Removed component: %s", current.name);
            componentsToRemove.insert(current.index);
            continue;
        }

        const ComponentInfo* updated = it->second;

        if (updated != nullptr)
        {
            if (current.stride != updated->stride)
            {
                componentsToRecreate.insert(current.index);

                std::cout
                    << "Reinstantiate: " << current.name
                    << " | Stride: "
                    << current.stride
                    << " -> "
                    << updated->stride
                    << std::endl;
            }
            else
            {
                // previousVTables.emplace(current.index, GetVTable(&current));
            }
        }

        newComponents.erase(it);
    }

    for (auto&& [hash, info] : newComponents)
    {
        if (info)
            Console::LogInfo("[ Script ] Added system: %s", info->name);
    }

    Signature requiredRemoveMask = 0;
    for (TypeID typeID : componentsToRemove)
        requiredRemoveMask |= (1ULL << typeID);

    Signature requiredAddMask = 0;
    for (TypeID typeID : componentsToRecreate)
        requiredAddMask |= (1ULL << typeID);

    requiredRemoveMask |= requiredAddMask;

    std::unordered_map<Scene*, std::vector<std::tuple<Signature, std::vector<Entity>>>> recreatePerScene;

    for (auto& scene : SceneManager::GetScenes())
    {
        std::vector<std::tuple<Signature, std::vector<Entity>>> archetypes;
        std::vector<std::tuple<Signature, std::vector<Entity>>> addArchetypes;

        auto& world = scene->world;

        auto& storage = world.GetStorage();

        for (size_t i=0;i<storage.count;i++)
        {
            auto& arche = storage.archetypes[i];

            Signature common = (arche.mask & requiredRemoveMask);
            if (common)
            {
                std::vector<Entity> entities;
                for (size_t j=0;j<arche.chunkCount;j++)
                {
                    Chunk* chunk = arche.chunks[j];

                    for (size_t k=0;k<chunk->count;k++)
                        entities.push_back(chunk->entities[k]);
                }

                if (!entities.empty())
                {
                    archetypes.emplace_back(common, entities);

                    Signature newMask = common & requiredAddMask;
                    if (newMask)
                        addArchetypes.emplace_back(newMask, entities);
                }
            }
        }

        for (auto&& [mask, entities] : archetypes)
        {
            for (auto& entity : entities)
                scene->world.DelComponentByMask(entity, mask);
        }


        if (!addArchetypes.empty())
            recreatePerScene.emplace(scene, addArchetypes);
    }

    HotReloadPass pass;
    // pass.previousVTables = previousVTables;
    pass.recreatePerScene = recreatePerScene;

    return pass;
}


void EditorWorkspace::ReloadComponentPass(const EditorWorkspace::HotReloadPass& pass)
{
    for (auto&& [scene, archetypes] : pass.recreatePerScene)
    {
        for (auto&& [mask, entities] : archetypes)
        {
            for (auto& entity : entities)
            {
                std::cout << "Add: " << entity << ": " << mask << std::endl;
                scene->world.AddComponentByMask(entity, mask);
            }
        }
    }
}

// ===================================== END =========================================================
void EditorWorkspace::Update()
{
    if (context.scene != nullptr)
        context.scene->Update();
}

void EditorWorkspace::Render3D()
{
    if (context.scene != nullptr)
        context.scene->Render();
}

void EditorWorkspace::DrawWorkspace()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    create_dock();

    viewManager.renderer();

    // create_window_viewport();

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
                filename.find(".jpeg") != std::string::npos ||
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

// #include <gr-base/core/assetProvider.hpp>
#include <gr-base/core/assetResolver.hpp>

class ProjectFileProvider : public IAssetProvider
{
    std::string m_directory;
public:
    ProjectFileProvider(const std::string& dir) : m_directory(dir)
    {}

    std::string resolve(const std::string& path) override
    {
        return m_directory + "/assets/" + path;
    }
};

void EditorWorkspace::Initialize()
{
    AssetResolver::Registry<ProjectFileProvider>(Scheme_Project, m_project->path);

    RenderPipelineManager::Register<ForwardPipeline>(PipelineForward);
}

void EditorWorkspace::Shutdown()
{
}

#include <gr-base/core/renderer.hpp>

#include "view/importAssetViewPopup.hpp"
#include "view/browserFile.hpp"

#include "view/editor/sceneViewport.hpp"

void EditorWorkspace::initialize()
{
    viewManager.add<LoadingTaskPopup>();
    viewManager.add<ImportAssetViewPopup>();

    viewManager.add<SceneViewport>(&context)->open = true;
    viewManager.add<SpawnObject>(&context);
    viewManager.add<EngineStatusView>();
    viewManager.add<SceneInspectorView>();
    viewManager.add<ConsoleWindowView>()->open = true;
    viewManager.add<BrowserFile>(&context)->open = true;
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
            // if (ImGui::MenuItem("Close"))
                // EventManager::NotifyAsync<EditorApplicationEvent_onCloseProject>(nullptr);

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
}

void EditorWorkspace::create_window_hierarchy()
{
    if (ImGui::Begin("hierarchy") && context.scene != nullptr)
        create_render_hierarchy(context.scene);

    ImGui::End();
}

void EditorWorkspace::create_render_hierarchy(Scene* scene)
{
    std::string name = SceneManager::GetSceneName(scene);

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

    if (
            ImGui::IsKeyReleased(ImGuiKey_Delete) &&
            !context.selector.empty())
    {
        for (auto go : context.selector.getSelected())
            scene->destroy(go);

        context.selector.deselect_all();


        GameObject root(scene, scene->GetRoot());

        if (auto rel = root.TryGetComponent<Relationship>())
        {
            Entity child = rel->firstChild;

            if (child != INVALID_ENTITY)
                context.selector.select(child);
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
    if (scene->CanSyncHierarchyUI())
    {
        uiEntityList.clear();
        scene->GetRootEntities(uiEntityList);
    }

    ImGui::BeginChild("SceneObjectsScroll");

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        popupFlags |= EditorPopupFlags_SceneAddOption;

    ImGuiListClipper clipper;
    clipper.Begin((int)uiEntityList.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            GameObject go(scene, uiEntityList[i]);
            ShowGameObjectHierarchy(go);
        }
    }
    clipper.End();

    if (ImGui::IsKeyReleased(ImGuiKey_F2) && ImGui::IsWindowHovered() && !context.selector.empty())
        popupFlags |= EditorPopupFlags_RenameGameObject;

    if (
            ImGui::IsKeyReleased(ImGuiKey_Escape) &&
            ImGui::IsWindowHovered()
        )
        context.selector.deselect_all();

    ImGui::EndChild();
}

void EditorWorkspace::ShowGameObjectHierarchy(GameObject& go)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto name = go.GetName();

    // auto rel = ;

    if (context.selector.has(go.entity))
        flags |= ImGuiTreeNodeFlags_Selected;

    auto rel = go.TryGetComponent<Relationship>();
    if (rel != nullptr && rel->firstChild != INVALID_ENTITY)
        flags &= ~ImGuiTreeNodeFlags_Leaf;

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        if (context.selector.has(go.entity))
        {
            context.selector.deselect(go.entity);
        } else
        {
            context.selector.select(go.entity);
        }
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        context.selector.deselect_all();

        context.selector.select(go.entity);
    }

    dragDropGameObject(go.entity, name);
    
    if (isOpen)
    {
        auto rel = go.TryGetComponent<Relationship>();
        if (rel != nullptr)
        {
            GameObject child(go.scene, rel->firstChild);

            while (child)
            {
                ImGui::PushID(go.entity);
                ShowGameObjectHierarchy(child);
                ImGui::PopID();

                rel = child.TryGetComponent<Relationship>();
                if (rel == nullptr)
                    break;

                child.entity = rel->nextSibling;
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
        context.scene->Unlink(child);

        context.scene->SetParent(entity, child);
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
            context.scene->create("Empty");
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void EditorWorkspace::create_window_property()
{
    if (ImGui::Begin("property", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar) &&
            context.scene != nullptr &&
            !context.selector.empty()
        )
        create_render_property();

    ImGui::End();
}

void EditorWorkspace::create_render_property()
{
    GameObject go(context.scene, context.selector.getSelected().back());
    if (!go.IsAlive())
        return;

    auto objName = go.GetName();

    ImGui::SeparatorText(objName.c_str());

    for (auto typeID : context.scene->world.GetAllComponentType(go.entity))
        ShowComponentInfoUI(go, ComponentRegistry::GetComponentByID(typeID));

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_AddComponent;
}

void EditorWorkspace::create_call_popup()
{
    switch (popupFlags)
    {
    case EditorPopupFlags_AddComponent:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
        break;
    case EditorPopupFlags_SceneAddOption:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
        break;
    case EditorPopupFlags_RenameGameObject:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
        break;
    default:
        break;
    }

    popupFlags = 0;
}

void EditorWorkspace::create_popup()
{
    // create_popup_create_scene();

    create_popup_add_component();

    create_popup_rename_object();

    PopupMenuAdd();
}

static int MyInputCallback(ImGuiInputTextCallbackData* data)
{
    auto* allComponentName = static_cast<std::vector<std::string>*>(data->UserData);

    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
    {
        // std::string filter(componentName);

        std::sort(allComponentName->begin(), allComponentName->end(),
            [&](const std::string& lhs, const std::string& rhs)
            {
                const bool lhsStarts = lhs.rfind(data->Buf, 0) == 0;
                const bool rhsStarts = rhs.rfind(data->Buf, 0) == 0;

                if (lhsStarts != rhsStarts)
                    return lhsStarts;

                return lhs < rhs;
            });
    }

    return 0;
}

void EditorWorkspace::create_popup_add_component()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
    if (!isOpen)
        return;

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    char componentName[256] = {'\0'};
    bool bAddComponent = ImGui::InputText(
            "##popup_add_componnet_inputtext_filter",
            componentName,
            sizeof(componentName),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit,
            &MyInputCallback,
            &allComponentName
        );

    auto regionAvail = ImGui::GetContentRegionAvail();

    for (auto it = allComponentName.begin(); it != allComponentName.end(); ++it)
    {
        bool isFirst = (it == allComponentName.begin());
        if (
                ImGui::Selectable(it->c_str(), isFirst) ||
                bAddComponent
            )
        {

            for (auto& entity : context.selector.getSelected())
            {
                GameObject gameObject(context.scene, entity);

                gameObject.AddComponentByName(it->c_str());
            }

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

    Entity entity = context.selector.getSelected().back();

    GameObject go(context.scene, entity);

    // auto object = GameObject::Get();
    auto entityName = go.GetName();

    char newName[256] = {0};
    strcpy(newName, entityName.c_str());

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    if (ImGui::InputText("##create_popup_rename_object_inputtext_0", newName, sizeof(newName), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        auto& n = go.GetOrAddComponent<Name>();
        if (n.value != nullptr)
            free(n.value);

        n.value = strdup(newName);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
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

    if (!bVisible)
    {
        go.scene->world.DelComponentByID(go.entity, info->index);
    }
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
void EditorWorkspace::OnPropertyTransformLocal(GameObject& go, TransformLocal& transform)
{
    bool dirty = false;

    ImGui::Text("Position");
    dirty |= ImGui::DragFloat3("##engine_ui_transform_position", transform.position.data);
    
    ImGui::Text("Scale");
    dirty |= ImGui::DragFloat3("##engine_ui_transform_scale", transform.scale.data);

    auto euler = Math::eulerAngles(transform.rotation);
    euler.x = Math::degree(euler.x);
    euler.y = Math::degree(euler.y);
    euler.z = Math::degree(euler.z);

    ImGui::Text("Rotation");
    if (ImGui::DragFloat3("##engine_ui_transform_rotation", &euler[0]))
    {
        transform.rotation =
            Math::normalize(Math::euler(Vector3{Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));

        dirty = true;
    }

    if (dirty)
        context.scene->MarkChanged<TransformWorld>(go.entity);
}

void EditorWorkspace::OnPropertyCamera(GameObject& go, Camera& camera)
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::Text("Far plane");
    ImGui::DragFloat("##camera_farplane", &camera.farPlane, 1.0f, 0.0f, 1000.0f);

    ImGui::Text("Near plane");
    ImGui::DragFloat("##camera_nearplane", &camera.nearPlane, 0.1f, 0.0f, 10.0f);

    /*
    m_cameraObjectSelected = ImGui::CollapsingHeader("Render") ? camera : nullptr;

    if (m_cameraObjectSelected != nullptr)
    {
        // camera->setRect(Rect{0.0f, 0.0f, 64.0f, 64.0f});

        auto rendertarget = camera->rendertarget;

        if (rendertarget != nullptr)
            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), {64.0f, 64.0f}, {0.0f, 1.0f}, {1.0f, 0.0f});
    }
    */
}

void EditorWorkspace::OnPropertyMeshFilter(GameObject& go, MeshFilter& meshFilter)
{
    auto mesh = AssetsManager::GetMesh(meshFilter.mesh);
    if (mesh == nullptr)
        return ImGui::Text("No mesh");

    ImGui::Text("Mesh Handle: %d", static_cast<MeshID>(meshFilter.mesh).value);
    ImGui::Text("Vertices: %ld", mesh->numVertices);
    ImGui::Text("Indices: %ld", mesh->numIndices);
    ImGui::Text("Sub: %ld", mesh->numSubmeshes);

    for (uint32_t i=0;i<mesh->numSubmeshes;i++)
    {
        auto& submesh = mesh->submeshes[i];

        ImGui::Text("Submesh %u | Count %lu | Offset %lu", i, submesh.indexCount, submesh.indexOffset);
    }
}

void EditorWorkspace::OnPropertyMeshRender(GameObject& go, MeshRender& meshRender)
{
    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f}))
    {}

    ImGui::Indent();
    for (size_t i=0;i<meshRender.numMaterials;i++)
    {
        auto& material = meshRender.materials[i];

        ImGui::Text("Material: %d", material);
    }
    ImGui::Unindent();
}

void EditorWorkspace::OnPropertyLight(GameObject& go, Light& light)
{
    LightType type =
        light.type;
    
    ImGui::Text("Type");
    if (ImGui::SliderInt("##light_type", (int*)&type, LightType_Direction, LightType_Spot, kLightTypeName[type]))
        light.type = type;
    
    if (light.type != LightType_Direction)
    {
        ImGui::Text("Constant");
        ImGui::DragFloat("##light_constant", &light.attenuationConstant, 0.01f, 0.0f, 10.0f);

        ImGui::Text("Linear");
        ImGui::DragFloat("##light_linear", &light.attenuationLinear, 0.001f, 0.0f, 1.0f);

        ImGui::Text("Quadract");
        ImGui::DragFloat("##light_quadract", &light.attenuationQuadratic, 0.001f, 0.0f, 2.0f);

        if (light.type == LightType_Spot)
        {
            ImGui::Text("Inner angle:");
            ImGui::DragFloat("##light_inconer", &light.angleInnerCone, 0.1f, 0.0f, 180.0f);

            ImGui::Text("Outer angle:");
            ImGui::DragFloat("##light_outerconner", &light.angleOuterCone, 0.1f, 0.0f, 180.0f);
        }
    }

    ImGui::Text("Color");
    
    if (ImGui::ColorButton("##light_color", *reinterpret_cast<ImVec4 *>(&light.color)))
        ImGui::OpenPopup("##light_color_popup");
    
    if (ImGui::BeginPopup("##light_color_popup"))
    {
        ImGui::ColorPicker4("#light_color_pick", reinterpret_cast<float *>(&light.color));

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

void EditorWorkspace::RenderShaderUniform(Material *material, gr::shader *shader)
{
    for (UniformID id=0;id<shader->count;id++)
    {
        auto& uniform = shader->uniforms[id];
        if (strstr(uniform.name, "u_mat") != nullptr)
            continue;

        switch (uniform.type)
        {
        case gr::UniformType::MAT3:
        case gr::UniformType::MAT4:
            continue;
        default:
            break;
        }

        ImGui::SeparatorText(uniform.name);

        switch (uniform.type)
        {
            case gr::UniformType::VEC3:
            {
                auto value = material->Get<Vector3>(uniform.name);
                if (value == nullptr)
                    material->Set<Vector3>(uniform.name, type_traits<Vector3>::zero);

                if (value != nullptr)
                    ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)value);
                break;
            }
            case gr::UniformType::FLOAT:
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

void EditorWorkspace::SetupTexture(IconType type, TextureRef texture)
{
    context.iconsUI[type] = texture;
}

void EditorWorkspace::loadComponentName()
{
    allComponentName.clear();

    auto infos = ComponentRegistry::GetComponents();

    for (size_t id=0;id<ComponentRegistry::GetComponentCount();id++)
    {
        auto& info = infos[id];

        allComponentName.push_back(info.name);
    }

    std::sort(
                allComponentName.begin(),
                allComponentName.end(),
                [](const auto &lhs, const auto &rhs)
                {
                    return lhs < rhs;
                }
            );
}

