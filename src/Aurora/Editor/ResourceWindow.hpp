#pragma once

#include <vector>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Delegate.hpp"

#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/ImGuizmo.h"

namespace Aurora
{
	class MainEditorPanel;

	class ResourceWindow
	{
	private:
		MainEditorPanel* m_MainPanel;
		ImFont* m_BigIconFont;
		UniqueEvent m_FileDropEvent;

		Path m_CurrentPath;
		Path m_CurrentBasePath;
	public:
		explicit ResourceWindow(MainEditorPanel* mainEditorPanel);

		void Update(double delta);

		void OnFilesDrop(const std::vector<Path>& files);

	private:
		void DrawPathDirectoryNodes(const Path& rootPath, const Path& basePath);
	};
}