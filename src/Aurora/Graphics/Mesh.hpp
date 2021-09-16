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

			// For debugging purposes
			Texture_ptr BaseColor = nullptr;
			Texture_ptr NormalMap = nullptr;
		};
	public:
		std::vector<Buffer_ptr>			m_Buffers;
		std::vector<PrimitiveSection>	m_Sections;


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