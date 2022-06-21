#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Vector.hpp"

namespace Aurora::Animation
{
	class Bone
	{
	public:
		int32_t Index;
		String Name;
		int32_t Parent;
		Matrix4 OffsetMatrix{};
		std::vector<Bone*> Children;
	public:
		Bone() : Index(0), Parent(-1), Name(), OffsetMatrix()
		{}

		inline Bone(int32_t index, int32_t parent, String name, const Matrix4 &offset) : Index(index), Parent(parent), Name(std::move(name)), OffsetMatrix(offset)
		{}
	};
}