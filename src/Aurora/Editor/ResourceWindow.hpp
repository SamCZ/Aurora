#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"
#include "Aurora/Resource/FileTree.hpp"

#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/ImGuizmo.h"

#include "WindowBase.hpp"

namespace Aurora
{
	class MainEditorPanel;
	class AssetPreviewRenderer;

	class ResourceWindow
	{
	private:
		MainEditorPanel* m_MainPanel;
		ImFont* m_BigIconFont;
		UniqueEvent m_FileDropEvent;

		Path m_CurrentPath;
		Path m_CurrentBasePath;
		Path m_SelectedAssetPath;

		int m_TreeId;

		std::map<Path, Texture_ptr> m_TextureIcons;
		std::vector<Path> m_TextureIconsToLoad;
		std::queue<Path> m_FilesToDelete;
		AssetPreviewRenderer* m_PreviewRenderer;

		std::queue<Path> m_FilesToImport;
		Path m_CurrentFileToImport;

		// Icons
		Texture_ptr m_ShaderIcon;
		Texture_ptr m_MaterialIcon;

		// CubeMapWindow
		struct CubeMapCreateData
		{
			bool CubeMapWindowOpened = false;
			String Filename;
			std::array<Texture_ptr, 6> CubeMapTextures;
		} m_CubeMapCreateData;

		Path m_FocusedResourceItem;

		bool m_CreatedMaterialInstancePopupOpen = false;
		Path m_SelectedMaterialDef;
		std::vector<PathNode> m_FoundMaterialDefs;
		std::vector<String> m_FoundMaterialDefsStrings;
		int m_SelectedFoundMaterialDef = 0;
		String m_NewMaterialInstanceName;
	public:
		explicit ResourceWindow(MainEditorPanel* mainEditorPanel);
		~ResourceWindow();

		void Update(double delta);

		inline const Path& GetSelectedAssetPath() const { return m_SelectedAssetPath; }
	private:
		void DrawFile(const PathNode& directoryIt, float iconSize);
		void OnFilesDrop(const std::vector<Path>& files);
		void DrawPathDirectoryNodes(const PathNode& rootPath, const Path& basePath);
		void LoadTexturePreviews();
		void QueueDeleteFile(const Path& path);
		void SelectAsset(const Path& path);
		void OpenAsset(const Path& path);

		void OpenCubeMapCreateWindow();
		void DrawCubeMapCreateWindow();

		void OpenCreateMaterialInstanceWindow(const Path& path);
		void DrawCreateMaterialInstanceWindow();
	public:
		const char* GetFileTypeIconOrTexture(const Path& path, Texture_ptr& textureOut);
	};


}