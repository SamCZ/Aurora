project(FMod)

FIND_PATH(FMOD_CORE_INCLUDE_DIR "fmod.h"
        PATHS
        ${FMOD_API_DIR}/core/inc
        $ENV{FMOD_HOME}/inc
        $ENV{EXTERNLIBS}/fmod/studio/inc
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        DOC "fmod - Headers"
        )
FIND_PATH(FMOD_STUDIO_INCLUDE_DIR "fmod_studio.h"
        PATHS
        ${FMOD_API_DIR}/studio/inc
        $ENV{FMOD_HOME}/inc
        $ENV{EXTERNLIBS}/fmod/studio/inc
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        DOC "fmod - Headers"
        )

FIND_LIBRARY(FMOD_CORE_LIBRARY NAMES fmod_vc
        PATHS
        ${FMOD_API_DIR}/core/lib/x64
        PATH_SUFFIXES lib lib64
        DOC "fmod - Library"
        )

find_file(FMOD_CORE_LIBRARY_DLL NAMES fmod.dll
        PATHS
        ${FMOD_API_DIR}/core/lib/x64
        )

FIND_LIBRARY(FMOD_STUDIO_LIBRARY NAMES fmodstudio_vc
        PATHS
        ${FMOD_API_DIR}/studio/lib/x64
        PATH_SUFFIXES lib lib64
        DOC "fmod - Library"
        )

find_file(FMOD_STUDIO_LIBRARY_DLL NAMES fmodstudio.dll
        PATHS
        ${FMOD_API_DIR}/studio/lib/x64
        )

if(FMOD_CORE_INCLUDE_DIR)
    set(AURORA_DLL_TO_COPY
            ${AURORA_DLL_TO_COPY}
            ${FMOD_CORE_LIBRARY_DLL}
            ${FMOD_STUDIO_LIBRARY_DLL}
            CACHE INTERNAL "AURORA_DLL_TO_COPY")
    set(FMod_NotFound OFF CACHE INTERNAL "FMod_NotFound")
else()
    set(FMod_NotFound ON CACHE INTERNAL "FMod_NotFound")
    return()
endif()

add_library(FMod SHARED IMPORTED)
set_property(TARGET FMod PROPERTY IMPORTED_LOCATION ${FMOD_CORE_LIBRARY_DLL} ${FMOD_STUDIO_LIBRARY_DLL})
set_property(TARGET FMod PROPERTY IMPORTED_IMPLIB ${FMOD_CORE_LIBRARY} ${FMOD_STUDIO_LIBRARY})

set(AURORA_ADD_INCLUDE_DIRS
        ${FMOD_CORE_INCLUDE_DIR}
        ${FMOD_STUDIO_INCLUDE_DIR}
        CACHE INTERNAL "AURORA_ADD_INCLUDE_DIRS")

#set_target_properties(FMod PROPERTIES LINKER_LANGUAGE C)
#target_include_directories(FMod PUBLIC ${FMOD_CORE_INCLUDE_DIR})
#target_include_directories(FMod PUBLIC ${FMOD_STUDIO_INCLUDE_DIR})

#target_link_libraries(FMod IMPORTED ${FMOD_CORE_LIBRARY} ${FMOD_STUDIO_LIBRARY})

message(${FMOD_CORE_INCLUDE_DIR})
message(${FMOD_CORE_LIBRARY})
message(${FMOD_CORE_LIBRARY_DLL})

message(${FMOD_STUDIO_INCLUDE_DIR})
message(${FMOD_STUDIO_LIBRARY})
message(${FMOD_STUDIO_LIBRARY_DLL})