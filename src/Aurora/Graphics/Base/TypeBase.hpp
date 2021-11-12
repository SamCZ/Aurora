#pragma once

#include "Aurora/Core/UUID.hpp"

namespace Aurora
{
	template<typename T>
	class TypeBase
	{
	private:
		UniqueIdHelper<T> m_UniqueId;
	public:
		[[nodiscard]] UniqueIdentifier GetUniqueID() const
		{
			// This unique ID is used to unambiguously identify the object for
			// tracking purposes.
			// Niether GL handle nor pointer could be safely used for this purpose
			// as both GL reuses released handles and the OS reuses released pointers
			return m_UniqueId.GetID();
		}

		static T* Null()
		{
			return nullptr;
		}
	};
}