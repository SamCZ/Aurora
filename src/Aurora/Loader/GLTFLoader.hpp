#pragma once

#include <Aurora/Core/Types.hpp>

namespace Aurora
{
	class XMesh;
	class IRenderDevice;

	class GLTFLoader
	{
	public:
		static std::vector<std::shared_ptr<XMesh>> LoadMeshFile(const Path& path, IRenderDevice* renderDevice);
	};
}