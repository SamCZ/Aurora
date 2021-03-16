#pragma once

#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	class Object : public SharedFromThis<Object>
	{
	public:
		String Name{};
	public:
		~Object() override = default;

		template<class T>
		inline bool IsA() const
		{
			return dynamic_cast<const T*>(this) != nullptr;
		}

		template<typename T>
		inline T* SafeCast()
		{
			return dynamic_cast<T*>(this);
		}

		template<typename T>
		inline const T* SafeConstCast() const
		{
			return dynamic_cast<const T*>(this);
		}

		template<typename T>
		inline T* Cast()
		{
			return static_cast<T*>(this);
		}

		template<typename T>
		inline const T* ConstCast() const
		{
			return static_cast<const T*>(this);
		}
	};
}