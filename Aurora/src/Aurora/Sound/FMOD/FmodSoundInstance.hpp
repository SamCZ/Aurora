#pragma once

#ifdef AU_FMOD_SOUND

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Math.hpp>

struct FMOD_STUDIO_EVENTINSTANCE;

namespace Aurora::FMOD
{
    class SoundEvent;

    enum class PlayBackState
    {
        Playing,
        Sustaining,
        Stopped,
        Starting,
        Stopping,

        ForceInt = 65536
    };

    enum class StopMode
    {
        AllowFadeOut,
        Immediate,

        ForceInt = 65536
    };

	AU_CLASS(SoundInstance)
	{
    private:
        SoundEvent* m_Event;
        FMOD_STUDIO_EVENTINSTANCE* m_Instance;
        bool m_IsOneShot;
    public:
        SoundInstance(SoundEvent* event, FMOD_STUDIO_EVENTINSTANCE* instance, bool oneShot);
        ~SoundInstance();

        void Play();
        void Stop(const StopMode& mode = StopMode::Immediate);
        void SetPaused(bool flag);

        PlayBackState GetPlaybackState();
        bool IsPlaying();
        bool IsPaused();

        void SetVolume(float volume);
        float GetVolume(bool real = true);

        void SetPitch(float pitch);
        float GetPitch(bool real = true);

        bool SetParameter(const String& name, float value);
        bool GetParameter(const String& name, float* value, float* finalvalue);

        void SetLocation(const Vector3D& location, const Vector3D& velocity);
    };
}
#endif