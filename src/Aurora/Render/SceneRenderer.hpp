#pragma once

#include <array>
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Core/Library.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Graphics/PassType.hpp"
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"

namespace Aurora
{
	class Scene;
	class FFrustum;
	class CameraComponent;
	class Actor;
	class SceneComponent;
	class MeshComponent;

	constexpr uint32_t MaxInstances = 1024;

	struct VisibleEntity
	{
		Aurora::Material* Material;
		Aurora::MeshComponent* MeshComponent;
		Aurora::Mesh* Mesh;
		uint MeshSection;
		LOD Lod;
		Matrix4 Transform;

		bool operator==(const VisibleEntity& other) const
		{
			return Material == other.Material && Mesh == other.Mesh && MeshSection == other.MeshSection;
		}

		bool operator!=(const VisibleEntity& other) const
		{
			return !operator==(other);
		}
	};

	struct ModelContext
	{
		Aurora::Material* Material;
		Aurora::Mesh* Mesh;
		MeshLodResource* LodResource;
		FMeshSection* MeshSection;
		Aurora::MeshComponent* MeshComponent;
		std::vector<Matrix4> Instances;
	};

	using RenderSet = std::vector<ModelContext>;

	typedef EventEmitter<PassType_t, DrawCallState&, CameraComponent*> PassRenderEventEmitter;

	struct OutlineActorSet
	{
		float Thickness = 2.0f;
		float CrossMaskOpacity = 0.9f;
		float IntersectionMaskOpacity = 0.5f;
		bool CrossMaskEnabled = false;
		bool CrossEnabled = false;
		FColor3 BaseColor = FColor3(0, 1, 0);
		FColor3 CrossColor = FColor3(0, 0.25f, 0);
		std::vector<Actor*> Actors;
		std::vector<SceneComponent*> Components;
	};

	class AU_API SceneRenderer
	{
	public:
		struct BloomSettings
		{
			bool Enabled = true;
			bool UseComputeShader = true;
			float Threshold = 1.1f;
			float Knee = 0.1f;
			float Intensity = 1.0f;
		};

		enum class ToneMapMode : uint8
		{
			Timothy,
			DX11DSK,
			Reinhard,
			Uncharted2,
			ACES,
			None
		};

		struct FToneMapSettings
		{
			bool LutToneMapEnabled = false;
			bool BasicToneMapEnabled = false;
			ToneMapMode BasicToneMapMode = ToneMapMode::None;
			Texture_ptr LutTexture = nullptr;

			[[nodiscard]] bool Enabled() const
			{
				return (LutToneMapEnabled && LutTexture != nullptr) || BasicToneMapEnabled;
			}
		};

		struct OutlineContext
		{
			std::vector<OutlineActorSet> Sets;
			bool ClearAfterFrame = true;

			inline OutlineActorSet& AddDefaultSet(const std::vector<Actor*>& actors, const std::vector<SceneComponent*>& components = {})
			{
				OutlineActorSet set;
				set.Actors = actors;
				set.Components = components;
				AddSet(set);
				return Sets.back();
			}
			inline void AddSet(const OutlineActorSet& set) { Sets.push_back(set); }
			inline void AddSet(OutlineActorSet&& set) { Sets.emplace_back(std::forward<OutlineActorSet>(set)); }
			inline void Clear() { Sets.clear(); }
		};

	protected:
		robin_hood::unordered_map<TTypeID, InputLayout_ptr> m_MeshInputLayouts;
		std::array<std::vector<VisibleEntity>, SortTypeCount> m_VisibleEntities;
		std::array<PassRenderEventEmitter, Pass::Count> m_InjectedPasses;

		Buffer_ptr m_InstancesBuffer;
		Buffer_ptr m_BaseVsDataBuffer;
		Buffer_ptr m_GlobDataBuffer;
		Buffer_ptr m_BonesBuffer;

		OutlineContext m_OutlineContext;

		BloomSettings m_BloomSettings;
		Shader_ptr m_BloomShader;
		Shader_ptr m_BloomShaderSS;
		Buffer_ptr m_BloomDescBuffer;
		const int m_BloomComputeWorkgroupSize = 16;

		float m_BloodEffect = 0.0f;
		float m_TargetBloodEffect = 0.0f;
	public:
		FToneMapSettings ToneMapSettings;
	public:

		SceneRenderer();
		virtual ~SceneRenderer() = default;

		virtual void LoadShaders();

		inline void ClearVisibleEntities()
		{
			for (int i = 0; i < SortTypeCount; ++i)
			{
				m_VisibleEntities[i].clear();
			}
		}

		void ShowBloodEffect(float intensity = 1.0f)
		{
			m_TargetBloodEffect = intensity;
		}

		void PrepareMeshComponent(MeshComponent* scene, CameraComponent* camera, const FFrustum& frustum);
		void PrepareVisibleEntities(Scene* scene, CameraComponent* camera, const FFrustum& frustum);
		void PrepareVisibleEntities(Actor* actor, CameraComponent* camera, const FFrustum& frustum);
		void FillRenderSet(RenderSet& renderSet, int numberOfPasses, ...);

		virtual void Render(Scene* scene, CameraComponent* debugCamera = nullptr) = 0;
		void RenderPass(PassType_t pass, DrawCallState& drawCallState, CameraComponent* camera, const RenderSet& renderSet, bool drawInjected = true);

		TemporalRenderTarget RenderBloom(const FViewPort& wp, const Texture_ptr& inputHDRRT);

		const InputLayout_ptr& GetInputLayoutForMesh(Mesh* mesh);
		PassRenderEventEmitter& GetPassEmitter(PassType_t passType) { return m_InjectedPasses[passType]; }

		BloomSettings& GetBloomSettings() { return m_BloomSettings; }
		OutlineContext& GetOutlineContext() { return m_OutlineContext; }
		[[nodiscard]] const OutlineContext& GetOutlineContext() const { return m_OutlineContext; }
	};
}