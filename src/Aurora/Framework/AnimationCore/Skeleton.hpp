#pragma once

#include <Aurora/Core/Vector.hpp>
#include <chrono>
#include "StaticVector.hpp"

using Seconds = std::chrono::duration<float>;

template<typename T, typename U>
inline U map(T const from_start, T const from_end, U const to_start, U const to_end, T const from)
{
	return to_start + (to_end - to_start) * (from - from_start) / (from_end - from_start);
}

template<typename T>
inline glm::quat map(T const from_start, T const from_end, glm::quat const to_start, glm::quat const to_end, T const from)
{
	return glm::slerp(to_start, to_end, (from - from_start) / (from_end - from_start));
}

struct PoseBone
{
	std::string name;

	static constexpr auto no_parent = static_cast<std::size_t>(-1);
	std::size_t parent_index{no_parent};

	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
};

struct Pose
{
	std::vector<PoseBone> bones;
};

struct AnimatedBone
{
	std::vector<glm::vec3> scales;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> translations;
};

struct Animation
{
	std::vector<AnimatedBone> bones;
};

template<typename T>
struct Keyframe
{
	Seconds time;
	T value;
};

template<typename T>
struct AnimationTrack
{
	std::vector<Keyframe<T>> keyframes_;

	AnimationTrack() = default;

	explicit AnimationTrack(std::vector<Keyframe<T>> keyframes) : keyframes_{std::move(keyframes)}
	{
	}

	[[nodiscard]] std::vector<T> extract_values() const
	{
		auto values = std::vector<T>{};
		values.reserve(keyframes_.size());

		for (auto const& keyframe: keyframes_)
		{
			values.push_back(keyframe.value);
		}
		return values;
	}

	void set_values(std::vector<T> const& values)
	{
		for (int i = 0; i < keyframes_.size(); ++i)
		{
			keyframes_[i].value = values[i];
		}
	}

	[[nodiscard]] bool is_empty() const
	{
		return keyframes_.empty();
	}

	Seconds duration() const
	{
		return keyframes_.empty() ? Seconds{} : keyframes_.back().time;
	}

	[[nodiscard]] T evaluate(Seconds const time) const
	{
		assert(!keyframes_.empty());

		auto const end_key = std::lower_bound(keyframes_.begin(), keyframes_.end(), time, [](Keyframe<T> const key, Seconds const time)
		{
			return key.time < time;
		});

		if (end_key == keyframes_.begin())
		{
			return keyframes_.front().value;
		}
		if (end_key == keyframes_.end())
		{
			return keyframes_.back().value;
		}

		auto const start_key = end_key - 1;

		return map(start_key->time.count(), end_key->time.count(), start_key->value, end_key->value, time.count());
	}

	[[nodiscard]] T evaluate(Seconds const time, T const default_value) const
	{
		if (is_empty())
		{
			return default_value;
		}
		return evaluate(time);
	}
};

struct Bone
{
	using ID = unsigned int;

	Bone* parent;
	std::string name;

	// The bone's index in the skeleton's bone array.
	ID id;

	// The bone's global bind transform and its inverse.
	glm::mat4 bind_transform;
	glm::mat4 inverse_bind_transform;

	// The bone's current global transform in an animation.
	glm::mat4 global_transform;

	// The transform applied to bones and skin in an animation, relative to the bind pose.
	// It is equal to inverse_bind_transform * global_transform.
	glm::mat4 animation_transform;

	// Transforms to apply between scale/rotation/translation components, to account for pivots etc.
	glm::mat4 pre_scaling;
	glm::mat4 pre_rotation;
	glm::mat4 pre_translation;
	glm::mat4 post_translation;

	// These are the components of the bone's local bind transform.
	glm::vec3 local_bind_scale;
	glm::quat local_bind_rotation;
	glm::vec3 local_bind_translation;

	// Keyframes for the scale/rotation/translation components of the bone's local transform.
	AnimationTrack<glm::vec3> scale_track;
	AnimationTrack<glm::quat> rotation_track;
	AnimationTrack<glm::vec3> translation_track;

