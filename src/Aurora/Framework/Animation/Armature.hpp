#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Bone.hpp"

namespace Aurora::Animation
{
	class Armature
	{
	public:
		std::vector<Bone> Bones;
		std::vector<Bone*> RootBones;
		robin_hood::unordered_map<String, uint32_t> BoneMapping;
		Matrix4 GlobalInverseTransform;
	};
}