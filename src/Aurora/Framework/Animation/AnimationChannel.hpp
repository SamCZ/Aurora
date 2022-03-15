#pragma once

#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Vector.hpp"

namespace Aurora::Animation
{
	class AnimationChannel
	{
	public:
		int Index;
		String Name;

		AnimationChannel() : Index(-1), Name() { }
		AnimationChannel(int index, String name) : Index(index), Name(std::move(name)) { }

		std::vector<std::pair<double, Vector3>> PositionKeys;
		std::vector<std::pair<double, Quaternion>> RotationKeys;
		std::vector<std::pair<double, Vector3>> ScaleKeys;
	};
}