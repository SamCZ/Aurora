#pragma once

#include "SceneComponent.hpp"
#include "Aurora/Core/Delegate.hpp"
#include "Aurora/Graphics/PassType.hpp"
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Physics/Frustum.hpp"
#include "Aurora/Physics/Ray.hpp"

namespace Aurora
{
	struct RenderViewPort;

	class AU_API CameraComponent : public SceneComponent
	{
	public:
		enum class ProjectionType : uint8_t
		{
			None = 0,
			Perspective,
			Orthogonal
		};

		struct PerspectiveSettings
		{
			float Aspect;
			float Fov;
			float Near;
			float Far;
		};

		struct OrthogonalSettings
		{
			float Left;
			float Right;
			float Bottom;
			float Top;
			float Near;
			float Far;
		};
	private:
		PassType_t m_SelectedPass;
		RenderViewPort* m_ViewPort;
		UniqueEvent m_ViewPortEvent;

		ProjectionType m_ProjectionType;
		PerspectiveSettings m_PerspectiveSettings;
		OrthogonalSettings m_OrthogonalSettings;

		Matrix4 m_Projection = glm::identity<Matrix4>();
		Matrix4 m_View = glm::identity<Matrix4>();
		FFrustum m_Frustum;

		FColor m_ClearColor;
	public:
		CLASS_OBJ(CameraComponent, SceneComponent);

		CameraComponent();
		~CameraComponent() override;

		[[nodiscard]] const Matrix4& GetProjectionMatrix() const { return m_Projection; }
		[[nodiscard]] const Matrix4& GetViewMatrix()
		{
			// TODO: think about updating only when transform changes too
			m_View = glm::inverse(GetTransformationMatrix());
			return m_View;
		}
		[[nodiscard]] Matrix4 GetProjectionViewMatrix() const { return m_Projection * m_View; }
		[[nodiscard]] ProjectionType GetProjectionType() const { return m_ProjectionType; }

		void SetPerspective(float fov, float near, float far);
		void SetOrthographic(float left, float right, float bottom, float top, float near, float far);

	private:
		void Resize(const Vector2i&);
	public:
		void UpdateFrustum();

		void Tick(double delta) override;

		[[nodiscard]] Vector3 GetWorldPositionFromScreen(float x, float y, float projectionZPos) const;
		bool GetScreenCoordinatesFromWorld(const Vector3& position, Vector2& out_ScreenPos) const;

		[[nodiscard]] Ray GetRayFromScreen(float x, float y) const;
		[[nodiscard]] Ray GetRayFromScreen(int x, int y) const;
		[[nodiscard]] Ray GetRayFromScreen(const Vector2i& screenPos) const;
		[[nodiscard]] Ray GetRay() const;

		[[nodiscard]] const PerspectiveSettings& GetPerspectiveSettings() const;
		[[nodiscard]] const OrthogonalSettings& GetOrthogonalSettings() const;

		inline void SetPass(PassType_t pass) { m_SelectedPass = pass; }
		[[nodiscard]] inline PassType_t GetPass() const { return m_SelectedPass; }

		void SetViewPort(RenderViewPort* wp);
		[[nodiscard]] inline RenderViewPort* GetViewPort() const { return m_ViewPort; }

		const FFrustum& GetFrustum() const { return m_Frustum; }

		inline void SetClearColor(const FColor& color) { m_ClearColor = color; }
		[[nodiscard]] inline const FColor& GetClearColor() const { return m_ClearColor; }
	};
}
