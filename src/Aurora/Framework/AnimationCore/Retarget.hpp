#pragma once

#include "Skeleton.hpp"

struct RetargetResult
{
	Animation animation;
	Pose bind_pose;
};

inline RetargetResult RetargetAnimation(Animation source_animation, Pose const& source_bind_pose, Pose target_bind_pose)
{
	assert(source_animation.bones.size() == source_bind_pose.bones.size());

	auto result = RetargetResult{};
	for (auto& target_pose_bone : target_bind_pose.bones)
	{
		auto const source_pos = std::find_if(source_bind_pose.bones.begin(), source_bind_pose.bones.end(), [&](auto const& x) { return x.name == target_pose_bone.name; });

		if (source_pos == source_bind_pose.bones.end()) {
			if (target_pose_bone.parent_index != PoseBone::no_parent) {
				auto const parent_rotation = target_bind_pose.bones[target_pose_bone.parent_index].rotation;
				target_pose_bone.translation = glm::rotate(parent_rotation, target_pose_bone.translation);
				target_pose_bone.rotation = parent_rotation;
			}
			result.bind_pose.bones.push_back(PoseBone{
				std::move(target_pose_bone.name),
				target_pose_bone.parent_index,
				target_pose_bone.scale,
				glm::identity<glm::quat>(),
				target_pose_bone.translation
			});
			result.animation.bones.push_back(AnimatedBone{});
		}
		else {
			if (target_pose_bone.parent_index == PoseBone::no_parent) {
				target_pose_bone.rotation = glm::inverse(source_pos->rotation) * target_pose_bone.rotation;
			}
			else {
				auto const parent_rotation = target_bind_pose.bones[target_pose_bone.parent_index].rotation;
				target_pose_bone.translation = glm::rotate(parent_rotation, target_pose_bone.translation);
				target_pose_bone.rotation = glm::inverse(source_pos->rotation) * parent_rotation * target_pose_bone.rotation;
			}

			// auto const translation_offset = target_pose_bone.translation - source_pos->translation;
			auto const translation_rotation_offset = glm::rotation(glm::normalize(source_pos->translation), glm::normalize(target_pose_bone.translation));
			auto const scale_factor = std::sqrt(glm::length2(target_pose_bone.translation) / glm::length2(source_pos->translation));

			auto& animated_bone = source_animation.bones[source_pos - source_bind_pose.bones.begin()];
			for (auto& translation : animated_bone.translations) {
				// translation = translation + translation_offset;
				// translation = (translation - source_pos->translation) * scale_factor + target_pose_bone.translation;
				translation = glm::rotate(translation_rotation_offset, translation * scale_factor);
			}
			result.animation.bones.push_back(std::move(animated_bone));

			result.bind_pose.bones.push_back(PoseBone{
				std::move(target_pose_bone.name),
				target_pose_bone.parent_index,
				target_pose_bone.scale,
				source_pos->rotation,
				target_pose_bone.translation
			});
		}
	}
	return result;
}