#pragma once

#include "Aurora/Core/Types.hpp"
#include "Base/Texture.hpp"

namespace Aurora
{
	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = 0;
		virtual void Update(double delta) = 0;
		virtual void Render() = 0;
		virtual Texture_ptr GetFinalRT(uint camera) = 0;
		virtual Texture_ptr GetIntermediateRT(const std::string& name) = 0;
	};
}