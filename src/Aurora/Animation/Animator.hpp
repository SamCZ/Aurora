#pragma once

namespace Aurora::Animation
{
	typedef void* ArmatureType_t;

	class Animator
	{
	private:
		ArmatureType_t m_Armature;
	public:
		Animator()
		{

		}

	public:
		inline void SetArmature(ArmatureType_t armature) { m_Armature = armature }
	};
}