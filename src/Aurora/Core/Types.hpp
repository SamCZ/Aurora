#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include "assert.hpp"
#include "Library.hpp"
#include "Enum.hpp"

typedef uint32_t uint;
typedef uint8_t uint8;
typedef int8_t int8;
using Path = std::filesystem::path;
using DataBlob = std::vector<uint8_t>;

template <typename T, typename ... Args>
constexpr std::shared_ptr<T> MakeShared(Args&& ...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#define BASE_OF(TypeName, BaseClass) typename std::enable_if<std::is_base_of<BaseClass, TypeName>::value>::type* = nullptr

#define BITF(bit) 1 << bit

#define AU_CLASS(A) \
class AU_API A;\
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

#define AU_CLASS_BODY(classname)                                       \
public:                                                                \
template<typename... Args>                                             \
static std::shared_ptr<classname> New(Args&& ...args)                  \
{                                                                      \
	return std::make_shared<classname>(std::forward<Args>(args)...);   \
}
