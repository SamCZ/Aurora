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
	: m_MainPanel(mainEditorPanel), m_CurrentPath(AURORA_PROJECT_DIR "/Assets"), m_CurrentBasePath(AURORA_PROJECT_DIR)
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
	}

	void ResourceWindow::Update(double delta)
	{
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

			ImGui::BeginChild("resource-file-list", sizeFirst);
			{
				for (const auto& path : GEngine->GetResourceManager()->GetFileSearchPaths())
				{
					String name = path.filename().string();

					bool open = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
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

				auto drawFile = [this](const std::filesystem::directory_entry& directoryIt) -> void
				{
					const Path& path = directoryIt.path();

					String fileName = path.filename().string();

					ImGui::PushID(fileName.c_str());

					ImGui::PushFont(m_BigIconFont);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
					if(directoryIt.is_directory())
					{
						ImGui::Selectable(ICON_FA_FOLDER);
						if(ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
						{
							m_CurrentPath = path;
						}
					}
					else
					{
						if(ImGui::Selectable(ICON_FA_FILE))
						{

						}
					}
					ImGui::PopStyleVar(2);
					ImGui::PopFont();

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
}