#include "SoundSystem.hpp"

#ifdef FMOD_SUPPORTED

#include <fmod.h>
#include <fmod_errors.h>
#include <fmod_studio.h>

#include "SoundCommon.hpp"
#include "SoundEvent.hpp"

#include <Aurora/AuroraEngine.hpp>

namespace Aurora::Sound
{
    SoundSystem::SoundSystem() : m_FModStudioSystem(nullptr)
    {
		AU_LOG_INFO("Initializing FMOD..." )
        ERRCHECK(FMOD_Studio_System_Create(&m_FModStudioSystem, FMOD_VERSION));
        ERRCHECK(FMOD_Studio_System_Initialize(m_FModStudioSystem, 1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL));


        /*LoadBank("Assets/Sounds/Master.bank");
        LoadBank("Assets/Sounds/Master.strings.bank");
        LoadBank("Assets/Sounds/World.bank");*/

        //SoundInstancePtr abimentSounds = GetEvent("event:/Ambience/Forest")->GetOrCreateInstance("Ambiance");
        //abimentSounds->Play();

		AU_LOG_INFO("FMOD complete.")
    }

    SoundSystem::~SoundSystem()
    {
        ERRCHECK(FMOD_Studio_System_Release(m_FModStudioSystem));
    }

    bool SoundSystem::LoadBank(const Path& path)
    {
        auto fileData = AuroraEngine::AssetManager->LoadFile(path);

        if(fileData == nullptr)
			return false;

        FMOD_STUDIO_BANK* bank = nullptr;

        bool status = ERRCHECK(FMOD_Studio_System_LoadBankMemory(
        		m_FModStudioSystem,
        		reinterpret_cast<const char*>(fileData->GetDataPtr()),
        		static_cast<int>(fileData->GetSize()),
        		FMOD_STUDIO_LOAD_MEMORY,
        		FMOD_STUDIO_LOAD_BANK_NORMAL,
        		&bank));
        //return ERRCHECK(FMOD_Studio_System_LoadBankFile(FModStudioSystem, FS::FixDevPath(path).string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

        return status;
    }

    void SoundSystem::Update()
    {
    	if(!m_PlayImmediateSounds.empty()) {
			for(const SoundInstance_ptr& sound : m_PlayImmediateSounds) {
				sound->Play();
			}

			m_PlayImmediateSounds.clear();
    	}

        ERRCHECK(FMOD_Studio_System_Update(m_FModStudioSystem));
    }

    SoundEvent_ptr SoundSystem::GetEvent(const String& name)
    {
        if(m_SoundEventsMap.find(name) != m_SoundEventsMap.end()) {
            return m_SoundEventsMap[name];
        }

        FMOD_STUDIO_EVENTDESCRIPTION* eventDesc = nullptr;

        if(!ERRCHECK(FMOD_Studio_System_GetEvent(m_FModStudioSystem, name.c_str(), &eventDesc))) {
			AU_LOG_ERROR("Cannot find event ",  name,  " in FMOD !")
            return nullptr;
        }

        SoundEvent_ptr event = std::make_shared<SoundEvent>(eventDesc);
        m_SoundEventsMap[name] = event;
        return event;
    }

    void SoundSystem::SetListener(const Vector3D& location, const Vector3D& velocity)
    {
        FMOD_3D_ATTRIBUTES attributes = { { 0 } };
		FMOD_VECTOR attenuationposition {};

        attributes.up.y = 1;
        attributes.forward.z = 1;

        attributes.position.x = static_cast<float>(location.x);
        attributes.position.y = static_cast<float>(location.y);
        attributes.position.z = static_cast<float>(location.z);

        attributes.velocity.x = static_cast<float>(velocity.x);
        attributes.velocity.y = static_cast<float>(velocity.y);
        attributes.velocity.z = static_cast<float>(velocity.z);

        ERRCHECK(FMOD_Studio_System_SetListenerAttributes(m_FModStudioSystem, 0, &attributes, &attenuationposition));
    }

    SoundInstance_ptr SoundSystem::PlaySoundOneShot(const String& eventName, float volume, float pitch)
    {
        SoundEvent_ptr event = GetEvent(eventName);

        if(event == nullptr) {
			AU_LOG_ERROR("Cannot find sound event ", eventName, " in FMOD!")
            return nullptr;
        }

        SoundInstance_ptr sound = event->CreateOneShotInstance();
        sound->SetVolume(volume);
        sound->SetPitch(pitch);

        m_PlayImmediateSounds.push_back(sound);

        return sound;
    }
}
#endif