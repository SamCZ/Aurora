#pragma once

#include <cstdint>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <map>
#include <functional>

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
		typedef std::function<void(const std::filesystem::path&, const FileStatus&)> FileWatchEvent;
	private:
		std::filesystem::path m_WatchPath;
		std::chrono::duration<int64_t, std::milli> m_Delay;
		bool m_Recursive;
		std::thread m_Thread;
		std::atomic_bool m_IsRunning;

		std::map<std::filesystem::path, std::filesystem::file_time_type> m_Paths;

		FileWatchEvent m_AsyncStatusEvent;
	public:
		FileWatcher(std::filesystem::path watchPath, bool recursive, std::chrono::duration<int64_t, std::milli> delay);
		~FileWatcher();

		void SetAsyncEventHandler(const FileWatchEvent& event);

		void Stop();
	private:
		void Update();

	public:
		static std::string FileStatusToString(const FileStatus& fileStatus);
	};
}
