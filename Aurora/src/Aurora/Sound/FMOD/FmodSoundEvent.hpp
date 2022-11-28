#pragma once

#ifdef AU_FMOD_SOUND

#include "FmodSoundInstance.hpp"
#include "Aurora/Core/Common.hpp"

#include <fmod.h>
#include <fmod_errors.h>
#include <fmod_studio.h>

namespace Aurora::FMOD
{
	AU_CLASS(SoundEvent)
	{
    private:
        FMOD_STUDIO_EVENTDESCRIPTION* m_Event;
        std::map<String, SoundInstance_ptr> m_Instances;
        bool m_HasLoadedSamples;
        std::map<String, FMOD_STUDIO_PARAMETER_DESCRIPTION> m_ParameterCacheMap;
    public:
        SoundEvent(FMOD_STUDIO_EVENTDESCRIPTION* event);
        ~SoundEvent();

        SoundInstance_ptr GetOrCreateInstance(const String& name);
        SoundInstance_ptr CreateOneShotInstance();

        FMOD_STUDIO_EVENTDESCRIPTION* GetEvent();

        bool GetParameterDesc(const String& name, FMOD_STUDIO_PARAMETER_DESCRIPTION* paramDesc);
    };
}
#endif