#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>

#include "AnimationChannel.hpp"

namespace Aurora::Framework
{
    class FAnimation
    {
    public:
        String Name;
        double Duration;
        double TicksPerSecond;
        List<FAnimationChannel> Channels;
    public:
        FAnimation() = default;
        inline FAnimation(String name, double duration, double tickPerSecond) : Name(name), Duration(duration), TicksPerSecond(tickPerSecond) {}
    };
}