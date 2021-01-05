#pragma once

#include <Aurora/Core/SmartPointer.hpp>

#include <Shader.h>
#include <RefCntAutoPtr.hpp>

using namespace Diligent;

namespace Aurora::Render
{
    struct FShaderCollection
    {
        /// Vertex shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Vertex;

        /// Pixel shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Pixel;

        /// Domain shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Domain;

        /// Hull shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Hull;

        /// Geometry shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Geometry;

        /// Amplification shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Amplification;

        /// Mesh shader to be used with the pipeline.
        RefCntAutoPtr<IShader> Mesh;

        FShaderCollection() = default;
    };
    DEFINE_PTR(FShaderCollection)
}