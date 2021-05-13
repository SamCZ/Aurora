#include "SoundEvent.hpp"

#ifdef FMOD_SUPPORTED
#include <fmod.h>
#include <fmod_errors.h>
#include <fmod_studio.h>

#include "SoundCommon.hpp"

namespace Aurora::Sound
{
    SoundEvent::SoundEvent(FMOD_STUDIO_EVENTDESCRIPTION* event) : m_Event(event), m_HasLoadedSamples(false) {}
    SoundEvent::~SoundEvent() = default;

    SoundInstance_ptr SoundEvent::GetOrCreateInstance(const String& name)
    {
        if(m_Instances.find(name) != m_Instances.end()) {
            return m_Instances[name];
        }

        FMOD_STUDIO_EVENTINSTANCE* eventInstance = nullptr;

        if(!ERRCHECK(FMOD_Studio_EventDescription_CreateInstance(m_Event, &eventInstance))) {
            return nullptr;
        }

        SoundInstance_ptr instance = std::make_shared<SoundInstance>(this, eventInstance, false);
        m_Instances[name] = instance;
        return instance;
    }

    SoundInstance_ptr SoundEvent::CreateOneShotInstance()
    {
        if(!m_HasLoadedSamples) {
            m_HasLoadedSamples = true;
            if(!ERRCHECK(FMOD_Studio_EventDescription_LoadSampleData(m_Event))) {
                std::cerr << "Cannot load sample data !" << std::endl;
                return nullptr;
            }
        }

        FMOD_STUDIO_EVENTINSTANCE* eventInstance = nullptr;

        if(!ERRCHECK(FMOD_Studio_EventDescription_CreateInstance(m_Event, &eventInstance))) {
            return nullptr;
        }

        return std::make_shared<SoundInstance>(this, eventInstance, true);
    }

    FMOD_STUDIO_EVENTDESCRIPTION* SoundEvent::GetEvent()
    {
        return m_Event;
    }

    bool SoundEvent::GetParameterDesc(const String& name, FMOD_STUDIO_PARAMETER_DESCRIPTION* paramDesc)
    {
        if(m_ParameterCacheMap.find(name) != m_ParameterCacheMap.end()) {
            memcpy(paramDesc, &m_ParameterCacheMap[name], sizeof(FMOD_STUDIO_PARAMETER_DESCRIPTION));
            return true;
        }

        if(!ERRCHECK(FMOD_Studio_EventDescription_GetParameterDescriptionByName(m_Event, name.c_str(), paramDesc))) {
            return false;
        }

        m_ParameterCacheMap[name] = *paramDesc;

        return true;
    }
}
#endif