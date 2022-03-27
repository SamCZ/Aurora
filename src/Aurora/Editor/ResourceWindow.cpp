#include "ResourceWindow.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/MeshComponent.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

#include "MainEditorPanel.hpp"

#include <climits>

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

		m_ShaderIcon = GEngine->GetResourceManager()->LoadIcon("Assets/Textures/Editor/shader.png", 56);
	}

	void ResourceWindow::DrawPathDirectoryNodes(const Path& rootPath, const Path& basePath)
	{
		ImGui::PushID(m_TreeId++);
		for (auto& directoryIt : std::filesystem::directory_iterator(rootPath))
		{
			Path path = directoryIt.path();

			if(!directoryIt.is_directory())
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
				DrawPathDirectoryNodes(path, basePath);
				ImGui::TreePop();
			}
		}
		ImGui::PopID();
	}

	void ResourceWindow::Update(double delta)
	{
		(void) delta;

		LoadTexturePreviews();

		{ // Delete queued files
			while (!m_FilesToDelete.empty())
			{
				Path path = m_FilesToDelete.front();
				m_FilesToDelete.pop();

				// Just to ensure that path exists if somehow managed to add it to queue twice
				if (!std::filesystem::exists(path))
				{
					continue;
				}

				if (ResourceManager::IsFileType(path, FT_IMAGE))
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

						if (open)
						{
							DrawPathDirectoryNodes(path / "Assets", path);
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

				float columnSize = BIG_ICON_BASE_WIDTH + 15;
				int columnCount = std::max(1, (int)floor(regionAvail / columnSize));

				if (ImGui::BeginTable("table-content-items", columnCount))
				{
					ImGui::TableNextColumn();

					for (auto& directoryIt : std::filesystem::directory_iterator(m_CurrentPath))
					{
						if(directoryIt.is_directory())
							DrawFile(directoryIt, BIG_ICON_BASE_WIDTH);
					}

					for (auto& directoryIt : std::filesystem::directory_iterator(m_CurrentPath))
					{
						if(!directoryIt.is_directory())
							DrawFile(directoryIt, BIG_ICON_BASE_WIDTH);
					}


					ImGui::EndTable();
				}

				ImGui::EndChild();
				ImGui::EndChild();

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	void ResourceWindow::DrawFile(const std::filesystem::directory_entry& directoryIt, float iconSize)
	{
		const Path& path = directoryIt.path();

		if(ResourceManager::IsIgnoredFileType(path))
			return;

		String fileName = path.filename().stem().string();

		ImGui::PushID(fileName.c_str());


		if(directoryIt.is_directory())
		{
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
		}
		else
		{
			bool selected = false;

			ImGui::PushFont(m_BigIconFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);

			Texture_ptr texture = nullptr;
			const char* icon = GetFileTypeIconOrTexture(path, texture);

			if (texture)
			{
				selected = EUI::ImageButton(texture, iconSize);
			}
			else
			{
				selected = ImGui::Selectable(icon);
			}

			ImGui::PopStyleVar(2);
			ImGui::PopFont();

			if (ImGui::BeginDragDropSource())
			{
				String strPath = path.string();
				char rawPath[PATH_MAX];
				strcpy(rawPath, strPath.c_str());

				ImGui::SetDragDropPayload("RESOURCE_PATH", rawPath, strPath.length()+1);

				{ // Move this to some method to remove duplicated code
					if (ResourceManager::IsFileType(path, FT_IMAGE))
					{
						auto it = m_TextureIcons.find(path);

						if (it != m_TextureIcons.end())
						{
							EUI::ImageButton(it->second, iconSize);
						}
						else
						{
							ImGui::Text(ICON_FA_IMAGE);
						}
					}
					else
					{
						ImGui::Text(ICON_FA_FILE);
					}
				}

				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", path.filename().string().c_str());
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				SelectAsset(path);
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("resourceManagerContextMenu");
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				OpenAsset(path);
			}

			if (ImGui::BeginPopup("resourceManagerContextMenu"))
			{
				ImGui::Text("File menu");
				ImGui::Separator();
				if (ImGui::Button("Delete"))
				{
					QueueDeleteFile(path);
				}

				ImGui::EndPopup();
			}

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
		if (m_TextureIconsToLoad.empty())
			return;

		ImGui::PushFont(m_BigIconFont);
		const float BIG_ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_FOLDER).x;
		ImGui::PopFont();

		Path path = m_TextureIconsToLoad[0];
		m_TextureIconsToLoad.erase(m_TextureIconsToLoad.begin());

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
		if (ResourceManager::IsFileType(path, FT_IMAGE))
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

		if (ResourceManager::IsFileType(path, FT_SHADER))
		{
			textureOut = m_ShaderIcon;
			return ICON_FA_FILE;
		}

		return ICON_FA_FILE;
	}
}