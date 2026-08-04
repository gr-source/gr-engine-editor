#pragma once

#include "view/windowView.hpp"

class SceneViewport : public WindowView
{
public:
    void Draw() override;

private:
    void show();

    void render_scene(Scene *scene);

    void UpdateResizeSceneFramebuffer();
};


