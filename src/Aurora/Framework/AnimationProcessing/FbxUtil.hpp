#pragma once

#include <Aurora/Core/Vector.hpp>

#define FBXSDK_SHARED 1
#include <fbxsdk.h>
#include <vector>
#include <glm/gtx/euler_angles.hpp>

namespace FbxUtil
{
	template<typename T>
	std::size_t vector_byte_size(std::vector<T> const& vector)
	{
		return vector.size() * sizeof(T);
	}

	inline glm::mat4 fbx_to_glm(FbxAMatrix const& m)
	{
		return glm::make_mat4(static_cast<double const*>(m));
	}

	inline glm::vec3 fbx_to_glm(FbxDouble3 const& vector)
	{
		return glm::vec3{vector.mData[0], vector.mData[1], vector.mData[2]};
	}

	inline glm::vec3 fbx_to_glm(FbxDouble4 const& vector)
	{
		return glm::vec3{vector.mData[0], vector.mData[1], vector.mData[2]};
	}

	inline glm::vec4 to_glm_vec(FbxVector4 const& vector) {
		return {
			vector.mData[0],
			vector.mData[1],
			vector.mData[2],
			vector.mData[3]
		};
	}

	inline glm::vec3 to_glm_vec3(FbxVector4 const& vector) {
		return {
			vector.mData[0],
			vector.mData[1],
			vector.mData[2]
		};
	}

	inline glm::vec2 to_glm_vec(FbxVector2 const& vector) {
		return {
			vector.mData[0],
			vector.mData[1],
		};
	}

	inline glm::mat4 euler_angles_to_mat4_xyz(glm::vec3 const euler)
	{
		return glm::eulerAngleXYZ(euler.x, euler.y, euler.z);
	}

	inline std::string trimmed_bone_name(FbxNode const* const bone_node)
	{
		auto const name = bone_node->GetNameOnly();
		return std::string{name.Mid(name.Find(':') + 1)};
	}

	inline bool string_ends_with(std::string const& string, std::string const& end)
	{
		return string.size() >= end.size() && !string.compare(string.size() - end.size(), end.size(), end);
	}

	inline bool is_end_bone(std::string const& bone_name)
	{
		return FbxUtil::string_ends_with(bone_name, "_end") || FbxUtil::string_ends_with(bone_name, "_End");
	}

	template<typename T>
	auto layer_element_at(FbxLayerElementTemplate<T> const* const layer, int const index)
	{
		return to_glm_vec(layer->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect
			? layer->GetDirectArray().GetAt(index)
			: layer->GetDirectArray().GetAt(layer->GetIndexArray().GetAt(index)));
	}

	inline FbxNode const* find_root_bone(FbxNode const* const root_node)
	{
		if (auto const* const attribute = root_node->GetNodeAttribute())
		{
			if (attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			{
				return root_node;
			}
		}

		for (int i = 0; i < root_node->GetChildCount(); ++i)
		{
			if (auto const* const node = find_root_bone(root_node->GetChild(i)))
			{
				return node;
			}
		}

		return nullptr;
	}
}