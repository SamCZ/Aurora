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

	AU_CLASS(StaticMesh) : public Mesh, public MeshBufferHelper<StaticMesh, StaticMeshVertexBufferEntry>
	{
	public:
		inline StaticMesh() : Mesh(), MeshBufferHelper(this)
		{

		}

		const InputLayout_ptr& GetInputLayout() const override;
	};
}