#pragma once

#include <cstdint>
#include <vector>
#include "GL.hpp"

namespace Aurora
{
	struct BufferRange
	{
		size_t StartOffset;
		size_t Length;

		[[nodiscard]] bool Overlaps(const BufferRange& _rhs) const {
			return StartOffset < (_rhs.StartOffset + _rhs.Length)
				&& _rhs.StartOffset < (StartOffset + Length);
		}
	};

	struct BufferLock
	{
		BufferRange mRange;
		GLsync mSyncObj;
	};

	class BufferLockManager
	{
	private:
		std::vector<BufferLock> m_BufferLocks;
		// Whether it's the CPU (true) that updates, or the GPU (false)
		bool m_CPUUpdates;
	public:
		explicit BufferLockManager(bool cpuUpdates);
		~BufferLockManager();

		void WaitForLockedRange(size_t lockBeginBytes, size_t lockLength);
		void LockRange(size_t lockBeginBytes, size_t lockLength);
	private:
		void Wait(GLsync* syncObj);
		void Cleanup(BufferLock* bufferLock);
	};
}