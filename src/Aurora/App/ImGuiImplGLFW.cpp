#include "ImGuiImplGLFW.hpp"

#include "GraphicsTypes.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "DebugUtilities.hpp"

namespace Aurora
{

	ImGuiImplGLFW::ImGuiImplGLFW(GLFWwindow *glfWwindow, IRenderDevice *pDevice, TEXTURE_FORMAT BackBufferFmt,
								 TEXTURE_FORMAT DepthBufferFmt, Uint32 InitialVertexBufferSize,
								 Uint32 InitialIndexBufferSize) :
			ImGuiImplDiligent{pDevice, BackBufferFmt, DepthBufferFmt, InitialVertexBufferSize, InitialIndexBufferSize}
	{
		ImGui_ImplGlfw_InitForVulkan(glfWwindow, true);

	}

	ImGuiImplGLFW::~ImGuiImplGLFW()
	{
		ImGui_ImplGlfw_Shutdown();
	}

	void ImGuiImplGLFW::NewFrame(Uint32 RenderSurfaceWidth, Uint32 RenderSurfaceHeight, SURFACE_TRANSFORM SurfacePreTransform)
	{
		VERIFY(SurfacePreTransform == SURFACE_TRANSFORM_IDENTITY, "Unexpected surface pre-transform");

		ImGui_ImplGlfw_NewFrame();
		ImGuiImplDiligent::NewFrame(RenderSurfaceWidth, RenderSurfaceHeight, SurfacePreTransform);

#ifdef DILIGENT_DEBUG
		{
			ImGuiIO& io = ImGui::GetIO();
			VERIFY(io.DisplaySize.x == 0 || io.DisplaySize.x == static_cast<float>(RenderSurfaceWidth),
				   "Render surface width (", RenderSurfaceWidth, ") does not match io.DisplaySize.x (", io.DisplaySize.x, ")");
			VERIFY(io.DisplaySize.y == 0 || io.DisplaySize.y == static_cast<float>(RenderSurfaceHeight),
				   "Render surface height (", RenderSurfaceHeight, ") does not match io.DisplaySize.y (", io.DisplaySize.y, ")");
		}
#endif
	}
}