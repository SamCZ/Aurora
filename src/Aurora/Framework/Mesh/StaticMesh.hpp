#pragma once

#include "Mesh.hpp"

namespace Aurora
{
	struct StaticMeshVertexBufferEntry
	{
		Vector3 Position{};
		Vector2 TexCoord{};
		Vector3 Normal{};
		Vector3 Tangent{};
		Vector3 BiTangent{};

		StaticMeshVertexBufferEntry() = default;

		StaticMeshVertexBufferEntry(const Vector3& position, const Vector2& texCoord) : Position(position), TexCoord(texCoord), Normal(0, 0, 0), Tangent(0, 0, 0), BiTangent(0, 0, 0)
		{

		}

		StaticMeshVertexBufferEntry(const Vector3& position, const Vector2& texCoord, const Vector3& normal) : Position(position), TexCoord(texCoord), Normal(normal), Tangent(0, 0, 0), BiTangent(0, 0, 0)
		{

		}
	};

	static LayoutElement StaticMeshLayout[5] {
			LayoutElement{0, 0, 3, VT_FLOAT32, false}, // Position
			LayoutElement{1, 0, 2, VT_FLOAT32, false}, // TexCoord
			LayoutElement{2, 0, 3, VT_FLOAT32, true}, // Normal
			LayoutElement{3, 0, 3, VT_FLOAT32, true}, // Tangent
			LayoutElement{4, 0, 3, VT_FLOAT32, true}, // BiTangent
	};

	AU_CLASS(StaticMesh) : public Mesh, public MeshBufferHelper<StaticMesh, StaticMeshVertexBufferEntry>
	{
	public:
		inline StaticMesh() : Mesh(), MeshBufferHelper(this)
		{

		}

		[[nodiscard]] LayoutElement* GetLayout() const override
		{
			return StaticMeshLayout;
		}
		int GetLayoutElementCount() const override
		{
			return 5;
		}
	};
}