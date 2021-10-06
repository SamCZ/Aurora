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
			struct Range
			{
				size_t IndexCount;
				size_t IndexByteOffset;
				bool Enabled;
			};

			InputLayout_ptr Layout;
			int MaterialIndex;
			int BufferIndex;
			EIndexBufferFormat IndexFormat;
			EPrimitiveType PrimitiveType;

			std::vector<Range> Ranges;

			void AddRange(size_t indexCount, size_t indexByteOffset)
			{
				Ranges.emplace_back(Range{indexCount, indexByteOffset, true});
			}
		};
	public:
		std::vector<Buffer_ptr>			m_Buffers;
		std::vector<PrimitiveSection>	m_Sections;
		std::shared_ptr<MaterialSet> Materials = std::make_shared<MaterialSet>();

	public:
		XMesh()
		{

		}

		~XMesh()
		{

		}
	};
}