#pragma once

#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	class Shader;

	AU_STRUCT(ShaderCollection)
	{
		/// Vertex shader to be used with the pipeline.
		std::shared_ptr<Shader> Vertex = nullptr;

		/// Pixel shader to be used with the pipeline.
		std::shared_ptr<Shader> Pixel = nullptr;

		/// Domain shader to be used with the pipeline.
		std::shared_ptr<Shader> Domain = nullptr;

		/// Hull shader to be used with the pipeline.
		std::shared_ptr<Shader> Hull = nullptr;

		/// Geometry shader to be used with the pipeline.
		std::shared_ptr<Shader> Geometry = nullptr;

		/// Amplification shader to be used with the pipeline.
		std::shared_ptr<Shader> Amplification = nullptr;

		/// Mesh shader to be used with the pipeline.
		std::shared_ptr<Shader> Mesh = nullptr;

		ShaderCollection() = default;
	};
}