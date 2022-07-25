#pragma once

#include <fbxsdk.h>
#include <vector>
#include <Aurora/Logger/Logger.hpp>

class MemoryFbxStream : public FbxStream
{
private:
	EState m_State = EState::eEmpty;
	std::vector<uint8_t> m_Data;
	mutable unsigned long long m_Position;
	unsigned long long m_Size;
public:
	explicit MemoryFbxStream(const std::vector<uint8_t>& data) : m_Data(data), m_Position(0), m_Size(data.size())
	{
		if (m_Size != 0)
			m_State = eClosed;
	}
	MemoryFbxStream() : m_Data(), m_Position(0), m_Size(0) { }
public:
	EState GetState() override
	{
		return m_State;
	}

	bool Open(void* pStreamData) override
	{
		if (m_Size == 0)
		{
			return false;
		}

		m_State = eOpen;

		return true;
	}

	bool Close() override
	{
		m_State = eClosed;
		return true;
	}

	bool Flush() override
	{
		m_Data.clear();
		m_Size = 0;
		m_Position = 0;

		return true;
	}

	int Write(const void* pData, int pSize) override
	{
		return 0;
	}

	int Read(void* pData, int pSize) const override
	{
		size_t maxRead = pSize;
		if(m_Position + pSize > m_Size)
		{
			maxRead = pSize - ((m_Position + pSize) - m_Size);
		}

		std::memcpy(pData, m_Data.data() + m_Position, maxRead);

		m_Position += maxRead;

		return maxRead;
	}

	[[nodiscard]] int GetReaderID() const override
	{
		return 0;
	}

	[[nodiscard]] int GetWriterID() const override
	{
		return -1;
	}

	void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos) override
	{
		if (pSeekPos == fbxsdk::FbxFile::eBegin)
		{
			m_Position = pOffset;
		}
		else if (pSeekPos == fbxsdk::FbxFile::eCurrent)
		{
			m_Position += pOffset;
		}
		else if (pSeekPos == fbxsdk::FbxFile::eEnd)
		{
			m_Position = m_Size - pOffset;
		}
	}

	[[nodiscard]] long GetPosition() const override
	{
		return long(m_Position);
	}

	void SetPosition(long pPosition) override
	{
		m_Position = pPosition;
	}

	[[nodiscard]] int GetError() const override
	{
		return 0;
	}

	void ClearError() override
	{

	}
};
