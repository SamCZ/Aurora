#pragma once

#include <memory>
#include <type_traits>

namespace Aurora
{
	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	template<typename T, typename... _Types>
	static SharedPtr<T> MakeShared(_Types&&... _Args);

    template<typename T, typename... _Types>
    SharedPtr<T> MakeShared(_Types &&... _Args) {
        return std::make_shared<T>(_Args...);
    }

    template<typename T>
	static SharedPtr<T> MakeShareable(T* pointer);

    template<typename T>
    SharedPtr<T> MakeShareable(T *pointer) {
        return SharedPtr<T> { pointer };
    }

#define StaticCastSharedPtr std::static_pointer_cast
#define DynamicCastSharedPtr std::dynamic_pointer_cast

#define New(classname, ...) MakeShared<classname>(__VA_ARGS__)

#define DEFINE_PTR(className) typedef SharedPtr<className> className ## Ptr; typedef WeakPtr<className> className ## WeakPtr;
#define PREDEFINE_PTR(type, className) type className; DEFINE_PTR(className)

#define T_IS(tName, type) std::is_same<tName, type>::value

	template<typename T>
	class SharedFromThis : public std::enable_shared_from_this<T>
	{
	public:
	    virtual ~SharedFromThis() = default;
		template<typename B>
		SharedPtr<B> AsShared();

        template<typename B>
        SharedPtr<B> AsSharedSafe();
	};

	template<typename T>
	template<typename B>
	SharedPtr<B> SharedFromThis<T>::AsShared()
	{
		return StaticCastSharedPtr<B, T>(this->shared_from_this());
	}

    template<typename T>
    template<typename B>
    SharedPtr<B> SharedFromThis<T>::AsSharedSafe()
    {

        return DynamicCastSharedPtr<B, T>(this->shared_from_this());
    }
}