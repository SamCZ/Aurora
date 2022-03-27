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

		ImGui::Begin("Resources");
		{
			ImGui::PushFont(m_BigIconFont);
			const float BIG_ICON_BASE_WIDTH = ImGui::CalcTextSize(ICON_FA_FOLDER).x;
			ImGui::PopFont();

			static const Path basePath = AURORA_PROJECT_DIR "/Assets";
			static String searchText;

			static ImVec2 sizeFirst(250, 0);
			static ImVec2 sizeSecond(0, 0);

			//EUI::Splitter(true, 0.0f, &sizeFirst.x, &sizeSecond.y, 0, 50);

			m_TreeId = 0;

			ImGui::BeginChild("resource-file-list", sizeFirst);
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

			ImGui::SameLine();

			ImGui::BeginChild("resource-file-folder-view", sizeSecond);
			//if(false)
			{
				{ // Top menu
					if(m_CurrentPath == basePath)
						ImGui::BeginDisabled();

					if(ImGui::IconButton(ICON_FA_ARROW_LEFT))
					{
						m_CurrentPath = m_CurrentPath.parent_path();
					}

					if(m_CurrentPath == basePath)
						ImGui::EndDisabled();

					ImGui::SameLine();
					ImGui::InputTextLabel(ICON_FA_SEARCH, searchText);
					ImGui::Separator();
				}

				Path currentRelative = std::filesystem::relative(m_CurrentPath, m_CurrentBasePath);

				ImGui::Text("%s", currentRelative.string().c_str());

				float regionAvail = ImGui::GetContentRegionAvail().x;
				float columnSize = BIG_ICON_BASE_WIDTH + 15;
				int columnCount = std::max(1, (int)floor(regionAvail / columnSize));
				ImGui::Columns(columnCount, "resource-columns", false);

				auto drawFile = [this, BIG_ICON_BASE_WIDTH](const std::filesystem::directory_entry& directoryIt) -> void
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

						if (ResourceManager::IsFileType(path, FT_IMAGE))
						{
							auto it = m_TextureIcons.find(path);

							if (it != m_TextureIcons.end())
							{
								selected = EUI::ImageButton(it->second, BIG_ICON_BASE_WIDTH);
							}
							else
							{
								if (!VectorContains(m_TextureIconsToLoad, path))
								{
									m_TextureIconsToLoad.push_back(path);
								}

								selected = ImGui::Selectable(ICON_FA_IMAGE);
							}
						}
						else
						{
							selected = ImGui::Selectable(ICON_FA_FILE);
						}

						ImGui::PopStyleVar(2);
						ImGui::PopFont();

						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("%s", path.filename().string().c_str());
						}

						if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
						{
							ImGui::OpenPopup("resourceManagerContextMenu");
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

					ImGui::NextColumn();

					ImGui::PopID();
				};

				for (auto& directoryIt : std::filesystem::directory_iterator(m_CurrentPath))
				{
					if(directoryIt.is_directory())
						drawFile(directoryIt);
				}

				for (auto& directoryIt : std::filesystem::directory_iterator(m_CurrentPath))
				{
					if(!directoryIt.is_directory())
						drawFile(directoryIt);
				}

				ImGui::Columns(1);
			}
			ImGui::EndChild();
		}
		ImGui::End();
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
	}
}