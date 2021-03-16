#include "CameraComponent.hpp"

namespace Aurora
{
	CameraComponent::CameraComponent(int width, int height)
			: SceneComponent(), m_Width(width), m_Height(height), m_LastViewMode(EViewMode::Orthographic),
			  m_ViewMode(EViewMode::Perspective),
			  m_zNear(0.1f), m_zFar(2000.0f), m_FOV(75.0f), m_Frustum(glm::identity<Matrix4>()),
			  m_ProjectionMatrix(),
			  m_ViewMatrix(),
			  m_ProjectionViewMatrix(),
			  m_IsFrustumUpdateEnabled(true) { }

	CameraComponent::CameraComponent(const Vector2i &size) : CameraComponent(size.x, size.y)
	{

	}

	void CameraComponent::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		UpdateProjectionMatrix();
	}

	void CameraComponent::Resize(const Vector2i &size)
	{
		m_Width = size.x;
		m_Height = size.y;
		UpdateProjectionMatrix();
	}

	void CameraComponent::Tick(double delta)
	{
		if (m_LastViewMode != m_ViewMode)
		{
			m_LastViewMode = m_ViewMode;
			UpdateProjectionMatrix();
		}

		//UpdateProjectionMatrix();

		m_ViewMatrix = GetViewMatrix();
		m_ProjectionViewMatrix = GetProjectionMatrix() * GetViewMatrix();

		if(m_IsFrustumUpdateEnabled)
			m_Frustum = Frustum(m_ProjectionViewMatrix);
	}

	Matrix4 CameraComponent::GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	Matrix4 CameraComponent::GetViewMatrix()
	{
		if(!m_NeedsUpdateMatrix) {
			return m_ViewMatrix;
		}

		m_NeedsUpdateMatrix = false;

		Vector3 location = GetLocation();
		Vector3 rotation = GetRotation();

		SceneComponent* parent = GetParent();
		if(parent != nullptr) {
			location += parent->GetLocation();
			rotation += parent->GetRotation();
		}

		// define your up vector
		glm::vec3 upVector = glm::vec3(0, 1, 0);
		// rotate around to a given bearing: yaw
		glm::mat4 camera = glm::rotate(glm::identity<glm::mat4>(), glm::radians(static_cast<float>(rotation.y)), upVector);
		// Define the 'look up' axis, should be orthogonal to the up axis
		glm::vec3 pitchVector = glm::vec3(1, 0, 0);
		// rotate around to the required head tilt: pitch
		camera = glm::rotate(camera, glm::radians(static_cast<float>(rotation.x)), pitchVector);

		glm::vec3 rollVector = glm::vec3(0, 0, 1);
		camera = glm::rotate(camera, glm::radians(static_cast<float>(rotation.z)), rollVector);

		// now get the view matrix by taking the camera inverse
		m_ViewMatrix = glm::inverse(camera) * glm::translate(glm::vec3(-location.x, -location.y, -location.z));

		return m_ViewMatrix;
	}

	Matrix4 CameraComponent::GetProjectionViewMatrix() const
	{
		return m_ProjectionViewMatrix;
	}

	void CameraComponent::UpdateProjectionMatrix()
	{
		if (m_ViewMode == EViewMode::Perspective)
		{
			float aspect = (float)m_Width / (float)m_Height;
			/*float h = glm::tan(glm::radians(m_FOV) * .5f) * m_zNear;
			float w = h * aspect;
			float frustumLeft = -w;
			float frustumRight = w;
			float frustumBottom = -h;
			float frustumTop = h;
			float frustumNear = m_zNear;
			float frustumFar = m_zFar;

			m_ProjectionMatrix = glm::frustum(frustumLeft, frustumRight, frustumBottom, frustumTop, frustumNear, frustumFar);*/
			m_ProjectionMatrix = glm::perspective(
					glm::radians(m_FOV), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
					aspect,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
					m_zNear,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
					m_zFar             // Far clipping plane. Keep as little as possible.
			);
		}
		else if(m_ViewMode == EViewMode::Orthographic)
		{
			m_ProjectionMatrix = glm::ortho(0.0f, (float)m_Width, (float)m_Height, 0.0f, -1.0f, 1.0f);
		}
	}

	void CameraComponent::SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		m_ProjectionMatrix = glm::ortho<float>(left, right, bottom, top, zNear, zFar);
		m_ViewMode = EViewMode::Custom;
	}

	void CameraComponent::SetOrtho(const Vector3& center, const Vector3& extent)
	{
		SetOrtho(center.x - extent.x, center.x + extent.x, center.y - extent.y, center.y + extent.y, center.z - extent.z, center.z + extent.z);
	}

	Vector3D CameraComponent::GetWorldPosition(double x, double y, double projectionZPos) const
	{
		Matrix4 inverseMat = glm::inverse(GetProjectionViewMatrix());

		Vector3D store = Vector3((2.0 * x) / (float)m_Width - 1.0, (2.0 * y) / (float)m_Height - 1.0, projectionZPos * 2 - 1);
		Vector4 proStore = inverseMat * Vector4(store, 1.0);
		store.x = proStore.x;
		store.y = proStore.y;
		store.z = proStore.z;
		store *= 1.0f / proStore.w;
		return store;
	}

	bool CameraComponent::GetScreenCoordinates(const Vector3D& position, Vector2D& out_ScreenPos) const
	{
		Vector4D result = (GetProjectionViewMatrix() * Vector4D(position, 1.0));

		/*Vector3D store = Vector3D(proj.x, proj.y, proj.z);
		store = store / proj.w;

		int viewPortRight = 1;
		int viewPortLeft = 0;
		int viewPortTop = 0;
		int viewPortBottom = 1;

		store.x = ((store.x + 1.0) * (viewPortRight - viewPortLeft) / 2.0 + viewPortLeft) * RenderPass->GetWidth();
		store.y = ((store.y + 1.0) * (viewPortTop - viewPortBottom) / 2.0 + viewPortBottom) * RenderPass->GetHeight();

		if(store.x < 0 || store.x > GetSize().x) {
            store.z += 2;
		}

        if(store.y < 0 || store.y > GetSize().y) {
            store.z += 2;
        }*/

		if(result.w > 0.0f) {
			// the result of this will be x and y coords in -1..1 projection space
			const double RHW = 1.0 / result.w;
			Vector4D PosInScreenSpace = Vector4D(result.x * RHW, result.y * RHW, result.z * RHW, result.w);

			// Move from projection space to normalized 0..1 UI space
			const double NormalizedX = ( PosInScreenSpace.x / 2.f ) + 0.5f;
			const double NormalizedY = 1.f - ( PosInScreenSpace.y / 2.f ) - 0.5f;

			out_ScreenPos.x = NormalizedX * (double)GetSize().x;
			out_ScreenPos.y = NormalizedY * (double)GetSize().y;

			if(out_ScreenPos.x < 0 || out_ScreenPos.x > GetSize().x) {
				return false;
			}

			if(out_ScreenPos.y < 0 || out_ScreenPos.y > GetSize().y) {
				return false;
			}

			return true;
		}

		return false;
	}

	Ray CameraComponent::GetRay(double x, double y) const
	{
		Vector3D click3d = GetWorldPosition(x, y, 0);
		Vector3D dir = glm::normalize(GetWorldPosition(x, y, 1) - click3d);
		return Ray(click3d, dir);
	}

	Ray CameraComponent::GetRay(int x, int y) const
	{
		return GetRay(static_cast<double>(x), static_cast<double>(y));
	}

	Ray CameraComponent::GetRay(const Vector2i& screenPos) const
	{
		return GetRay(screenPos.x, screenPos.y);
	}

	Vector2i CameraComponent::GetSize() const
	{
		return {m_Width, m_Height};
	}

	Vector3D GetRotationColumn(const Matrix4 & mat, int i)
	{
		Matrix4 trns = glm::transpose(mat);

		Vector3D store;

		store.x = trns[i][0];
		store.y = trns[i][1];
		store.z = trns[i][2];
		return store;
	}

	const Matrix4 &CameraComponent::GetTransformMatrix()
	{
		return SceneComponent::GetTransformMatrix();
	}

	Vector3D CameraComponent::GetForwardVector()
	{
		return GetRotationColumn(GetViewMatrix() , 2);
	}

	Vector3D CameraComponent::GetUpVector()
	{
		return GetRotationColumn(GetViewMatrix(), 1);
	}

	Vector3D CameraComponent::GetLeftVector()
	{
		return GetRotationColumn(GetViewMatrix(), 0);
	}
}