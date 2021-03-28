#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include <map>
#include <iostream>

#define BASE_OF(TypeName, BaseClass) typename std::enable_if<std::is_base_of<BaseClass, TypeName>::value>::type* = nullptr

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

using String = std::basic_string<char>;
using Path = std::filesystem::path;

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

#ifndef NDEBUG
#   ifndef AU_DEBUG_COUT
#       define AU_DEBUG_COUT(content) std::cout << content << std::endl
#   endif

#   ifndef AU_DEBUG_CERR
#       define AU_DEBUG_CERR(content) std::cerr << content << std::endl
#   endif
#else
#   ifndef AU_DEBUG_COUT
#       define AU_DEBUG_COUT(content)
#   endif

#   ifndef AU_DEBUG_CERR
#       define AU_DEBUG_CERR(content)
#   endif
#endif

#define AU_THROW_ERROR(content) std::cerr << content << std::endl; exit(-1)

inline std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> list;
	std::stringstream buffer;

	for (char c : str) {
		if(c == delimiter) {
			if(buffer.tellp() != 0) {
				list.push_back(buffer.str());
				buffer.str("");
			}
		} else {
			buffer << c;
		}
	}

	if(buffer.tellp() != 0) {
		list.push_back(buffer.str());
	}

	return list;
}