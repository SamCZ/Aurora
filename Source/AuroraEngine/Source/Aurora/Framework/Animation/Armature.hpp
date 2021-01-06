#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>
#include "Bone.hpp"

namespace Aurora::Framework
{
    class FArmature
    {
    public:
        List<FBone> Bones;
        List<FBone*> RootBones;
        Map<String, uint32_t> BoneMapping;
        Matrix4 GlobalInverseTransform;
    };
}