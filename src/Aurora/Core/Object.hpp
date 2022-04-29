#pragma once

#include <type_traits>
#include <memory>

#include "Hash.hpp"
#include "Library.hpp"

namespace Aurora
{
	class ObjectBase;

	template <typename Type>
	struct AU_API TypeIDCache
	{
		constexpr static const TTypeID value = Hash_djb2(Type::TypeName());
	};

	class AU_API ObjectBase
	{
	protected:
		virtual ~ObjectBase() = default;
	public:
		static constexpr const char* TypeName() { return "ObjectBase"; }
		static TTypeID TypeID() { return TypeIDCache<ObjectBase>::value; }
		[[nodiscard]] virtual TTypeID GetTypeID() const = 0;
		[[nodiscard]] virtual const char* GetTypeName() const { return TypeName(); }
		[[nodiscard]] virtual bool HasType(TTypeID type) const { return false; }

		template<typename T>
		bool IsA() const
		{
			return HasType(T::TypeID());
		}

		static ObjectBase* SafeCast(ObjectBase* ptr) { return ptr; }
		static const ObjectBase* SafeCast(const ObjectBase* ptr) { return ptr; }
		static ObjectBase* Cast(ObjectBase* ptr) { return ptr; }
		static ObjectBase& Cast(ObjectBase& ptr) { return ptr; }
		static const ObjectBase* Cast(const ObjectBase* ptr) { return ptr; }
		static const ObjectBase& Cast(const ObjectBase& ptr) { return ptr; }
		static std::shared_ptr<ObjectBase> SafeCast(const std::shared_ptr<ObjectBase>& ptr) { return ptr; }
		static std::shared_ptr<ObjectBase> Cast(const std::shared_ptr<ObjectBase>& ptr) { return ptr; }

	};

#define CLASS_OBJ(className, parentClass) \
    typedef parentClass Super;\
	static constexpr const char* TypeName() { return #className; } \
	static TTypeID TypeID() { return TypeIDCache<className>::value; } \
	[[nodiscard]] TTypeID GetTypeID() const override { return TypeID(); } \
	[[nodiscard]] const char* GetTypeName() const override { return TypeName(); } \
	[[nodiscard]] bool HasType(TTypeID type) const override { if (type == TypeID()) return true; else return parentClass::HasType(type); } \
	static className* SafeCast(ObjectBase* ptr) \
	{\
		if(ptr == nullptr) return nullptr;\
		if(ptr->HasType(TypeID())) { return static_cast<className*>(ptr); }\
		else return nullptr;\
	}\
	static const className* SafeCast(const ObjectBase* ptr) \
	{\
		if(ptr == nullptr) return nullptr;\
		if(ptr->HasType(TypeID())) { return static_cast<const className*>(ptr); }\
		else return nullptr;\
	}\
	static className* Cast(ObjectBase* ptr) { if(ptr == nullptr) return nullptr; return static_cast<className*>(ptr); }\
	static className& Cast(ObjectBase& ptr) { return static_cast<className&>(ptr); }\
	static const className* Cast(const ObjectBase* ptr) { if(ptr == nullptr) return nullptr; return static_cast<const className*>(ptr); }\
	static const className& Cast(const ObjectBase& ptr) { return static_cast<const className&>(ptr); } \
	static std::shared_ptr<className> SafeCast(const std::shared_ptr<ObjectBase>& ptr) \
	{ \
		if(ptr == nullptr) return nullptr; \
		if(ptr->HasType(TypeID())) { return static_pointer_cast<className>(ptr); } \
		else return nullptr; \
	} \
	static std::shared_ptr<className> Cast(const std::shared_ptr<ObjectBase>& ptr) { return std::static_pointer_cast<className>(ptr); }
}