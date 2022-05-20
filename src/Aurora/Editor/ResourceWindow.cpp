#include "ResourceWindow.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

#include "MainEditorPanel.hpp"

#include "AssetPreviewRenderer.hpp"

namespace Aurora
{
	ResourceWindow::ResourceWindow(MainEditorPanel* mainEditorPanel)
	: m_MainPanel(mainEditorPanel), m_CurrentPath(AURORA_PROJECT_DIR "/Assets"), m_CurrentBasePath(AURORA_PROJECT_DIR), m_TreeId(0)
	{
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 }; // Will not be copied by AddFont* so keep in scope.
		ImFontConfig config;
		config.MergeMode = false;
		//config.PixelSnapH = true;
		auto iconFontData = new std::vector<uint8>(GEngine->GetResourceManager()->LoadFile("Assets/Fonts/fa-solid-900.ttf"));
		m_BigIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(iconFontData->data(), iconFontData->size(), 56, &config, icons_ranges);

		m_FileDropEvent = GEngine->GetWindow()->GetFileDropEmitter().BindUnique(this, &ResourceWindow::OnFilesDrop);

		m_ShaderIcon = GEngine->GetResourceManager()->LoadResourceIcon("Assets/Textures/Editor/shader.png", 56);
		m_MaterialIcon = GEngine->GetResourceManager()->LoadResourceIcon("Assets/Textures/Editor/MaterialTexture.png", 56);

