#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>

namespace Aurora::Framework
{
    class FBone
    {
    public:
        uint32_t Index;
        String Name;
        int32_t Parent;
        Matrix4 OffsetMatrix{};
        List<FBone*> Children;
    public:
        FBone() = default;
        inline FBone(uint32_t index, int32_t parent, String name, const Matrix4& offset) : Index(index), Parent(parent), Name(std::move(name)), OffsetMatrix(offset) { }
    };
}