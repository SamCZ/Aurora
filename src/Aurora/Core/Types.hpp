#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include "assert.hpp"

typedef uint32_t uint;
typedef uint8_t uint8;
typedef int8_t int8;
using Path = std::filesystem::path;
using DataBlob = std::vector<uint8_t>;


#define BASE_OF(TypeName, BaseClass) typename std::enable_if<std::is_base_of<BaseClass, TypeName>::value>::type* = nullptr

#define BITF(bit) 1 << bit

#define AU_CLASS(A) \
class A;\
\
typedef std::shared_ptr<A> A ## _ptr;\
typedef std::weak_ptr<A> A ## _wptr;\
typedef std::shared_ptr<const A> A ## _conptr;\
typedef std::weak_ptr<const A> A ## _wconptr;\
\
class A

#define AU_STRUCT(A) \
struct A;\
\
typedef std::shared_ptr<A> A ## _ptr;\
typedef std::weak_ptr<A> A ## _wptr;\
typedef std::shared_ptr<const A> A ## _conptr;\
typedef std::weak_ptr<const A> A ## _wconptr;\
\
struct A

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

#define AU_CLASS_BODY(classname)                                       \
public:                                                                \
template<typename... Args>                                             \
static std::shared_ptr<classname> New(Args&& ...args)                  \
{                                                                      \
	return std::make_shared<classname>(std::forward<Args>(args)...);   \
}
