#pragma once

#include <vector>
#include <array>
#include <memory>
#include <map>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Common.hpp"

#include "Base/IRenderDevice.hpp"

namespace Aurora
{
	class Material;

	class TemporalRT
	{
	private:
		struct TempRTKey
		{
			int Width;
			int Height;
			GraphicsFormat Format;
			uint Flags;

			bool operator==(const TempRTKey& left) const
			{
				return Width == left.Width && Height == left.Height && Format == left.Format && Flags == left.Flags;
			}

			bool operator!=(const TempRTKey& left) const
			{
				return !operator==(left);
			}
		};
	private:
		friend class GraphicUtilities;

		String m_Name;
		TempRTKey m_Key;
		Texture_ptr m_Texture;
		bool m_InUse;
	public:
		explicit TemporalRT(const TempRTKey& key, Texture_ptr texture) : m_Key(key), m_Texture(std::move(texture)), m_InUse(true) {}

		explicit operator Texture_ptr()
		{
			return m_Texture;
		}

		void Free();

		[[nodiscard]] const String& Name() const { return m_Name; }
	private:
		inline void SetName(const String& name) { m_Name = name; }
	};

	class GraphicUtilities
	{
	private:
		friend class AuroraEngine;

		static std::vector<TemporalRT*> m_TemporalRenderTargets;
	public:
		static void Init();
		static void Destroy();
	public:
		static Texture_ptr CreateTextureArray(const std::vector<Path>& textures);
		static Texture_ptr CreateCubeMap(const std::array<Path, 6>& textures);
		static Texture_ptr CreateRenderTarget2D(const char* name, int width, int height, const GraphicsFormat& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav = false);
		static Texture_ptr CreateRenderTargetDepth2D(const char* name, int width, int height, const GraphicsFormat& format, bool useAsShaderResource, bool useUav = false);

		static void Blit(Texture_ptr src, Texture_ptr dest);
		static void Blit(std::shared_ptr<Material>& material, Texture_ptr src, Texture_ptr dest);

		static void Blit(std::shared_ptr<Material>& material, const std::map<String, Texture_ptr>& srcTextures, const Texture_ptr& dest, bool clear = true);

		static Texture_ptr GetPlaceholderTexture();
		static void SetPlaceholderTexture(Texture_ptr texture);

		static std::shared_ptr<Material> Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending = false);

		static TemporalRT* GetTemporalRT(const String& name, int width, int height, GraphicsFormat format, uint flags);
	};
}