	glm::mat4 calculate_local_transform(glm::vec3 const scale, glm::quat const rotation, glm::vec3 const translation) const
	{
		return post_translation * glm::translate(glm::mat4{1.f}, translation) * pre_translation * glm::mat4_cast(rotation) * pre_rotation * glm::scale(glm::mat4{1.f}, scale) * pre_scaling;
	}

	Bone() = default;
};

struct Skeleton
{
	StaticVector<Bone, 256> bones_;

	void calculate_local_bind_components()
	{
		for (auto& bone: bones_)
		{
			bone.inverse_bind_transform = glm::inverse(bone.bind_transform);

			auto const local = bone.parent ? bone.parent->inverse_bind_transform * bone.bind_transform : glm::inverse(bone.post_translation) * bone.bind_transform;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(local, bone.local_bind_scale, bone.local_bind_rotation, bone.local_bind_translation, skew, perspective);
		}
	}

	/*auto extract_animation() const -> animation_retargeting::Animation
	{
		auto animation = animation_retargeting::Animation{};
		animation.bones.reserve(bones_->size());

		for (auto const& bone: *bones_)
		{
			animation.bones.push_back(animation_retargeting::AnimatedBone{bone.scale_track.extract_values(), bone.rotation_track.extract_values(), bone.translation_track.extract_values()});
		}
		return animation;
	}

	auto extract_pose() const -> animation_retargeting::Pose
	{
		auto pose = animation_retargeting::Pose{};
		pose.bones.reserve(bones_->size());

		for (auto const& bone: *bones_)
		{
			pose.bones.push_back(animation_retargeting::PoseBone{bone.name, bone.parent ? bone.parent->id : animation_retargeting::PoseBone::no_parent, bone.local_bind_scale, bone.local_bind_rotation,
			                                                     bone.local_bind_translation,});
		}

		return pose;
	}

	void set_animation_values(animation_retargeting::Animation const& animation)
	{
		for (auto const i: util::indices(animation.bones))
		{
			auto& bone = (*bones_)[i];
			auto const& animation_bone = animation.bones[i];
			bone.scale_track.set_values(animation_bone.scales);
			bone.rotation_track.set_values(animation_bone.rotations);
			bone.translation_track.set_values(animation_bone.translations);
		}
	}

	void set_bind_pose(animation_retargeting::Pose const& pose)
	{
		for (auto const i: util::indices(pose.bones))
		{
			auto& bone = (*bones_)[i];
			bone.local_bind_scale = pose.bones[i].scale;
			bone.local_bind_rotation = pose.bones[i].rotation;
			bone.local_bind_translation = pose.bones[i].translation;

			auto const local = bone.calculate_local_transform(bone.local_bind_scale, bone.local_bind_rotation, bone.local_bind_translation);

			bone.bind_transform = bone.parent ? bone.parent->bind_transform * local : local;
			bone.inverse_bind_transform = glm::inverse(bone.bind_transform);
		}
	}

	Bone const* bone_by_name(char const* const name) const
	{
		auto const pos = bone_iterator_by_name_(name);
		if (pos != bones_->end())
		{
			return &*pos;
		}
		return nullptr;
	}

	Bone* bone_by_name(char const* const name)
	{
		auto pos = bone_iterator_by_name_(name);
		if (pos != bones_->end())
		{
			return &*pos;
		}
		return nullptr;
	}

	Bone::Id bone_id_by_name(char const* const name) const
	{
		return static_cast<Bone::Id>(bone_iterator_by_name_(name) - bones_->begin());
	}

	Bone const* bone_by_id(Bone::Id const id) const
	{
		return &(*bones_)[id];
	}

	Bone* bone_by_id(Bone::Id const id)
	{
		return &(*bones_)[id];
	}

	std::size_t bone_count() const
	{
		return bones_->size();
	}

	auto const& bones() const
	{
		return *bones_;
	}

	auto& bones()
	{
		return *bones_;
	}*/
};