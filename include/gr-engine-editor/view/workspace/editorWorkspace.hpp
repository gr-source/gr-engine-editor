#pragma once

#include <climits>
#include <cstdint>
#include <filesystem>

#include <functional>
#include <gr-base/core/gameObject.hpp>
#include <gr-base/core/console.hpp>

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/rendertarget.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include <gr-base/asset/assetPtr.hpp>

#include "Global.h"

#include "guizmo/ImGuizmo.h"

#include "manager/viewManager.hpp"
#include "plugin/default.hpp"

#include <gr-render/default.hpp>

#include <gr-base/script/default.hpp>

#include "core/default.hpp"

#include "view/workspace/iworkspace.hpp"

#include "view/editor/editorContext.hpp"

#include <gr-base/ecs/default.hpp>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <gr-ini/config.h>

#ifdef __cplusplus
}
#endif // __cplusplus

using EditorPopupFlags_ = std::uint32_t;

enum EditorPopupFlags : EditorPopupFlags_
{
    EditorPopupFlags_AddComponent           = 7,
    EditorPopupFlags_RenameGameObject       = 16,
    EditorPopupFlags_SceneAddOption         = 17,
};

#define GET_FLAG_NAME(c) #c

enum class EngineState
{
    Edit,
    Play,
};

typedef struct gtreefile
{
    std::string path;

    gtreefile *parent;

    std::string name;

    std::string extension;

    AssetPtr<gr::texture> texture;

    bool bdirectory;

    std::vector<gtreefile> children;
} gtreefile;

class EditorWorkspace : public IWorkspace
{
    struct HotReloadPass
    {
        std::unordered_map<Scene*, std::vector<std::tuple<Signature, std::vector<Entity>>>> recreatePerScene;
    };
public:
    EditorWorkspace();
    ~EditorWorkspace() override;

    const char *GetWorkspaceID() const override
    {
        return "Editor";
    }

    void Update() override;

    // void onRender3D() override;

    void DrawWorkspace() override;

    // void onFocus() override;

    // void onBlur() override;

    void dropPath(const std::vector<std::string>& entries) override;

    void loadScript(const std::string& dir);

    HotReloadPass CheckComponentInfo(std::vector<ComponentInfo>& components);

    void ReloadComponentPass(const HotReloadPass& pass);

    void LoadReources();
    
    void SetupTexture(IconType type, TextureHandle h);

    inline ViewManager& GetViewManager()
    {
        return viewManager;
    }

private:
    std::unordered_map<std::string, std::function<void(GameObject *)>> m_guiscriptmap;

    EngineState m_state;

    Project* m_project;

    ImGuiViewport *m_viewport;

    std::vector<std::string> allComponentName;

    uint32_t popupFlags;

    ImGuiIO *mImGuiIO;

    bool bBuildDock;

    std::vector<Entity> uiEntityList;

    std::vector<Entity> entitiesMarker;

    ViewManager viewManager;

    EditorContext context;

    void initialize();
    void shutdown();

    void create_dock();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void create_window_hierarchy();

    void create_render_hierarchy(Scene* scene);

    void drop_gameobject(std::function<void (Entity)> func);

    void RenderSceneObject(Scene *scene);

    void ShowGameObjectHierarchy(GameObject& go);

    bool dragDropGameObject(Entity entity, const std::string &name);

    void PopupMenuAdd();

    void drop_gameobject_in_scene();

    void create_window_property();

    void create_render_property();

    void create_call_popup();

    void create_popup();

    void create_popup_add_component();

    void create_popup_rename_object();

    void dropScriptInAddComponent();

    void ShowComponentInfoUI(GameObject& go, const ComponentInfo* info);

    void ShowUserComponentUI(void *component);

    void renderDropGameObject(GameObject **ref);

    void OnPropertyTransformLocal(GameObject& go, TransformLocal *transform);

    void OnPropertyCamera(GameObject& go, Camera *camera);
    
    void OnPropertyMeshFilter(GameObject& go, MeshFilter *meshFilter);

    void OnPropertyMeshRender(GameObject& go, MeshRender *meshRender);

    void OnPropertyLight(GameObject& go, Light *light);

    void render_shader_property(const std::string &name, Material *material);

    void RenderShaderUniform(Material *material, gr::shader *shader);

    void dropShaderInMaterial(Material *material);

    void InsertTexture(Material *material);

    template <typename T, typename... Args>
    static void OutputGameObject(GameObject **ref, GameObject *go);

    void loadComponentName();
};


