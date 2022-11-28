#include "GLBufferLock.hpp"
#include "Aurora/Core/assert.hpp"

namespace Aurora
{
	GLuint64 kOneSecondInNanoSeconds = 1000000000;

	BufferLockManager::BufferLockManager(bool cpuUpdates) : m_CPUUpdates(cpuUpdates)
	{

	}

	BufferLockManager::~BufferLockManager()
	{
		for (auto & m_BufferLock : m_BufferLocks) {
			Cleanup(&m_BufferLock);
		}

		m_BufferLocks.clear();
	}


	void BufferLockManager::WaitForLockedRange(size_t _lockBeginBytes, size_t _lockLength)
	{
		BufferRange testRange = { _lockBeginBytes, _lockLength };
		std::vector<BufferLock> swapLocks;
		for (auto& m_BufferLock : m_BufferLocks)
		{
			if (testRange.Overlaps(m_BufferLock.mRange)) {
				Wait(&m_BufferLock.mSyncObj);
				Cleanup(&m_BufferLock);
			} else {
				swapLocks.push_back(m_BufferLock);
			}
		}

		m_BufferLocks.swap(swapLocks);
	}

	void BufferLockManager::LockRange(size_t _lockBeginBytes, size_t _lockLength)
	{
		BufferRange newRange = { _lockBeginBytes, _lockLength };
		GLsync syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		BufferLock newLock = { newRange, syncName };

		m_BufferLocks.push_back(newLock);
	}

	void BufferLockManager::Wait(GLsync* _syncObj)
	{
		if  (m_CPUUpdates)
		{
			GLbitfield waitFlags = 0;
			GLuint64 waitDuration = 0;
			while (true)
			{
				GLenum waitRet = glClientWaitSync(*_syncObj, waitFlags, waitDuration);
				if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
					return;
				}

				if (waitRet == GL_WAIT_FAILED) {
					au_assert(!"Not sure what to do here. Probably raise an exception or something.");
					return;
				}

				// After the first time, need to start flushing, and wait for a looong time.
				waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
				waitDuration = kOneSecondInNanoSeconds;
			}
		}
		else
		{
			glWaitSync(*_syncObj, 0, GL_TIMEOUT_IGNORED);
		}
	}

	void BufferLockManager::Cleanup(BufferLock* _bufferLock)
	{
		glDeleteSync(_bufferLock->mSyncObj);
	}
}