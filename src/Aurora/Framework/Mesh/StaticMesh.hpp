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
			{"POSITION", offsetof(StaticMeshVertexBufferEntry, Position) },
			{"TEXCOORD", offsetof(StaticMeshVertexBufferEntry, TexCoord) },
			{"NORMAL", offsetof(StaticMeshVertexBufferEntry, Normal) },
			{"TANGENT", offsetof(StaticMeshVertexBufferEntry, Tangent) },
			{"BINORMAL", offsetof(StaticMeshVertexBufferEntry, BiTangent) },

			/*{"WORLD_PER_INSTANCE", 0 },
			{"WORLD_PER_INSTANCE", 16 },
			{"WORLD_PER_INSTANCE", 32 },
			{"WORLD_PER_INSTANCE", 48 }*/
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
		[[nodiscard]] int GetLayoutElementCount() const override
		{
			return 5;
		}
	};
}