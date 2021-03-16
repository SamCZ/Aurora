#pragma once

#include <Shader.h>
#include <RefCntAutoPtr.hpp>

#include "Aurora/Core/Common.hpp"

using namespace Diligent;

namespace Aurora
{
	AU_STRUCT(ShaderCollection)
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

		ShaderCollection() = default;
	};
}