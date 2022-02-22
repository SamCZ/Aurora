#pragma once

#include <assimp/Importer.hpp>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"

namespace Aurora
{
	class AssimpModelLoader
	{
	private:
		Assimp::Importer m_Importer;
	public:
		void ImportModel(const String& name, const DataBlob& data);
	};
}
