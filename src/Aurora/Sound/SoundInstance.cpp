#include "SoundInstance.hpp"

#ifdef FMOD_SUPPORTED

#include <fmod.h>
#include <fmod_errors.h>
#include <fmod_studio.h>

#include "SoundCommon.hpp"
#include "SoundEvent.hpp"

namespace Aurora::Sound
{
    SoundInstance::SoundInstance(SoundEvent* event, FMOD_STUDIO_EVENTINSTANCE* instance, bool oneShot)
        : m_Event(event), m_Instance(instance), m_IsOneShot(oneShot)
    {

    }

    SoundInstance::~SoundInstance()
    {
        if(m_IsOneShot) {
            ERRCHECK(FMOD_Studio_EventInstance_Release(m_Instance));
        }
    }

    void SoundInstance::Play()
    {
        ERRCHECK(FMOD_Studio_EventInstance_Start(m_Instance));
    }

    void SoundInstance::Stop(const StopMode& mode)
    {
        ERRCHECK(FMOD_Studio_EventInstance_Stop(m_Instance, (FMOD_STUDIO_STOP_MODE)mode));
    }

    void SoundInstance::SetPaused(bool flag)
    {
        ERRCHECK(FMOD_Studio_EventInstance_SetPaused(m_Instance, flag));
    }

    PlayBackState SoundInstance::GetPlaybackState()
    {
        FMOD_STUDIO_PLAYBACK_STATE state;
        ERRCHECK(FMOD_Studio_EventInstance_GetPlaybackState(m_Instance, &state));
        return (PlayBackState)state;
    }

    bool SoundInstance::IsPlaying()
    {
        return GetPlaybackState() == PlayBackState::Playing;
    }

    bool SoundInstance::IsPaused()
    {
        return GetPlaybackState() == PlayBackState::Playing;
    }

    void SoundInstance::SetVolume(float volume)
    {
        ERRCHECK(FMOD_Studio_EventInstance_SetVolume(m_Instance, volume));
    }

    float SoundInstance::GetVolume(bool real)
    {
        float volume;
        float finalvolume;
        ERRCHECK(FMOD_Studio_EventInstance_GetVolume(m_Instance, &volume, &finalvolume));

        return real ? volume : finalvolume;
    }

    void SoundInstance::SetPitch(float pitch)
    {
        ERRCHECK(FMOD_Studio_EventInstance_SetPitch(m_Instance, pitch));
    }

    float SoundInstance::GetPitch(bool real)
    {
        float pitch;
        float finalPitch;
        ERRCHECK(FMOD_Studio_EventInstance_GetPitch(m_Instance, &pitch, &finalPitch));

        return real ? pitch : finalPitch;
    }

    bool SoundInstance::SetParameter(const String& name, float value)
    {
        FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;

        if(!m_Event->GetParameterDesc(name, &paramDesc)) {
            std::cerr << "Cannot find parameter " << name << " in fmod event !" << std::endl;
            return false;
        }

        value = std::max<float>(value, paramDesc.minimum);
        value = std::min<float>(value, paramDesc.maximum);

        return ERRCHECK(FMOD_Studio_EventInstance_SetParameterByID(m_Instance, paramDesc.id, value, false));
    }

    bool SoundInstance::GetParameter(const String& name, float* value, float* finalvalue)
    {
        FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;

        if(!m_Event->GetParameterDesc(name, &paramDesc)) {
            std::cerr << "Cannot find parameter " << name << " in fmod event !" << std::endl;
            return false;
        }

        return ERRCHECK(FMOD_Studio_EventInstance_GetParameterByID(m_Instance, paramDesc.id, value, finalvalue));
    }

    void SoundInstance::SetLocation(const Vector3D& location, const Vector3D& velocity)
    {
        FMOD_3D_ATTRIBUTES attributes = { { 0 } };

        attributes.up.y = 1;
        attributes.forward.z = 1;

        attributes.position.x = location.x;
        attributes.position.y = location.y;
        attributes.position.z = location.z;

        attributes.velocity.x = velocity.x;
        attributes.velocity.y = velocity.y;
        attributes.velocity.z = velocity.z;

        ERRCHECK(FMOD_Studio_EventInstance_Set3DAttributes(m_Instance, &attributes));
    }
}
#endif