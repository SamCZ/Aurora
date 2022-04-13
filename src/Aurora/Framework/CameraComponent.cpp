#include "CameraComponent.hpp"

#include "Aurora/Core/assert.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"

namespace Aurora
{
	CameraComponent::CameraComponent()
		: m_SelectedPass(Pass::Ambient),
		m_ViewPort(nullptr),
		m_ProjectionType(ProjectionType::None),
		m_PerspectiveSettings(),
		m_OrthogonalSettings(),
		m_ClearColor(0, 0, 0, 255)
	{

	}

	CameraComponent::~CameraComponent() = default;

	void CameraComponent::SetPerspective(float fov, float near, float far)
	{
		au_assert(m_ViewPort != nullptr);

		m_ProjectionType = ProjectionType::Perspective;
		float aspect = (float)m_ViewPort->GetWidth() / (float)m_ViewPort->GetHeight();

		m_PerspectiveSettings.Aspect = aspect;
		m_PerspectiveSettings.Fov = fov;
		m_PerspectiveSettings.Near = near;
		m_PerspectiveSettings.Far = far;

		m_Projection = glm::perspective(glm::radians(fov), aspect, near, far);
	}

	void CameraComponent::SetOrthographic(float left, float right, float bottom, float top, float near, float far)
	{
		m_ProjectionType = ProjectionType::Orthogonal;
		m_OrthogonalSettings.Left = left;
		m_OrthogonalSettings.Right = right;
		m_OrthogonalSettings.Bottom = bottom;
		m_OrthogonalSettings.Top = top;
		m_OrthogonalSettings.Near = near;
		m_OrthogonalSettings.Far = far;

		m_Projection = glm::ortho(left, right, bottom, top, near, far);
	}

	void CameraComponent::UpdateFrustum()
	{
		m_Frustum = FFrustum(GetProjectionMatrix() * GetViewMatrix());
	}

	void CameraComponent::SetViewPort(RenderViewPort* wp)
	{
		if(!wp)
			return;

		m_ViewPort = wp;
		m_ViewPortEvent = wp->ResizeEmitter.BindUnique(this, &CameraComponent::Resize);
	}

	void CameraComponent::Resize(const Vector2i&)
	{
		if (m_ProjectionType != ProjectionType::Perspective)
			return;

		AU_LOG_INFO("Camera was resized !");

		SetPerspective(m_PerspectiveSettings.Fov, m_PerspectiveSettings.Near, m_PerspectiveSettings.Far);
	}

	const CameraComponent::PerspectiveSettings &CameraComponent::GetPerspectiveSettings() const
	{
		au_assert(m_ProjectionType == ProjectionType::Perspective);
		return m_PerspectiveSettings;
	}

	const CameraComponent::OrthogonalSettings &CameraComponent::GetOrthogonalSettings() const
	{
		au_assert(m_ProjectionType == ProjectionType::Orthogonal);
		return m_OrthogonalSettings;
	}

	Vector3 CameraComponent::GetWorldPositionFromScreen(float x, float y, float projectionZPos) const
	{
		au_assert(m_ProjectionType == ProjectionType::Perspective);
		au_assert(m_ViewPort != nullptr);

		Matrix4 inverseMat = glm::inverse(GetProjectionViewMatrix());

		Vector3 store = Vector3((2.0 * x) / (float)m_ViewPort->GetWidth() - 1.0, (2.0 * y) / (float)m_ViewPort->GetHeight() - 1.0, projectionZPos * 2 - 1);
		Vector4 proStore = inverseMat * Vector4(store, 1.0);
		store.x = proStore.x;
		store.y = proStore.y;
		store.z = proStore.z;
		store *= 1.0f / proStore.w;
		return store;
	}

	bool CameraComponent::GetScreenCoordinatesFromWorld(const Vector3 &position, Vector2 &out_ScreenPos) const
	{
		au_assert(m_ProjectionType == ProjectionType::Perspective);
		au_assert(m_ViewPort != nullptr);

		Vector4 result = (GetProjectionViewMatrix() * Vector4(position, 1.0f));

		if(result.w > 0.0f)
		{
			// the result of this will be x and y coords in -1..1 projection space
			const float RHW = 1.0f / result.w;
			Vector4 PosInScreenSpace = Vector4(result.x * RHW, result.y * RHW, result.z * RHW, result.w);

			// Move from projection space to normalized 0..1 UI space
			const float NormalizedX = ( PosInScreenSpace.x / 2.f ) + 0.5f;
			const float NormalizedY = 1.f - ( PosInScreenSpace.y / 2.f ) - 0.5f;

			out_ScreenPos.x = NormalizedX * (float)m_ViewPort->GetWidth();
			out_ScreenPos.y = NormalizedY * (float)m_ViewPort->GetHeight();

			if(out_ScreenPos.x < 0 || out_ScreenPos.x > (float)m_ViewPort->GetWidth())
			{
				return false;
			}

			if(out_ScreenPos.y < 0 || out_ScreenPos.y > (float)m_ViewPort->GetHeight())
			{
				return false;
			}

			return true;
		}

		return false;
	}

	void CameraComponent::Tick(double delta)
	{
		// TODO: think about updating only when transform changes too
		m_View = glm::inverse(GetTransformationMatrix());

		UpdateFrustum();
	}
}