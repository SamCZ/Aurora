#pragma once

#ifdef AU_FMOD_SOUND

#include <iostream>
#include <fmod_common.h>
#include <fmod_errors.h>
#include <Aurora/Logger/Logger.hpp>

namespace Aurora::FMOD
{
    static bool ERRCHECK(const FMOD_RESULT& result)
    {
        if (result != FMOD_OK) {
			AU_LOG_ERROR("FMOD Error! ", FMOD_ErrorString(result));
            return false;
        }

        return true;
    }
}
#endif