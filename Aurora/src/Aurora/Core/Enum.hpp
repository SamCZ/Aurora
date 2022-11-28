#pragma once

#ifndef AU_ENUM
#   define AU_ENUM(enum_name, num_type) enum class enum_name : num_type
#endif

#ifndef AU_ENUM_FLAGS
#   define AU_ENUM_FLAGS(enum_name, num_type) enum class enum_name : num_type;\
    inline enum_name operator|(enum_name a, enum_name b)\
    {\
        return static_cast<enum_name>(static_cast<num_type>(a) | static_cast<num_type>(b));\
    }\
    inline bool operator &(enum_name a, enum_name b)\
    {\
        return (static_cast<num_type>(a) & static_cast<num_type>(b)) != 0;\
    }\
    inline enum_name& operator |=(enum_name& a, enum_name b)\
    {\
        return a = a | b;\
    }\
    AU_ENUM(enum_name, num_type)
#endif

#define CAT(a,b) a ## b
