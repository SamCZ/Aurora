#pragma once

#include <functional>
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "SceneComponent.hpp"
#include "Actor.hpp"

namespace Aurora
{
	class CameraComponent;
	class FFrustum;

	struct ShadowLayerData
	{
		Vector2i Resolution;
		Matrix4 ShadowCameraMatrix;
		Vector2 OrthographicSize;
		float OrthographicNear, OrthographicFar;
		float CutPlane;

		explicit ShadowLayerData(const Vector2i& resolution) : Resolution(resolution) { }
	};

	class LightComponent : public SceneComponent
	{
	private:
		float m_Intensity;
		bool m_CastShadows;
		Vector3 m_LightColor;
	public:
		Texture_ptr RenderTexture = nullptr;
	public:
		CLASS_OBJ(LightComponent, SceneComponent);

		LightComponent() : m_Intensity(1.0f), m_CastShadows(true), m_LightColor(1.0f) {}

		[[nodiscard]] float GetIntensity() const { return m_Intensity; }
		[[nodiscard]] float& GetIntensity() { return m_Intensity; }
		[[nodiscard]] Color GetColor() const { return m_LightColor; }
		[[nodiscard]] Vector3& GetColor() { return m_LightColor; }
		bool CastShadows() const { return m_CastShadows; }
		bool& CastShadows() { return m_CastShadows; }

		[[nodiscard]] Vector3 GetDirection() const { return -glm::normalize(GetForwardVector()); }
	};

	typedef std::function<void(CameraComponent*, const FFrustum* frustum, const Matrix4& lightViewMatrix, int layer)> LightRenderFnc;

	class DirectionalLightComponent : public LightComponent
	{
	public:
		std::vector<ShadowLayerData> Layers;
		std::vector<Matrix4> ShadowMatrices;
	public:
		CLASS_OBJ(DirectionalLightComponent, LightComponent);

		void SetupShadowmaps(int32 numShadowLevels, const Vector2i& size);
		void SetTarget(CameraComponent* mainCamera);
		void SetupSplitDistances(float zNear, float zFar, const float power);

		float CutZ(const int layer) const { return Layers[layer].CutPlane; }

		void Render(DrawCallState& drawCallState, const LightRenderFnc& renderCallback);

		static std::vector<Vector3> FrustumCorners(const Matrix4& imvp, float z0, float z1);
	private:
		std::vector<Vector3> LayerFrustumCorners(CameraComponent* mainCamera, int layer);
		std::vector<Vector3> CameraFrustumCorners(CameraComponent* mainCamera, float z0, float z1);

		std::vector<Vector4> CalculateClipPlanes(const Matrix4& lightProjection);
	};

	class PointLightComponent : public LightComponent
	{
	private:
		float m_Radius = 10.0f;
	public:
		CLASS_OBJ(PointLightComponent, LightComponent);

		[[nodiscard]] float GetRadius() const { return m_Radius; }
		[[nodiscard]] float& GetRadius() { return m_Radius; }
	};

	class SpotLightComponent : public LightComponent
	{
	public:
		CLASS_OBJ(SpotLightComponent, LightComponent);
	};

	///////////////////////////////////////////////////////////////////////////////////

	class LightBase : public Actor
	{
	public:
		CLASS_OBJ(LightBase, Actor);
	};

	class DirectionalLight : public LightBase
	{
	public:
		CLASS_OBJ(DirectionalLight, LightBase);
		DEFAULT_COMPONENT(DirectionalLightComponent);
	};

	class PointLight : public LightBase
	{
	public:
		CLASS_OBJ(PointLight, LightBase);
		DEFAULT_COMPONENT(PointLightComponent);
	};

	class SpotLight : public LightBase
	{
	public:
		CLASS_OBJ(SpotLight, LightBase);
		DEFAULT_COMPONENT(SpotLightComponent);
	};
}