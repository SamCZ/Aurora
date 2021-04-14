#include "FileWatcher.hpp"

namespace Aurora
{
	FileWatcher::FileWatcher(std::filesystem::path watchPath, bool recursive, std::chrono::duration<int64_t, std::milli> delay)
	: m_WatchPath(std::move(watchPath)), m_Recursive(recursive), m_Delay(delay), m_Thread(), m_IsRunning(true), m_Paths()
	{
		m_Thread = std::thread([this]() -> void {
			for(auto &file : std::filesystem::recursive_directory_iterator(m_WatchPath)) {
				m_Paths[file.path()] = std::filesystem::last_write_time(file);
			}

			while(m_IsRunning) {
				Update();
				std::this_thread::sleep_for(m_Delay);
			}
		});
	}

	FileWatcher::~FileWatcher()
	{
		Stop();
	}

	void FileWatcher::Stop()
	{
		m_IsRunning = false;
		if(m_Thread.joinable()) {
			m_Thread.join();
		}
	}

	void FileWatcher::Update()
	{
		auto it = m_Paths.begin();

		while (it != m_Paths.end()) {
			if (!std::filesystem::exists(it->first)) {
				if(std::filesystem::is_regular_file(std::filesystem::path(it->first))) {
					if(m_AsyncStatusEvent) m_AsyncStatusEvent(it->first, FileStatus::Deleted);
				}
				it = m_Paths.erase(it);
			} else {
				it++;
			}
		}

		// Check if a file was created or modified
		for(auto &file : std::filesystem::recursive_directory_iterator(m_WatchPath)) {
			auto current_file_last_write_time = std::filesystem::last_write_time(file);

			if(!m_Paths.contains(file.path())) {
				m_Paths[file.path()] = current_file_last_write_time;

				if(std::filesystem::is_regular_file(std::filesystem::path(file.path()))) {
					if(m_AsyncStatusEvent) m_AsyncStatusEvent(file.path(), FileStatus::Created);
				}
			} else if(m_Paths[file.path()] != current_file_last_write_time) {
				m_Paths[file.path()] = current_file_last_write_time;

				if(std::filesystem::is_regular_file(std::filesystem::path(file.path()))) {
					if(m_AsyncStatusEvent) m_AsyncStatusEvent(file.path(), FileStatus::Modified);
				}
			}
		}
	}

	std::string FileWatcher::FileStatusToString(const FileWatcher::FileStatus &fileStatus)
	{
		switch (fileStatus) {
			case FileStatus::Created:
				return "created";
			case FileStatus::Modified:
				return "modified";
			case FileStatus::Deleted:
				return "deleted";
		}

		return "undefined";
	}

	void FileWatcher::SetAsyncEventHandler(const FileWatcher::FileWatchEvent &event)
	{
		m_AsyncStatusEvent = event;
	}
}