		m_PreviewRenderer = new AssetPreviewRenderer();
	}

	ResourceWindow::~ResourceWindow()
	{
		delete m_PreviewRenderer;
	}

	void ResourceWindow::DrawPathDirectoryNodes(const PathNode& rootPath, const Path& basePath)
	{
		CPU_DEBUG_SCOPE("DrawPathDirectoryNodes");

		ImGui::PushID(m_TreeId++);
		for (const PathNode& node : rootPath)
		{
			const Path& path = node.Path;

			if(!node.IsDirectory)
				continue;

			bool selected = path == m_CurrentPath;

			uint8_t flags = ImGuiTreeNodeFlags_OpenOnArrow;

			if(selected)
				flags |= ImGuiTreeNodeFlags_Selected;

			bool open = ImGui::TreeNodeEx(path.filename().string().c_str(), flags);

			if(ImGui::IsItemClicked())
			{
				m_CurrentPath = path;
				m_CurrentBasePath = basePath;
			}

			if(open)
			{
				DrawPathDirectoryNodes(node, basePath);
				ImGui::TreePop();
			}
		}
		ImGui::PopID();
	}

	void ResourceWindow::Update(double delta)
	{
		(void) delta;

		DrawCubeMapCreateWindow();
		DrawCreateMaterialInstanceWindow();

		CPU_DEBUG_SCOPE("ResourceWindow");

		LoadTexturePreviews();

		{ // Delete queued files
			CPU_DEBUG_SCOPE("DeleteQueue");

			while (!m_FilesToDelete.empty())
			{
				Path path = m_FilesToDelete.front();
				m_FilesToDelete.pop();

				// Just to ensure that path exists if somehow managed to add it to queue twice
				if (!std::filesystem::exists(path))
				{
					continue;
				}

				if (ResourceManager::IsFileType(path, static_cast<FileType>(FT_IMAGE | FT_CUBEMAP)))
				{
					m_TextureIcons.erase(path);
					VectorRemove(m_TextureIconsToLoad, path);
				}

				GEngine->GetResourceManager()->UnloadAsset(path);
				std::filesystem::remove(path);
			}
		}

		ImGui::Begin("Resources", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			CPU_DEBUG_SCOPE("ResourceWindowBegin");
			ImGui::PushFont(m_BigIconFont);
			const float BIG_ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_FOLDER).x;
			ImGui::PopFont();

			m_TreeId = 0;

			int tableFlags = ImGuiTableFlags_Resizable;
			if (ImGui::BeginTable("table-content-browser", 2, tableFlags))
			{
				ImGui::TableNextColumn();

				ImGui::BeginChild("resource-file-list");
				{
					for (const auto& path : GEngine->GetResourceManager()->GetFileSearchPaths())
					{
						String name = path.filename().string();
						String id = name;
						id.append("##header_");
						id.append(name);

						bool open = ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
						if (ImGui::IsItemClicked())
						{
							m_CurrentPath = path / "Assets";
							m_CurrentBasePath = path;
						}

#ifdef _WIN32
						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::Selectable("Open in explorer"))
							{
								ShellExecuteA(NULL, "open", ((path / "Assets").string()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
							}
							ImGui::EndPopup();
						}
#endif

						if (open)
						{
							FileTree* tree = GEngine->GetResourceManager()->GetFileTree(path / "Assets");
							DrawPathDirectoryNodes(*tree, path);
						}
					}
				}
				ImGui::EndChild();

				ImGui::TableNextColumn();

				ImGui::BeginChild("resource-file-folder-view-container");

				static String searchText;


				float regionAvail = ImGui::GetContentRegionAvail().x;

				Path baseAssetPath = m_CurrentBasePath / "Assets";

				{ // Top menu
					if (m_CurrentPath == baseAssetPath)
						ImGui::BeginDisabled();

					if (ImGui::IconButton(ICON_FA_ARROW_LEFT))
					{
						m_CurrentPath = m_CurrentPath.parent_path();
					}

					if(m_CurrentPath == baseAssetPath)
						ImGui::EndDisabled();

					ImGui::SameLine();
					ImGui::InputTextLabel(ICON_FA_SEARCH, searchText);
					ImGui::Separator();
				}

				ImGui::BeginChild("resource-file-folder-view");

				if (ImGui::BeginPopupContextWindow())
				{
					bool isResourceFocused = !m_FocusedResourceItem.empty();

					if (isResourceFocused)
					{
						if (ResourceManager::IsFileType(m_FocusedResourceItem, FT_MATERIAL_DEF))
						{
							if (ImGui::MenuItem("Create Material Instance from this"))
							{
								OpenCreateMaterialInstanceWindow(m_FocusedResourceItem);
							}
							ImGui::Separator();
						}
					}

					if (!isResourceFocused && ImGui::BeginMenu("Create"))
					{
						if (ImGui::MenuItem("CubeMap"))
						{
							OpenCubeMapCreateWindow();
						}

						if (ImGui::MenuItem("Material Instance"))
						{
							OpenCreateMaterialInstanceWindow("");
						}

						ImGui::EndMenu();
					}
#ifdef _WIN32
					if (ImGui::MenuItem("Open in explorer"))
					{
						ShellExecuteA(nullptr, "open", isResourceFocused ? m_FocusedResourceItem.string().c_str() : m_CurrentPath.string().c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
					}
#endif
					ImGui::EndPopup();
				}
				else
				{
					m_FocusedResourceItem = "";
				}

				bool foundPath = false;

				if (FileTree* tree = GEngine->GetResourceManager()->GetFileTree(m_CurrentBasePath / "Assets"))
				{
					if (PathNode* subNode = tree->Find(m_CurrentPath))
					{
						foundPath = true;

						float columnSize = BIG_ICON_BASE_WIDTH + 15;
						int columnCount = std::max(1, (int)floor(regionAvail / columnSize));

						if (ImGui::BeginTable("table-content-items", columnCount))
						{
							CPU_DEBUG_SCOPE("ResourceIcons");
							ImGui::TableNextColumn();

							if (searchText.empty())
							{
								for (const auto& directoryIt : *subNode)
								{
									if(directoryIt.IsDirectory)
										DrawFile(directoryIt, BIG_ICON_BASE_WIDTH);
								}

								for (const auto& directoryIt : *subNode)
								{
									if(!directoryIt.IsDirectory)
										DrawFile(directoryIt, BIG_ICON_BASE_WIDTH);
								}
							}
							else
							{
								std::vector<PathNode> foundFiles;
								tree->SearchFor(searchText, foundFiles, false);

								for (const auto& directoryIt : foundFiles)
								{
									DrawFile(directoryIt, BIG_ICON_BASE_WIDTH);
								}
							}

							ImGui::EndTable();
						}
					}
				}

				if (!foundPath)
				{
					ImGui::Text("Path not found %s!", m_CurrentPath.string().c_str());
					if (ImGui::Button("Click here to return to base path"))
					{
						m_CurrentPath = m_CurrentBasePath / "Assets";
					}
				}

				ImGui::EndChild();
				ImGui::EndChild();

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	void ResourceWindow::DrawFile(const PathNode& node_root, float iconSize)
	{
		CPU_DEBUG_SCOPE("DrawFile");

		const Path& path = node_root.Path;

		if(ResourceManager::IsIgnoredFileType(path))
			return;


		String fileName = path.filename().stem().string();

		{
			CPU_DEBUG_SCOPE("PushID");
			ImGui::PushID(fileName.c_str());
		}

		if (node_root.IsDirectory)
		{
			CPU_DEBUG_SCOPE("DirMode");
			ImGui::PushFont(m_BigIconFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
			ImGui::Selectable(ICON_FA_FOLDER);
			ImGui::PopStyleVar(2);
			ImGui::PopFont();

			if(ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
			{
				m_CurrentPath = path;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", path.filename().string().c_str());
				m_FocusedResourceItem = path;
			}
		}
		else
		{
			CPU_DEBUG_SCOPE("FileMode");
			bool selected = false;

			ImGui::PushFont(m_BigIconFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);

			Texture_ptr texture = nullptr;
			const char* icon = GetFileTypeIconOrTexture(path, texture);

			if (texture)
			{
				CPU_DEBUG_SCOPE("DrawImageButton");
				selected = EUI::ImageButton(texture, iconSize);
			}
			else
			{
				CPU_DEBUG_SCOPE("DrawIconButton");
				selected = ImGui::Selectable(icon);
			}

			ImGui::PopStyleVar(2);
			ImGui::PopFont();

			if (ImGui::BeginDragDropSource())
			{
				String strPath = std::filesystem::relative(path, m_CurrentBasePath).string();
				char rawPath[260];
				strcpy(rawPath, strPath.c_str());

				ImGui::SetDragDropPayload("RESOURCE_PATH", rawPath, strPath.length()+1);

				if (texture)
				{
					EUI::ImageButton(texture, iconSize);
				}
				else
				{
					ImGui::PushFont(m_BigIconFont);
					ImGui::Text("%s", icon);
					ImGui::PopFont();
				}

				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", path.filename().string().c_str());
				m_FocusedResourceItem = path;
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				SelectAsset(path);
			}

			/*if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("resourceManagerContextMenu");
			}*/

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				OpenAsset(path);
			}

			/*if (ImGui::BeginPopup("resourceManagerContextMenu"))
			{
				ImGui::Text("File menu");
				ImGui::Separator();
				if (ImGui::Button("Delete"))
				{
					QueueDeleteFile(path);
				}

				ImGui::EndPopup();
			}*/

		}

		ImGui::TextWrapped("%s", fileName.substr(0, std::min<int>(10, fileName.length())).c_str());

		ImGui::TableNextColumn();

		ImGui::PopID();
	}

	void ResourceWindow::OnFilesDrop(const std::vector<Path>& files)
	{
		for (const auto &item : files)
		{
			if(is_directory(item))
			{
				AU_LOG_WARNING("Cannot load directory: ", item.string());
				continue;
			}

			GEngine->GetResourceManager()->ImportAsset(item, m_CurrentPath);
		}
	}

	void ResourceWindow::LoadTexturePreviews()
	{
		CPU_DEBUG_SCOPE("LoadTexturePreviews");

		if (m_TextureIconsToLoad.empty())
			return;

		ImGui::PushFont(m_BigIconFont);
		const float BIG_ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_FOLDER).x;
		ImGui::PopFont();

		Path path = m_TextureIconsToLoad[0];
		m_TextureIconsToLoad.erase(m_TextureIconsToLoad.begin());

		if (ResourceManager::IsFileType(path, FT_CUBEMAP))
		{
			Texture_ptr cubeMap = GEngine->GetResourceManager()->LoadCubeMapDef(path);
			m_TextureIcons[path] = m_PreviewRenderer->RenderCubeMap(Vector2i(BIG_ICON_BASE_WIDTH), cubeMap);
			return;
		}

		if (ResourceManager::IsFileType(path, FT_MATERIAL_DEF))
		{
			auto mat = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition(path);
			m_TextureIcons[path] = m_PreviewRenderer->RenderMaterial(Vector2i(BIG_ICON_BASE_WIDTH), mat);
			return;
		}

		if (ResourceManager::IsFileType(path, FT_MATERIAL_INS))
		{
			auto mat = GEngine->GetResourceManager()->LoadMaterial(path);
			m_TextureIcons[path] = m_PreviewRenderer->RenderMaterial(Vector2i(BIG_ICON_BASE_WIDTH), mat);
			return;
		}

		TextureLoadDesc loadDesc = {
			.Width = static_cast<int>(BIG_ICON_BASE_WIDTH),
			.Height = static_cast<int>(BIG_ICON_BASE_WIDTH),
			.GenerateMips = false,
			.GenerateMetaFile = false,
			.ForceSRGB = false,
			.DoNotCache = true
		};

		Texture_ptr texture_icon = GEngine->GetResourceManager()->LoadTexture(path, loadDesc);
		m_TextureIcons[path] = texture_icon;
	}

	void ResourceWindow::QueueDeleteFile(const Path& path)
	{
		m_FilesToDelete.push(path);

		if(path == m_SelectedAssetPath)
		{
			m_SelectedAssetPath = "";
		}
	}

	void ResourceWindow::SelectAsset(const Path &path)
	{
		m_SelectedAssetPath = path;
	}

	void ResourceWindow::OpenAsset(const Path &path)
	{
		Path relativePath = std::filesystem::relative(path, m_CurrentBasePath);

		if (ResourceManager::IsFileType(relativePath, static_cast<FileType>(FT_MATERIAL_DEF | FT_MATERIAL_INS)))
		{
			m_MainPanel->GetMaterialWindow()->Open(relativePath);
		}
	}

	const char *ResourceWindow::GetFileTypeIconOrTexture(const Path &path, Texture_ptr &textureOut)
	{
		if (ResourceManager::IsFileType(path, static_cast<FileType>(FT_IMAGE | FT_CUBEMAP | FT_MATERIAL_DEF | FT_MATERIAL_INS)))
		{
			auto it = m_TextureIcons.find(path);

			if (it != m_TextureIcons.end())
			{
				textureOut = it->second;
			}
			else
			{
				if (!VectorContains(m_TextureIconsToLoad, path))
				{
					m_TextureIconsToLoad.push_back(path);
				}
			}
			return ICON_FA_IMAGE;
		}

		if (ResourceManager::IsFileType(path, FT_CUBEMAP))
		{
			return ICON_FA_IMAGE;
		}

		if (ResourceManager::IsFileType(path, FT_SHADER))
		{
			textureOut = m_ShaderIcon;
			return ICON_FA_FILE;
		}

		/*if (ResourceManager::IsFileType(path, static_cast<FileType>(FT_MATERIAL_DEF | FT_MATERIAL_INS)))
		{
			textureOut = m_MaterialIcon;
			return ICON_FA_FILE;
		}*/

		if (ResourceManager::IsFileType(path, FT_AMESH))
		{
			textureOut = nullptr;
			return ICON_FA_CUBE;
		}

		if (ResourceManager::IsFileType(path, FT_FONT))
		{
			textureOut = nullptr;
			return ICON_FA_FONT;
		}

		return ICON_FA_FILE;
	}

	void ResourceWindow::OpenCubeMapCreateWindow()
	{
		m_CubeMapCreateData.CubeMapWindowOpened = true;

		for (int i = 0; i < 6; ++i)
		{
			m_CubeMapCreateData.CubeMapTextures[i] = nullptr;
		}

		m_CubeMapCreateData.Filename = "";
	}

	static constexpr float CubeMapTileSize = 136;

	bool TextureSlot(const char* name, Texture_ptr* texture = nullptr)
	{
		if (!texture)
		{
			return false;
		}

		ImGui::PushID(name);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		bool pressed;

		if (*texture)
		{
			pressed = EUI::ImageButton(*texture, CubeMapTileSize);

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", name);
			}
		}
		else
		{
			pressed = ImGui::Button(name, ImVec2(CubeMapTileSize, CubeMapTileSize));
		}

		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor();

		if (Texture_ptr droppedTexture = EUI::AcceptTextureFileDrop())
		{
			*texture = droppedTexture;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("TextureSlot_Popup");
		}

		if (ImGui::BeginPopup("TextureSlot_Popup"))
		{
			if (ImGui::Button("Remove"))
			{
				*texture = nullptr;
			}

			ImGui::EndPopup();
		}

		ImGui::PopID();

		return pressed;
	}

	void ResourceWindow::DrawCubeMapCreateWindow()
	{
		if (!m_CubeMapCreateData.CubeMapWindowOpened)
			return;

		if (!ImGui::Begin("Create CubeMap", &m_CubeMapCreateData.CubeMapWindowOpened))
		{
			ImGui::End();
			return;
		}

		ImGui::InputTextLabel("File name: ", m_CubeMapCreateData.Filename);

		ImGui::Indent(CubeMapTileSize);
		TextureSlot("Top", &m_CubeMapCreateData.CubeMapTextures[2]);
		ImGui::Unindent(CubeMapTileSize);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);

		TextureSlot("Back", &m_CubeMapCreateData.CubeMapTextures[4]); ImGui::SameLine(0, 0);
		TextureSlot("Right", &m_CubeMapCreateData.CubeMapTextures[0]); ImGui::SameLine(0, 0);
		TextureSlot("Front", &m_CubeMapCreateData.CubeMapTextures[5]); ImGui::SameLine(0, 0);
		TextureSlot("Left", &m_CubeMapCreateData.CubeMapTextures[1]);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);

		ImGui::Indent(CubeMapTileSize);
		TextureSlot("Bottom", &m_CubeMapCreateData.CubeMapTextures[3]);
		ImGui::Unindent(CubeMapTileSize);

		ImGui::Separator();

		ImGui::BeginDisabled(m_CubeMapCreateData.Filename.empty());

		if (ImGui::Button("Create"))
		{
			Path cubeMapPath = m_CurrentPath / (m_CubeMapCreateData.Filename + ".cubemap");

			GEngine->GetResourceManager()->SaveCubeMapDef(cubeMapPath, m_CubeMapCreateData.CubeMapTextures);

			m_CubeMapCreateData.CubeMapWindowOpened = false;
		}

		ImGui::EndDisabled();

		ImGui::End();
	}

	void ResourceWindow::OpenCreateMaterialInstanceWindow(const Path& path)
	{
		m_CreatedMaterialInstancePopupOpen = true;

		m_SelectedMaterialDef = path;

		for (FileTreeContainer* treeContainer : GEngine->GetResourceManager()->GetFileTreeContainers())
		{
			treeContainer->Tree->SearchForFilesWithExtension(".matd", m_FoundMaterialDefs);
		}

		m_FoundMaterialDefsStrings.clear();
		m_SelectedFoundMaterialDef = 0;
		m_NewMaterialInstanceName = "";

		int i = 0;
		for (const auto& item: m_FoundMaterialDefs)
		{
			m_FoundMaterialDefsStrings.push_back(item.Path.filename().string());

			if (item.Path == path)
			{
				m_SelectedFoundMaterialDef = i;
			}

			i++;
		}
	}

	static bool String_ArrayGetter(void* data, int idx, const char** out_text)
	{
		std::vector<String>* nodes = reinterpret_cast<std::vector<String>*>(data);

		if (out_text)
			*out_text = nodes->at(idx).c_str();
		return true;
	}

	void ResourceWindow::DrawCreateMaterialInstanceWindow()
	{
		if (m_CreatedMaterialInstancePopupOpen)
		{
			m_CreatedMaterialInstancePopupOpen = false;
			ImGui::OpenPopup("Create Material Instance");
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Create Material Instance", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Combo("Material Definition", &m_SelectedFoundMaterialDef, String_ArrayGetter, (void*)&m_FoundMaterialDefsStrings, (int)m_FoundMaterialDefsStrings.size());
			ImGui::Separator();
			ImGui::InputTextLabel("Name", m_NewMaterialInstanceName);

			/*ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
			ImGui::Separator();

			static bool dont_ask_me_next_time = false;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
			ImGui::PopStyleVar();*/

			Path newInsPath = m_CurrentPath / (m_NewMaterialInstanceName + ".mat");

			bool exists = false;
			if (GEngine->GetResourceManager()->FileExists(newInsPath))
			{
				exists = true;
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "File already exists !");
			}

			if (m_NewMaterialInstanceName.empty() || exists)
			{
				ImGui::BeginDisabled();
			}

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				GEngine->GetResourceManager()->CreateMaterialInstance(newInsPath, m_FoundMaterialDefs[m_SelectedFoundMaterialDef].Path);
				ImGui::CloseCurrentPopup();
			}

			if (m_NewMaterialInstanceName.empty())
			{
				ImGui::EndDisabled();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}