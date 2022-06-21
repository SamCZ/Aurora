#include "SkeletalMeshComponent.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"

using namespace Aurora::Animation;

namespace Aurora
{
	Matrix4 Bones[MAX_BONES];

	const AnimationChannel* FindChannel(const FAnimation& animation, const Bone& bone)
	{
		for (auto& channel : animation.Channels)
		{
			if (channel.Index == bone.Index)
			{
				return &channel;
			}
		}

		return nullptr;
	}

	Vector3 Interpolate(const Vector3& start, const Vector3& end, double factor)
	{
		return start + (float)factor * (end - start);
	}

	Quaternion Interpolate(Quaternion a, Quaternion b, double factor)
	{
		float blend = (float)factor;

		a = glm::normalize(a);
		b = glm::normalize(b);

		/*Quaternion result;
		float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		float one_minus_blend = 1.0f - blend;

		if (dot_product < 0.0f)
		{
			result.x = a.x * one_minus_blend + blend * -b.x;
			result.y = a.y * one_minus_blend + blend * -b.y;
			result.z = a.z * one_minus_blend + blend * -b.z;
			result.w = a.w * one_minus_blend + blend * -b.w;
		}
		else
		{
			result.x = a.x * one_minus_blend + blend * b.x;
			result.y = a.y * one_minus_blend + blend * b.y;
			result.z = a.z * one_minus_blend + blend * b.z;
			result.w = a.w * one_minus_blend + blend * b.w;
		}

		return glm::normalize(result);*/

		return glm::slerp(a, b, blend);
	}

	template<typename T>
	T InterpolateChannel(double time, const std::vector<AnimationKey<T>>& channelData)
	{
		if (channelData.empty())
		{
			return T();
		}

		if (channelData.size() == 1)
		{
			return channelData[0].Value;
		}

		int channelIndex = 0;

		for (int i = 1; i < channelData.size(); ++i)
		{
			if (channelData[i].Time > time)
			{
				channelIndex = i - 1;
				break;
			}
		}

		int nextChannelIndex = channelIndex + 1;
		if (nextChannelIndex >= channelData.size())
		{
			std::cerr << "Next channel index(" << nextChannelIndex << ") not found!" << std::endl;
			return channelData[channelIndex].Value;
		}

		double delta_time = channelData[nextChannelIndex].Time - channelData[channelIndex].Time;
		double factor = (time - channelData[channelIndex].Time) / delta_time;

		factor = glm::abs(factor);

		T start = channelData[channelIndex].Value;
		T end = channelData[nextChannelIndex].Value;

		return Interpolate(start, end, factor);
	}

	void TransformBonesSingleAnimation(const FAnimation& animation, const Matrix4& globalInverseTransform, double time, const Bone& bone, glm::mat4 parentTransform, Matrix4* transforms)
	{
		const AnimationChannel* channel = FindChannel(animation, bone);

		Matrix4 bone_transformation(1.0f);

		if (channel)
		{
			Vector3 position = InterpolateChannel(time, channel->PositionKeys);
			Quaternion rotation = InterpolateChannel(time, channel->RotationKeys);
			Vector3 scale = InterpolateChannel(time, channel->ScaleKeys);

			bone_transformation = glm::translate(position) * Matrix4(rotation) * glm::scale(scale);
		}

		Matrix4 global_transformation = parentTransform * bone_transformation;

		transforms[bone.Index] = globalInverseTransform * global_transformation * bone.OffsetMatrix;

		for (Bone* child : bone.Children)
		{
			TransformBonesSingleAnimation(animation, globalInverseTransform, time, *child, global_transformation, transforms);
		}
	}

	void TransformBonesSingleAnimation(const Armature& armature, const FAnimation& animation, double time, Matrix4* transforms)
	{
		for (Bone* bone : armature.RootBones)
		{
			TransformBonesSingleAnimation(animation, armature.GlobalInverseTransform, time, *bone, glm::identity<Matrix4>(), transforms);
		}
	}

	void SkeletalMeshComponent::Tick(double delta)
	{
		if (m_Mesh->Animations.empty())
			return;

		const FAnimation& animation = m_Mesh->Animations[SelectedAnimation];
		TransformBonesSingleAnimation(GetSkeletalMesh()->Armature, animation, AnimationTime, Bones);

		if (Playing)
			AnimationTime += delta * animation.TicksPerSecond;

		if (AnimationLooping)
		{
			AnimationTime = std::fmod(AnimationTime, animation.Duration);
		}
		else
		{
			AnimationTime = glm::clamp<double>(AnimationTime, 0, animation.Duration);

			if (AnimationTime >= animation.Duration)
			{
				Playing = false;
				AnimationTime = 0;
			}
		}
	}

	void SkeletalMeshComponent::UploadAnimation(Buffer_ptr& buffer)
	{
		GEngine->GetRenderDevice()->WriteBuffer(buffer, Bones);
	}
}