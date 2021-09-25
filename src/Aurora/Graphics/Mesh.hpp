#pragma once

#include "Base/Buffer.hpp"
#include "Base/InputLayout.hpp"
#include "Base/PrimitiveType.hpp"
#include "Base/Texture.hpp"
#include "MaterialSet.hpp"

namespace Aurora
{
	class XMesh
	{
	public:
		struct PrimitiveSection
		{
			InputLayout_ptr Layout;
			int MaterialIndex;
			int BufferIndex;
			size_t IndexCount;
			size_t IndexOffset;
			size_t IndexByteOffset;
			EIndexBufferFormat IndexFormat;
			EPrimitiveType PrimitiveType;
		};
	public:
		std::vector<Buffer_ptr>			m_Buffers;
		std::vector<PrimitiveSection>	m_Sections;
		std::shared_ptr<MaterialSet> Materials = std::make_shared<MaterialSet>();

	public:
		XMesh()
		{
			std::cout << "new xmesh" << std::endl;
		}

		~XMesh()
		{
			std::cout << "delete xmesh" << std::endl;
		}
	};
}