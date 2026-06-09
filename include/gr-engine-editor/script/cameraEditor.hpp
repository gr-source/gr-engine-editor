#pragma once

#include "gr-base/ecs/default.hpp"

struct CameraEditor
{};

template <>
struct type_traits<CameraEditor>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(CameraEditor);
};

