#include "view/shadereditor.hpp"

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

// #include <gr-base/ecs/gameObject.hpp>

#include "imgui.h"

ShaderEditor::ShaderEditor()
{}

ShaderEditor::~ShaderEditor()
{}

void ShaderEditor::onFocus()
{
    // EditorController::bCanMove = false;
}

void ShaderEditor::onBlur()
{
    // EditorController::bCanMove = true;
}

void ShaderEditor::onRenderUI()
{}




