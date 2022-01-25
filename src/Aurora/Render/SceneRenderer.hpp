#pragma once

#include <map>
#include <array>
#include <functional>
#include <vector>
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/Mesh.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Framework/Entity.hpp"

#include "Shaders/World/instancing.h"

namespace Aurora
{
	class Scene;
	class Material;
	class XMesh;
	class FFrustum;

	struct VisibleEntity
	{
		Aurora::Material* Material;
		XMesh* Mesh;
		uint MeshSection;
		Matrix4 Transform;
	};

	struct ModelContext
	{
		Aurora::Material* Material;
		XMesh* Mesh;
		XMesh::PrimitiveSection* MeshSection;
		std::vector<Matrix4> Instances;
	};

	using RenderSet = std::vector<ModelContext>;

	struct LightSettings
	{
		bool ForceUpdate = true;
		Texture_ptr CustomSkyCubeMap = nullptr;
	};

	class SceneRenderer
	{
	public:
		struct BloomSettings
		{
			bool Enabled = true;
			float Threshold = 1.1f;
			float Knee = 0.1f;
			float UpsampleScale = 1.0f;
			float Intensity = 1.0f;
			float DirtIntensity = 1.0f;
		};

		struct DirLightShadowSettings
		{
			uint8_t NumOfCascades;
			std::vector<uint16_t> CascadeResolutions;
		};
		typedef EventEmitter<EPassType, DrawCallState&, FFrustum&, glm::mat4> PassRenderEventEmitter;
	private:
		Scene* m_Scene;
		RenderManager* m_RenderManager;
		IRenderDevice* m_RenderDevice;

		std::vector<entt::entity> m_VisibleEntities;
		std::map<TTypeID, uint> m_VisibleTypeCounters;
		entt::registry m_VisibleEntitiesRegistry;

		std::array<std::vector<entt::entity>, SortTypeCount> m_FinalSortedEntities;

		std::map<EPassType, PassRenderEventEmitter> m_InjectedPasses;

		Buffer_ptr m_InstancingBuffer;

		DirLightShadowSettings m_DirCascadeSettings;
		std::vector<Texture_ptr> m_DirCascadeTextures;
		std::vector<Matrix4> m_DirCascadesMatrices;
		std::vector<float> m_DirCascadesDistances;
		Buffer_ptr m_DirCascadeUniformBuffer;

		LightSettings m_LightSettings;
		Texture_ptr m_SkyTextureCubeMap;

		Shader_ptr m_PBRCompositeShader;
		Shader_ptr m_SkyShader;
		Shader_ptr m_RenderSkyCubeShader;
		Shader_ptr m_NormalBevelShader;
		Shader_ptr m_HDRShader;

		Shader_ptr m_PreFilterShader;
		Shader_ptr m_IRRCShader;

		// Bloom
		Shader_ptr m_BloomShader;
		const int m_BloomComputeWorkgroupSize = 16;
		/*Texture_ptr m_BloomTempRT[3];
		Vector2ui m_BloomTempRTSize;
		Vector3ui m_BloomGroupSize;*/
		Buffer_ptr m_BloomDescBuffer;
		BloomSettings m_BloomSettings;
		//

		Entity m_CurrentCameraEntity;

		Texture_ptr m_SkyCubeMap;
		Texture_ptr m_PreFilteredMap;
		Texture_ptr m_IRRCMap;
		Texture_ptr m_BrdfMap;
	public:
		SceneRenderer(Scene* scene, RenderManager* renderManager, IRenderDevice* renderDevice);
		~SceneRenderer();

		Texture_ptr RenderPreethamSky(const Vector2ui& resolution, float turbidity, float azimuth, float inclination);
		Texture_ptr RenderPrefilterEnvMap(const Texture_ptr& envMap);
		Texture_ptr RenderIRRConvMap(const Texture_ptr& envMap);
		void CreateBrdfMap();

		void AddVisibleEntity(Material* material, XMesh* mesh, uint meshSection, const Matrix4& transform);

		void PrepareRender(FFrustum* frustum);
		void SortVisibleEntities();

		RenderSet BuildRenderSet();

		void Render(entt::entity cameraEntityID);
		void RenderPass(DrawCallState& drawCallState, const std::vector<ModelContext>& modelContexts, EPassType passType, FFrustum& frustum, glm::mat4 viewMatrix);

		BloomSettings& GetBloomSettings() { return m_BloomSettings; }
		LightSettings& GetLightSettings() { return m_LightSettings; }

		inline void InjectRenderToPass(EPassType passType, PassRenderEventEmitter::Delegate delegate)
		{
			m_InjectedPasses[passType].Bind(delegate);
		}

	private:
		std::vector<std::pair<glm::mat4, glm::mat4>> GetLightSpaceMatrices(std::vector<float> rations, const CameraComponent* mainCamera, const Matrix4& cameraViewMatrix, const Vector3& lightDir);
	};
}
