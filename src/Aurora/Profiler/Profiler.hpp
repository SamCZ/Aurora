#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <map>

#include <GLFW/glfw3.h>

#define AU_PROFILER_ENABLED

namespace Aurora
{
	class Profiler
	{
	public:
		struct ProfileNode
		{
			friend class Profiler;

			std::string Name;
			double BeginTime;
			double EndTime;

			ProfileNode* ParentNode;

			std::vector<ProfileNode> Childs;

			ProfileNode() : Name(), BeginTime(0), EndTime(0), ParentNode(nullptr), Childs() { }

			explicit ProfileNode(std::string name) : Name(std::move(name)), BeginTime(0), EndTime(0), ParentNode(nullptr), Childs() {}

			[[nodiscard]] inline double GetElapsedTimeMillis() const
			{
				return (EndTime - BeginTime) * 1000.0;
			}

		private:
			inline void UpdateRootNodes()
			{
				ParentNode = nullptr;

				for(ProfileNode& child : Childs) {
					child.UpdateRootNodes();
					child.ParentNode = this;
				}
			}

			inline void Begin()
			{
				BeginTime = glfwGetTime();
			}

			inline void End()
			{
				EndTime = glfwGetTime();
			}

			inline void Clear()
			{
				/*for(ProfileNode* child : Childs) {
					child->Clear();
					delete child;
				}*/
				Childs.clear();

				BeginTime = 0;
				EndTime = 0;
			}
		};
	private:
		static ProfileNode rootNode;
		static ProfileNode* m_CurrentNode;
		static ProfileNode m_LastFrameRootNode;
		static bool m_IsPaused;
	public:
#ifdef AU_PROFILER_ENABLED
		static void Begin(const std::string& name);
		static void End(const std::string& name);

		static void RestartProfiler();
		static void Finalize();
#else
		inline static void Begin(const std::string& name) { (void)name; }
		inline static void End(const std::string& name) { (void)name; }

		inline static void RestartProfiler() {}
		inline static void Finalize() {}
#endif

		inline static const ProfileNode* GetRootNode() { return &rootNode; }

		static void DrawWithImGui(bool createWindow = true);
	};
}
