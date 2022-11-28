#pragma once

#include <RmlUi/Core/FileInterface.h>
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	class AU_API RmlUIFileInterface : public Rml::FileInterface
	{
	public:
		Rml::FileHandle Open(const Rml::String& path) override;
		void Close(Rml::FileHandle file) override;
		size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
		bool Seek(Rml::FileHandle file, long offset, int origin) override;
		size_t Tell(Rml::FileHandle file) override;

		size_t Length(Rml::FileHandle file) override;

		bool LoadFile(const Rml::String& path, Rml::String& out_data) override;
	};
}
