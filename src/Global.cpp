#include "Global.h"

const char* getDragDrop(const DragDrop& type) {
    switch (type) {
    case DragDrop::TEXTURE:
        return "#Texture";
    case DragDrop::GAMEOBJECT:
        return "#GameObject";
    case DragDrop::SCENES_CHILD:
        return "#ScenesChild";
    case DragDrop::MATERIAL:
        return "#Material";
    default:
        return "None";
    }
}

FileType getFileType(const std::string& extension) {
    if (extension == ".obj") {
        return FileType::OBJ;
    } else if (extension == ".mtl") {
        return FileType::MTL;
    } else if (extension == ".png") {
        return FileType::PNG;
    } else if (extension == ".jpg") {
        return FileType::JPG;
    } else if (extension == ".gr") {
        return FileType::GR;
    } else if (extension == ".mt") {
        return FileType::MT;
    } else if (extension == ".got") {
        return FileType::GOT;
    } else if (extension == ".fbx") {
        return FileType::FBX;
    }
    return FileType::NONE;
}

const char* getFileTypeName(const FileType& type) {
    switch (type) {
    case FileType::OBJ:
        return ".obj";
    case FileType::MTL:
        return ".mtl";
    case FileType::PNG:
        return ".png";
    case FileType::JPG:
        return ".jpg";
    case FileType::GR:
        return ".gr";
    case FileType::MT:
        return ".mt";
    case FileType::GOT:
        return ".got";
    case FileType::SCN:
        return ".scn";
    case FileType::FBX:
        return ".fbx";
    default:
        return "none";
    }
}






