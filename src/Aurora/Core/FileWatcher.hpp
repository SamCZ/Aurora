#pragma once

#include <cstdint>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <map>
#include <functional>

#include <queue>
#include <mutex>

namespace Aurora
{
	class FileWatcher
	{
	public:
		enum class FileStatus : uint8_t
		{
			Created = 0,
			Modified,
			Deleted
		};

		enum FileWatchType
		{
			RegularFile = 1u << 0u,
			Directory = 1u << 1u,
			BlockDevice = 1u << 2u,
			CharacterFile = 1u << 3u,
			IPC_Pipe = 1u << 4u,
			IPC_Socket = 1u << 5u,
			Symlink = 1u << 6u,
		};

		typedef std::function<void(const std::filesystem::path&, const FileStatus&)> FileWatchEvent;
		typedef uint16_t WatchFlags;
	private:
		std::filesystem::path m_WatchPath;
		std::chrono::duration<int64_t, std::milli> m_Delay;
		bool m_Recursive;
		uint16_t m_WatchFlags;

		std::thread m_Thread;
		std::atomic_bool m_IsRunning;

		std::map<std::filesystem::path, std::filesystem::file_time_type> m_Paths;

		FileWatchEvent m_AsyncStatusEvent;
		FileWatchEvent m_SyncStatusEvent;

		std::queue<std::pair<std::filesystem::path, FileStatus>> m_SyncQueue;
		mutable std::mutex m_Mutex;
	public:
		FileWatcher(std::filesystem::path watchPath, bool recursive, std::chrono::duration<int64_t, std::milli> delay, WatchFlags watchFlags = RegularFile);
		~FileWatcher();

		void SetAsyncEventHandler(const FileWatchEvent& event);
		void SetEventHandler(const FileWatchEvent& event);
		void Stop();
		void Update();
	private:
		void UpdateThread();
		void CallEvents(const std::filesystem::path& path, const FileStatus& status);
		void UpdateFileStatus(const std::filesystem::directory_entry& entry);
		bool IsFileWatchable(const std::filesystem::path& path) const;
		bool IsFileWatchable(const std::filesystem::directory_entry& entry) const;
	public:
		static std::string FileStatusToString(const FileStatus& fileStatus);
	};
}
