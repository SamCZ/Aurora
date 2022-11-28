#pragma once

#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Math.hpp"

namespace Aurora::Animation
{
	template<typename T>
	struct AnimationKey
	{
		double Time;
		T Value;
	};

	struct AnimationChannel
	{
		int Index;
		String Name;

		std::vector<AnimationKey<Vector3>> PositionKeys;
		std::vector<AnimationKey<Quaternion>> RotationKeys;
		std::vector<AnimationKey<Vector3>> ScaleKeys;

		AnimationChannel() : Index(-1), Name() { }
		AnimationChannel(int index, String name) : Index(index), Name(std::move(name)) { }
	};
}