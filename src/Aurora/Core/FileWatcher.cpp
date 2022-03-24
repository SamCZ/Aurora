#include "FileWatcher.hpp"
#include "Aurora/Logger/Logger.hpp"

namespace Aurora
{
#if defined(_WIN32)
	static void wcharToCharArray(const WCHAR* src, char* dest, int len) {
		for (unsigned int i = 0; i < len / sizeof(WCHAR); ++i) {
			dest[i] = static_cast<char>(src[i]);
		}
		dest[len / sizeof(WCHAR)] = '\0';
	}

	static const DWORD READ_DIR_CHANGE_FILTER = FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_FILE_NAME;

	class FileHandler
	{
	public:
		static void CALLBACK Notify(DWORD status, DWORD tferred, LPOVERLAPPED over)
		{
			auto* task = (FileWatcher*)over->hEvent;
			if (status == ERROR_OPERATION_ABORTED)
			{
				return;
			}
			if (tferred == 0) return;

			auto* info = (FILE_NOTIFY_INFORMATION*)task->m_info;

			while (info) {
				auto action = info->Action;

				EFileAction actionEnum = EFileAction::Unknown;

				switch (action) {
					case FILE_ACTION_RENAMED_NEW_NAME:
					case FILE_ACTION_RENAMED_OLD_NAME:
						actionEnum = EFileAction::Renamed;
						break;
					case FILE_ACTION_ADDED:
						actionEnum = EFileAction::Added;
						break;
					case FILE_ACTION_MODIFIED:
						actionEnum = EFileAction::Modified;
						break;
					case FILE_ACTION_REMOVED:
						actionEnum = EFileAction::Removed;
						break;
					default: break;
				}

				char tmp[MAX_PATH];
				wcharToCharArray(info->FileName, tmp, info->FileNameLength);

				task->m_Mutex.lock();
				task->m_ThreadEvents.emplace(actionEnum, task->m_WatchingPath / tmp);
				task->m_Mutex.unlock();

				info = info->NextEntryOffset == 0 ? nullptr : (FILE_NOTIFY_INFORMATION*)(((uint8_t*)info) + info->NextEntryOffset);
			}
		}
	};
	#endif

	FileWatcher::FileWatcher(Path path) : m_WatchingPath(std::move(path))
	{
#if defined(_WIN32)
		m_handle = CreateFile(m_WatchingPath.string().c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
		if (m_handle == INVALID_HANDLE_VALUE)
		{
			AU_LOG_ERROR("FIle watcher handle is invalid !");
		}

		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_overlapped.hEvent = this;

		m_Thread = std::thread([this](){
			while (true)
			{
				BOOL status = ReadDirectoryChangesW(m_handle, m_info, sizeof(m_info), TRUE, READ_DIR_CHANGE_FILTER, &m_received, &m_overlapped, &FileHandler::Notify);
				if (status == FALSE) break;
				SleepEx(INFINITE, TRUE);
			}
		});
#endif
	}

	FileWatcher::~FileWatcher()
	{
#if defined(_WIN32)
		CancelIoEx(m_handle, nullptr);
		CloseHandle(m_handle);
		m_Thread.join();
#endif
	}

	void FileWatcher::Update()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		while(!m_ThreadEvents.empty())
		{
			FileEvent e = m_ThreadEvents.front();
			m_ThreadEvents.pop();
			m_EventListeners.Invoke(std::forward<EFileAction>(e.Action), e.FilePath);
		}
	}
}