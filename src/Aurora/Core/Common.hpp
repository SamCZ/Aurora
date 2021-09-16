#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include <map>
#include <iostream>
#include "../Logger/Logger.hpp"

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
}                                                                      \





template<typename T>
class SharedFromThis : public std::enable_shared_from_this<T>
{
public:
	virtual ~SharedFromThis() = default;
	template<typename B>
	std::shared_ptr<B> AsShared();

	template<typename B>
	std::shared_ptr<B> AsSharedSafe();

	inline std::shared_ptr<T> ThisShared()
	{
		return this->shared_from_this();
	}
};

template<typename T>
template<typename B>
std::shared_ptr<B> SharedFromThis<T>::AsShared()
{
	return std::static_pointer_cast<B, T>(this->shared_from_this());
}

template<typename T>
template<typename B>
std::shared_ptr<B> SharedFromThis<T>::AsSharedSafe()
{

	return std::dynamic_pointer_cast<B, T>(this->shared_from_this());
}

template<typename T>
inline bool VectorRemove(std::vector<T>& vector, T& data)
{
	auto iter = std::find(vector.begin(), vector.end(), data);
	if(iter == vector.end()) {
		return false;
	}

	vector.erase(iter);
	return true;
}

template<typename T>
inline bool VectorContains(std::vector<T>& vector, T& data)
{
	return std::find(vector.begin(), vector.end(), data) != vector.end();
}

struct dotted : std::numpunct<char> {
	char do_thousands_sep()   const override { return ' '; }  // separate with dots
	std::string do_grouping() const override { return "\3"; } // groups of 3 digits
	static void imbue(std::ostream &os) {
		os.imbue(std::locale(os.getloc(), new dotted));
	}
};

template<typename T>
inline std::string Stringify(T val)
{
	std::ostringstream oss;
	dotted::imbue(oss);
	oss << val;
	return oss.str();
}