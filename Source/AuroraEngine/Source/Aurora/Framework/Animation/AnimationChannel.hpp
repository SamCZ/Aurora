#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>

namespace Aurora::Framework
{
    class FAnimationChannel
    {
    public:
        int Index;
        String Name;

        FAnimationChannel() : Index(-1), Name()
        {

        }

        FAnimationChannel(int index, String name) : Index(index), Name(std::move(name))
        {

        }

        List<Pair<double, Vector3>> PositionKeys;
        List<Pair<double, Quaternion>> RotationKeys;
        List<Pair<double, Vector3>> ScaleKeys;
    };
}