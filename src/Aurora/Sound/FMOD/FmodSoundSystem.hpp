#pragma once

#ifdef AU_FMOD_SOUND

// FMOD VERSION: 2.01

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>

struct FMOD_STUDIO_SYSTEM;

namespace Aurora::FMOD
{
    class SoundEvent;
    AU_CLASS(SoundEvent);

    class SoundInstance;
	AU_CLASS(SoundInstance);

	AU_CLASS(SoundSystem)
	{
    private:
        FMOD_STUDIO_SYSTEM* m_FModStudioSystem;
        std::map<String, SoundEvent_ptr> m_SoundEventsMap;
        std::vector<SoundInstance_ptr> m_PlayImmediateSounds;
    public:
        SoundSystem();
        ~SoundSystem();

        bool LoadBank(const Path& path);

        void Update();

        SoundEvent_ptr GetEvent(const String& name);

        void SetListener(const Vector3D& location, const Vector3D& velocity);

        SoundInstance_ptr PlaySoundOneShot(const String& eventName, float volume = 1.0f, float pitch = 1.0f);
    };
}
#endif