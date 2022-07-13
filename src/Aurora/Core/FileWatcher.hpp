#pragma once

#include "Types.hpp"
#include "Delegate.hpp"

#if defined(_WIN32)
#include <Windows.h>
#elif __APPLE__
#else
#include <sys/inotify.h>
#include <sys/select.h>
// TODO: Complete file watcher for linux https://github.com/nem0/LumixEngine/blob/master/src/editor/linux/file_system_watcher.cpp
#endif

#include <thread>
#include <queue>
#include <mutex>

namespace Aurora
{
	enum class EFileAction : uint8_t
	{
		Unknown = 0,
		Added,
		Renamed,
		Modified,
		Removed
	};

	class FileWatcher
	{
		friend class FileHandler;
	private:
		struct FileEvent
		{
			EFileAction Action;
			Path FilePath;
			Path PrevPath;
		};

		Path m_WatchingPath;
		EventEmitter<EFileAction, const Path&, const Path&> m_EventListeners;
#if defined(_WIN32)
		uint8_t m_info[4096]{};
		HANDLE m_handle = nullptr;
		DWORD m_received = 0;
		OVERLAPPED m_overlapped{};
		std::thread m_Thread;
		mutable std::mutex m_Mutex;
		std::queue<FileEvent> m_ThreadEvents;
#endif
	public:
		explicit FileWatcher(Path path);
		~FileWatcher();

		void Update();

		EventEmitter<EFileAction, const Path&, const Path&>& GetEmitter() { return m_EventListeners; }
	};
}