#--------------------------------
# Vulkan
#--------------------------------
if(NOT TARGET Vulkan::Vulkan)
    if(WIN32)
        if(NOT DEFINED ENV{VULKAN_SDK})
            message(FATAL_ERROR "Environment variable VULKAN_SDK is not defined")
        endif()
        if(NOT IS_DIRECTORY "$ENV{VULKAN_SDK}")
            message(FATAL_ERROR "Environment variable VULKAN_SDK does not point to valid directory, it points to '$ENV{VULKAN_SDK}'")
        endif()

        if(NOT (".lib" IN_LIST CMAKE_FIND_LIBRARY_SUFFIXES))
            set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES};.lib")
        endif()

        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")

        # 64-bit
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
            find_program(Vulkan_GLSLC_EXECUTABLE
                    NAMES glslc
                    HINTS "$ENV{VULKAN_SDK}/Bin"
                    )
            # 32-bit
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/Lib32/vulkan-1.lib")

            find_program(Vulkan_GLSLC_EXECUTABLE
                    NAMES glslc
                    HINTS "$ENV{VULKAN_SDK}/Bin32"
                    )
        else()
            message(FATAL_ERROR "Unsupported size of `void*` ${CMAKE_SIZEOF_VOID_P}")
        endif()

    elseif(ANDROID)

        if(${ANDROID_PLATFORM} LESS 24)
            message(FATAL_ERROR "Android version ${ANDROID_PLATFORM} does not support Vulkan. Only API version 24 and above (Android 7.0)")
        endif()

    elseif(UNIX)

        if(DEFINED ENV{VULKAN_SDK})
            if(NOT IS_DIRECTORY "$ENV{VULKAN_SDK}")
                message(FATAL_ERROR "Environment variable VULKAN_SDK does not point to valid directory, it points to '$ENV{VULKAN_SDK}'")
            endif()

            find_path(Vulkan_INCLUDE_DIRS
                    NAMES vulkan/vulkan.h
                    HINTS "$ENV{VULKAN_SDK}/include")
            find_library(Vulkan_LIBRARIES
                    NAMES vulkan
                    HINTS "$ENV{VULKAN_SDK}/lib")
            find_program(Vulkan_GLSLC_EXECUTABLE
                    NAMES glslc
                    HINTS "$ENV{VULKAN_SDK}/bin")
        else()
            find_path(Vulkan_INCLUDE_DIRS
                    NAMES vulkan/vulkan.h)
            find_library(Vulkan_LIBRARIES
                    NAMES vulkan)
            find_program(Vulkan_GLSLC_EXECUTABLE
                    NAMES glslc)
        endif()
    endif()

    message(NOTICE "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
    message(NOTICE "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
    message(NOTICE "Vulkan_GLSLC_EXECUTABLE: ${Vulkan_GLSLC_EXECUTABLE}")

    if(NOT TARGET Vulkan::Vulkan)
        add_library(Vulkan::Vulkan UNKNOWN IMPORTED GLOBAL)
        set_target_properties(Vulkan::Vulkan PROPERTIES
                IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    endif()

    if(Vulkan_GLSLC_EXECUTABLE AND NOT TARGET Vulkan::glslc)
        add_executable(Vulkan::glslc IMPORTED GLOBAL)
        set_property(TARGET Vulkan::glslc PROPERTY IMPORTED_LOCATION "${Vulkan_GLSLC_EXECUTABLE}")
    endif()
endif()