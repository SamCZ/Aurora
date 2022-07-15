#pragma once

#include "MeshComponent.hpp"

namespace Aurora
{
class AU_API SkeletalMeshComponent : public MeshComponent
{
private:
	SkeletalMesh_ptr m_Mesh = nullptr;
	Matrix4 Bones[MAX_BONES];
public:
	CLASS_OBJ(SkeletalMeshComponent, MeshComponent);

	double AnimationTime = 0;
	int32_t SelectedAnimation = 0;
	bool AnimationLooping = false;
	bool Playing = false;

	[[nodiscard]] Mesh_ptr GetMesh() const override { return m_Mesh; }
	[[nodiscard]] const SkeletalMesh_ptr& GetSkeletalMesh() const { return m_Mesh; }
	[[nodiscard]] bool HasMesh() const override { return m_Mesh != nullptr; }

	void Tick(double delta) override;
	void UploadAnimation(Buffer_ptr& buffer) override;

	void Play(int32_t animationIndex, bool loop)
	{
		SelectedAnimation = animationIndex;
		AnimationLooping = loop;
		AnimationTime = 0;
		Playing = true;
	}

	int32_t GetBoneIndex(const String& name) const
	{
		if (m_Mesh == nullptr)
		{
			return -1;
		}

		return m_Mesh->Armature.BoneMapping[name];
	}

	Matrix4 GetBoneTransform(int32_t boneIndex) const
	{
		return Bones[boneIndex];
	}

	int32_t GetSocketIndex(const String& socket) const override { return GetBoneIndex(socket); }
	Matrix4 GetSocketTransform(int32_t socketId) const override { return GetBoneTransform(socketId); }

	void SetMesh(const Mesh_ptr& mesh) override
	{
		if(!mesh)
			return;

		if(!mesh->HasType(GetSupportedMeshType()))
		{
			return;
		}

		m_Mesh = SkeletalMesh::Cast(mesh);
		m_MaterialSlots = m_Mesh->MaterialSlots;
	}

	[[nodiscard]] TTypeID GetSupportedMeshType() const override { return SkeletalMesh::TypeID(); }
};
}