#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/Delegate.hpp>

namespace Aurora
{
	AU_CLASS(AssetManager);

	AU_CLASS(ResourceObject)
	{
	public:
		typedef EventList<ResourceObject*>::Fnc ResourceChangedEvent;
	protected:
		Path m_Path;
		bool m_FromAssetPackage;
		bool m_IsLoaded;
		EventList<ResourceObject*> m_ResourceChangedEvents;
	protected:
		inline ResourceObject(Path path, bool fromAssetPackage) : m_Path(std::move(path)), m_FromAssetPackage(fromAssetPackage), m_IsLoaded(false) {}
		virtual ~ResourceObject() = default;
	public:
		[[nodiscard]] inline virtual bool Load(bool forceReload)
		{
			return false;
		}

		[[nodiscard]] inline virtual bool Save()
		{
			return false;
		}
	public:
		[[nodiscard]] const Path& GetPath() const noexcept { return m_Path; }
		[[nodiscard]] bool IsFromAssetPackage() const noexcept { return m_FromAssetPackage;}
		[[nodiscard]] bool IsLoaded() const noexcept { return m_IsLoaded; }
		EventList<ResourceObject*>& ResourceUpdateEvents() { return m_ResourceChangedEvents; }
	};
}