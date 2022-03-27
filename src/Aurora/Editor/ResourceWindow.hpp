#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

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

		int m_TreeId;

		std::map<Path, Texture_ptr> m_TextureIcons;
		std::vector<Path> m_TextureIconsToLoad;
		std::queue<Path> m_FilesToDelete;
	public:
		explicit ResourceWindow(MainEditorPanel* mainEditorPanel);

		void Update(double delta);

		void OnFilesDrop(const std::vector<Path>& files);

	private:
		void DrawPathDirectoryNodes(const Path& rootPath, const Path& basePath);
		void LoadTexturePreviews();
		void QueueDeleteFile(const Path& path);
	};
}