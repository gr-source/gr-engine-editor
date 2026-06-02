
/*
    Criado: seg 13 nov 2023 11∶47∶36
    Atualizado: 18/05/26 14:18
    By: NoirVoid
*/


#pragma once

#include <map>
#include <array>
#include <mutex>
#include <limits>
#include <string>
#include <memory>
#include <vector>
#include <future>
#include <fstream>
#include <ostream>
#include <istream>
#include <iostream>
#include <algorithm>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include "core/exception/Exception.h"

#include <gr-math/gmath.hpp>

/* pasta: src */
class PostProcessing;
class Editor;
class Config;

/* pasta: src/engine */
class Mesh;
class Light;
class Shader;
class Skybox;
class Terrain;
class Texture;
class Material;
class Animation;
// class GameObject;
class ScreenBuffer;
class ScreenRender;
class BufferManager;
class ObjectManager;

/* pasta: src/engine/component */
class EditorController;
class BoxCollider;
class MeshRender;
class LineRender;
class Transform;
class Rigidbody;
class Collider;
class Animator;
class Camera;

class ImGuiRender;

/* pasta: src/engine/ecs */

/* pasta: src/engine/core/scenes */
class SceneManager;
class Scene;

enum class FileType : int {
	NONE = 0,
	OBJ  = 1,
	MTL  = 2,
	PNG  = 3,
	JPG  = 4,
	GR   = 5,
	MT   = 6,
	GOT  = 7,
	SCN  = 8,
	FBX  = 9
};

enum class DragDrop : int {
	TEXTURE      = 0,
	GAMEOBJECT   = 1,
	SCENES_CHILD = 2,
	MATERIAL     = 3,
	NONE         = -1
};

enum class DirectoryType : int {
	NONE      = 0,
	MODEL     = 1,
	MATERIAL  = 2,
	TEXTURE   = 3,
	SHADER    = 4,
	SCENE     = 5,
	ICON      = 6,
	RESOURCES = 7
};

/* objetm o tipo de objeto pra dragdrop */
const char* getDragDrop(const DragDrop& type);

/* pega o tipo do arquivo pela estensao */
FileType getFileType(const std::string& extension);

/* pega o a externsao do arquivo pelo tipo */
const char* getFileTypeName(const FileType& type);




