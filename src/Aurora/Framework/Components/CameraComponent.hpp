#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/Vector.hpp>
#include <Aurora/Physics/Frustum.hpp>

#include <Aurora/Framework/Components/Base/SceneComponent.hpp>

namespace Aurora
{
	class CameraComponent final : public SceneComponent
	{
	public:
		enum class EViewMode
		{
			Perspective,
			Orthographic,
			Custom
		};
	private:
		uint m_CameraID;
		EViewMode m_LastViewMode;
		EViewMode m_ViewMode;

		Matrix4 m_ProjectionMatrix;
		Matrix4 m_ViewMatrix;
		Matrix4 m_ProjectionViewMatrix;

		float m_zNear;
		float m_zFar;
		float m_FOV;
		Frustum m_Frustum;

		int m_Width;
		int m_Height;

		bool m_NeedsUpdateMatrix;

		bool m_IsFrustumUpdateEnabled;
	public:
		CameraComponent(uint cameraID, int width, int height);
		explicit CameraComponent(uint cameraID, const Vector2i& size);

		void Resize(int width, int height);
		void Resize(const Vector2i& size);

		void Tick(double delta) override;

		[[nodiscard]] Matrix4 GetProjectionMatrix() const;
		[[nodiscard]] Matrix4 GetViewMatrix();
		[[nodiscard]] Matrix4 GetProjectionViewMatrix() const;

		void UpdateProjectionMatrix();

		void SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
		void SetOrtho(const Vector3& center, const Vector3& extent);

		[[nodiscard]] Vector3D GetWorldPosition(double x, double y, double projectionZPos) const;
		bool GetScreenCoordinates(const Vector3D& position, Vector2D& out_ScreenPos) const;

		[[nodiscard]] Ray GetRay(double x, double y) const;
		[[nodiscard]] Ray GetRay(int x, int y) const;
		[[nodiscard]] Ray GetRay(const Vector2i& screenPos) const;

		[[nodiscard]] Vector2i GetSize() const;

		const Matrix4& GetTransformMatrix() override;

		[[nodiscard]] Vector3D GetForwardVector() override;
		[[nodiscard]] Vector3D GetUpVector() override;
		[[nodiscard]] Vector3D GetLeftVector() override;

		inline const Frustum& GetFrustum() { return m_Frustum; }

		inline float GetFov() const { return m_FOV; }
		inline void SetFov(float fov) { m_FOV = fov; }

		inline void SetZNear(float zNear) { m_zNear = zNear; }
		inline void SetZFar(float zFar) { m_zFar = zFar; }

		[[nodiscard]] inline float GetZNear() const { return m_zNear; }
		[[nodiscard]] inline float GetZFar() const { return m_zFar; }

		inline void SetFrustumUpdateEnabled(bool enabled) { m_IsFrustumUpdateEnabled = enabled; }
		[[nodiscard]] inline bool IsFrustumUpdateEnabled() const { return m_IsFrustumUpdateEnabled; }

		inline explicit operator uint() const { return m_CameraID; }
		inline uint ID() const { return m_CameraID; }
	protected:
		inline void MarkTransformUpdate() override
		{
			SceneComponent::MarkTransformUpdate();
			m_NeedsUpdateMatrix = true;
		}
	};
}
