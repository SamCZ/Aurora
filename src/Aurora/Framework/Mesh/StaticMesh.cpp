#include "StaticMesh.hpp"
#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{
	/*static std::vector<LayoutElement> StaticMeshLayout = { // NOLINT(cert-err58-cpp)
			{"POSITION", offsetof(StaticMeshVertexBufferEntry, Position) },
			{"TEXCOORD", offsetof(StaticMeshVertexBufferEntry, TexCoord) },
			{"NORMAL", offsetof(StaticMeshVertexBufferEntry, Normal) },
			{"TANGENT", offsetof(StaticMeshVertexBufferEntry, Tangent) },
			{"BINORMAL", offsetof(StaticMeshVertexBufferEntry, BiTangent) },

			{"WORLD_PER_INSTANCE", 0 },
			{"WORLD_PER_INSTANCE", 16 },
			{"WORLD_PER_INSTANCE", 32 },
			{"WORLD_PER_INSTANCE", 48 }
	};*/

	 const InputLayout_ptr& StaticMesh::GetInputLayout() const
	{
		static InputLayout_ptr layout = RD->CreateInputLayout({
			{"in_pos", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMeshVertexBufferEntry, Position), false, 0},
			{"in_tex", GraphicsFormat::RG32_FLOAT, 0, offsetof(StaticMeshVertexBufferEntry, TexCoord), false, 1},

			{"in_normal", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMeshVertexBufferEntry, Normal), false, 2},
			{"in_tangent", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMeshVertexBufferEntry, Tangent), false, 3},
			{"in_biTangent", GraphicsFormat::RGB32_FLOAT, 0, offsetof(StaticMeshVertexBufferEntry, BiTangent), false, 4},
		});

		return layout;
	}
}