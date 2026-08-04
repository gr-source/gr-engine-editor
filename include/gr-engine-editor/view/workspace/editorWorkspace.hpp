#pragma once

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
#include <string>
#include <vector>

#include "view/browserFile.hpp"

#include "core/default.hpp"

#include "view/workspace/iworkspace.hpp"

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
    EditorPopupFlags_CreateScene            = 18
};

#define GET_FLAG_NAME(c) #c


enum IconType : int {
    ICON_FOLDER         = 0,
    ICON_FILE           = 1,
    ICON_BACK_FILE      = 2,
    ICON_GAMEOBJECT     = 3,
	ICON_PENCIL         = 4,
	ICON_LIGHT          = 5,
	ICON_TEXTURE        = 6,
	ICON_PNG            = 7,
	ICON_PLAY_GAME      = 8,
	ICON_STOP_GAME      = 9,
	ICON_BIN            = 10,
	ICON_IMPORT         = 11,
	ICON_ASCENDENTE     = 12,
	ICON_DESCENDENTE    = 13,
	ICON_EDIT           = 14,
	ICON_ANIMATION      = 15,
	ICON_SHADER         = 16,
	ICON_SCENE          = 17,
	ICON_SCRIPT         = 18
};

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

    std::unordered_map<IconType, AssetPtr<gr::texture>> iconsUI;

    std::shared_ptr<RenderTarget> m_backbuffer;

    // ComponentHandle m_cameraID;

    EngineState m_state;

    Project* m_project;

    Scene *m_scene;

    Camera* m_cameraObjectSelected;

    Plugin *m_plugin;

    ImGuiViewport *m_viewport;

    AssetPtr<gr::texture> s_texturePlayGame;

    uint32_t popupFlags;

    ImGuiIO *mImGuiIO;

    bool bBuildDock;

    ImVec4 FPSColor;

    float fpsTimer;

    int fpsFrameCount;

    int fps;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    std::vector<Entity> uiEntityList;

    std::vector<Entity> entitiesMarker;

    ViewManager viewManager;

    void initialize();
    void shutdown();

    virtual void dropPath(const char* path) {}

    void create_dock();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void create_window_hierarchy();

    void create_render_hierarchy(Scene* scene);

    void drop_gameobject(std::function<void (Entity)> func);

    void RenderSceneObject(Scene *scene);

    void ShowGameObjectHierarchy(GameObject& go);

    bool dragDropGameObject(Entity entity, const std::string &name);

    void PopupMenuAdd();

    void create_window_viewport();

    void create_render_viewport();

    void drop_scene_file_in_viewport();

    void render_scene(Scene *scene);

    void UpdateResizeSceneFramebuffer();

    void create_render_guizmo(Scene *scene, const GameObject* go);

    void create_guizmo_manipulate(const GameObject& gameObject, const Matrix4x4& projection, const Matrix4x4& view);

    void OnPlayButton();

    void OnBuildScript();

    void OnPostSaveSnapshot(std::string file);

    void OnPostLoadSnapshot(Scene* scene);

    void OnStopButton();

    void drop_gameobject_in_scene();

    void create_window_property();

    void create_render_property();

    void create_call_popup();

    void create_popup();

    void create_popup_create_scene();

    void create_popup_add_component();

    void create_popup_rename_object();

    void dropScriptInAddComponent();

    void ShowComponentInfoUI(GameObject& go, const ComponentInfo* info);

    void ShowUserComponentUI(void *component);

    void renderDropGameObject(GameObject **ref);

    void OnPropertyTransform(GameObject& go, TransformLocal *transform);

    void OnPropertyCamera(GameObject& go, Camera *camera);
    
    void OnPropertyMeshFilter(GameObject& go, MeshFilter *meshFilter);

    void OnPropertyMeshRender(GameObject& go, MeshRender *meshRender);

    void OnPropertyLight(GameObject& go, Light *light);

    // bool render_material(MaterialID id);

    void render_shader_property(const std::string &name, Material *material);

    void RenderShaderUniform(Material *material, gr::shader *shader);

    void dropShaderInMaterial(Material *material);

    void InsertTexture(Material *material);

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    static void OutputGameObject(GameObject **ref, GameObject *go);

    void generateConfig(const std::string &file);
};

template <typename T, typename... Args>
inline void EditorWorkspace::RenderProperty(Args&&... args)
{
    if (m_scene == nullptr || !m_scene->has_object_selected())
        return;

    GameObject go(m_scene, m_scene->get_object_selected().back());
    if (!go.GetComponent<T>())
        return;

    std::invoke(std::bind(std::forward<Args>(args)..., go, go.GetComponent<T>()));
}

