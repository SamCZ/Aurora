#pragma once

#include "Base/Buffer.hpp"
#include "Base/InputLayout.hpp"
#include "Base/PrimitiveType.hpp"
#include "Base/Texture.hpp"
#include "MaterialSet.hpp"
#include "Aurora/Physics/AABB.hpp"

namespace Aurora
{
	class Entity;

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
		AABB m_Bounds;
		bool m_BoundsPreTransformed = false;
		bool m_HasBounds = false;

	public:
		XMesh() = default;
		virtual ~XMesh() = default;

		virtual bool BeforeSectionAdd(PrimitiveSection& section, Entity* cameraEntity) { return true; }
	};
}