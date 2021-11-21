#include "RmlUIFileInterface.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	struct AuRmlFileHandle
	{
		DataBlob Data;
		uint32_t Pos;
		uint32_t Length;
	};

	Rml::FileHandle RmlUIFileInterface::Open(const Rml::String &path)
	{
		DataBlob data = GetEngine()->GetResourceManager()->LoadFile(path);

		if(data.empty())
			return (Rml::FileHandle)nullptr;

		auto* handle = new AuRmlFileHandle();
		handle->Data = data;
		handle->Pos = 0;
		handle->Length = data.size();

		return (Rml::FileHandle)handle;
	}

	void RmlUIFileInterface::Close(Rml::FileHandle file)
	{
		if(!file) return;

		auto* handle = (AuRmlFileHandle*)file;
		delete handle;
	}

	size_t RmlUIFileInterface::Read(void *buffer, size_t size, Rml::FileHandle file)
	{
		if(!file) return 0;
		auto* handle = (AuRmlFileHandle*)file;

		std::memcpy(buffer, handle->Data.data() + handle->Pos, size);

		return size;
	}

	bool RmlUIFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
	{
		if(!file) return false;
		auto* handle = (AuRmlFileHandle*)file;
		handle->Pos = origin + offset;
		return handle->Pos < handle->Length - 1;
	}

	size_t RmlUIFileInterface::Tell(Rml::FileHandle file)
	{
		if(!file) return 0;

		auto* handle = (AuRmlFileHandle*)file;
		return handle->Pos;
	}

	size_t RmlUIFileInterface::Length(Rml::FileHandle file)
	{
		if(!file) return 0;

		auto* handle = (AuRmlFileHandle*)file;
		return handle->Length;
	}

	bool RmlUIFileInterface::LoadFile(const Rml::String &path, Rml::String &out_data)
	{
		out_data = GetEngine()->GetResourceManager()->LoadFileToString(path);
		return !out_data.empty();
	}
}