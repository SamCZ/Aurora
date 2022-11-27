#pragma once

#include "Aurora/Core/Math.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Bone.hpp"

namespace Aurora::Animation
{
	struct Armature
	{
		std::vector<Bone> Bones;
		std::vector<Bone*> RootBones;
		robin_hood::unordered_map<String, int32_t> BoneMapping;
		Matrix4 GlobalInverseTransform;
	};
}