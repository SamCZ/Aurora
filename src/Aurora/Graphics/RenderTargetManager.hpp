#pragma once

#include <PipelineState.h>

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/Vector.hpp>
#include <Aurora/Graphics/Texture.hpp>

using namespace Diligent;

namespace Aurora
{
	struct TargetInfo
	{
		Texture_ptr Texture;
		TextureDesc TextureDesc;
		bool AutoResize;
	};

	AU_CLASS(RenderTargetPack)
	{
	public:
		friend class RenderTargetManager;
	private:
		std::unordered_map<String, std::pair<int, TargetInfo*>> m_Targets;
		TargetInfo* m_DepthTarget;
	public:
		inline RenderTargetPack() : m_Targets(), m_DepthTarget(nullptr) {}

		void Apply(bool clear = true);
		void Apply(GraphicsPipelineDesc& pipelineDesc);
	};

	AU_CLASS(RenderTargetManager)
	{
	private:
		std::map<String, TargetInfo> m_Targets;
		Vector2i m_LastSize;
		std::map<String, RenderTargetPack_ptr> m_Packs;
	public:
		RenderTargetManager();
	public:
		void AddTarget(const String& name, const TEXTURE_FORMAT& format, const Vector4& clearColor = Vector4(0, 0, 0, 0), bool useAsShaderResource = true, bool useUav = false, bool autoResize = true);

		Texture_ptr GetTarget(const String& name);
		ITextureView* GetTargetView(const String& name, const TEXTURE_VIEW_TYPE& viewType = TEXTURE_VIEW_RENDER_TARGET);

		void CreatePack(const String& name, const std::vector<String>& target_names, const String& depth_target_name = "__NONE__");
		RenderTargetPack_ptr GetPack(const String& name);

		void Resize(int width, int height);
		void Resize(const Vector2i& size);

		void Clear();
	};
}