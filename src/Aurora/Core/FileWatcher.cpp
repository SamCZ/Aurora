#include "FileWatcher.hpp"

#include <iostream>

namespace Aurora
{
	FileWatcher::FileWatcher(std::filesystem::path watchPath, bool recursive, std::chrono::duration<int64_t, std::milli> delay, WatchFlags watchFlags)
	: m_WatchPath(std::move(watchPath)), m_Recursive(recursive), m_Delay(delay), m_WatchFlags(watchFlags), m_Thread(), m_IsRunning(true), m_Paths()
	{
		if(!std::filesystem::exists(m_WatchPath)) {
			std::cerr << "Could not init FileWatcher path " << m_WatchPath << " doesn't exists !" << std::endl;
			return;
		}

		m_Thread = std::thread([this]() -> void {
			if(m_Recursive) {
				for(auto &file : std::filesystem::recursive_directory_iterator(m_WatchPath)) {
					if(IsFileWatchable(file)) m_Paths[file.path()] = std::filesystem::last_write_time(file);
				}
			} else {
				for(auto &file : std::filesystem::directory_iterator(m_WatchPath)) {
					if(IsFileWatchable(file)) m_Paths[file.path()] = std::filesystem::last_write_time(file);
				}
			}

			while(m_IsRunning) {
				UpdateThread();
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

	void FileWatcher::UpdateThread()
	{
		auto it = m_Paths.begin();

		while (it != m_Paths.end()) {
			if (!std::filesystem::exists(it->first)) {
				CallEvents(it->first, FileStatus::Deleted);
				it = m_Paths.erase(it);
			} else {
				it++;
			}
		}

		// Check if a file was created or modified
		if(m_Recursive) {
			for(auto &file : std::filesystem::recursive_directory_iterator(m_WatchPath)) {
				UpdateFileStatus(file);
			}
		} else {
			for(auto &file : std::filesystem::directory_iterator(m_WatchPath)) {
				UpdateFileStatus(file);
			}
		}
	}

	void FileWatcher::UpdateFileStatus(const std::filesystem::directory_entry &entry)
	{
		auto current_file_last_write_time = std::filesystem::last_write_time(entry);

		if(!m_Paths.contains(entry.path())) {
			if(!IsFileWatchable(entry)) {
				return;
			}

			m_Paths[entry.path()] = current_file_last_write_time;

			CallEvents(entry.path(), FileStatus::Created);
		} else if(m_Paths[entry.path()] != current_file_last_write_time) {
			m_Paths[entry.path()] = current_file_last_write_time;

			CallEvents(entry.path(), FileStatus::Modified);
		}
	}

	bool FileWatcher::IsFileWatchable(const std::filesystem::path &path) const
	{
		if(!std::filesystem::exists(path)) {
			return false;
		}

		if(std::filesystem::is_regular_file(path) && (m_WatchFlags & RegularFile) != 0) return true;
		if(std::filesystem::is_directory(path) && (m_WatchFlags & Directory) != 0) return true;
		if(std::filesystem::is_block_file(path) && (m_WatchFlags & BlockDevice) != 0) return true;
		if(std::filesystem::is_character_file(path) && (m_WatchFlags & CharacterFile) != 0) return true;
		if(std::filesystem::is_fifo(path) && (m_WatchFlags & IPC_Pipe) != 0) return true;
		if(std::filesystem::is_socket(path) && (m_WatchFlags & IPC_Socket) != 0) return true;
		if(std::filesystem::is_symlink(path) && (m_WatchFlags & Symlink) != 0) return true;

		return false;
	}

	bool FileWatcher::IsFileWatchable(const std::filesystem::directory_entry& entry) const
	{
		return IsFileWatchable(entry.path());
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

	void FileWatcher::SetEventHandler(const FileWatcher::FileWatchEvent &event)
	{
		m_SyncStatusEvent = event;
	}

	void FileWatcher::Update()
	{
		m_Mutex.lock();
		while(!m_SyncQueue.empty()) {
			auto val = m_SyncQueue.front();
			m_SyncQueue.pop();

			if(m_SyncStatusEvent) m_SyncStatusEvent(val.first, val.second);
		}
		m_Mutex.unlock();
	}

	void FileWatcher::CallEvents(const std::filesystem::path& path, const FileStatus& status)
	{
		/*std::filesystem::is_regular_file(std::filesystem::path(it->first))
		if(!path.has_extension()) {
			return;
		}*/


		if(m_AsyncStatusEvent) m_AsyncStatusEvent(path, status);

		if(m_SyncStatusEvent) {
			m_Mutex.lock();
			m_SyncQueue.push({path, status});
			m_Mutex.unlock();
		}
	}
}
