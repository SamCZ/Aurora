#include "SceneComponent.hpp"

namespace Aurora
{

	bool SceneComponent::AttachToComponent(SceneComponent* InParent)
	{
		if (m_Parent == InParent)
		{
			return false;
		}

		DetachFromComponent();

		m_Parent = InParent;
		m_Owner = InParent->m_Owner;
		m_Scene = InParent->m_Scene;

		InParent->m_Components.push_back(this);
		return true;
	}

	void SceneComponent::DetachFromComponent()
	{
		if (m_Parent)
		{
			m_Parent->m_Components.erase(std::find(m_Parent->m_Components.begin(), m_Parent->m_Components.end(), this));
			m_Owner = nullptr;
			m_Parent = nullptr;
		}
	}

	const Matrix4& SceneComponent::GetTransformMatrix()
	{
		static Vector3 axisX = Vector3(1, 0, 0);
		static Vector3 axisY = Vector3(0, 1, 0);
		static Vector3 axisZ = Vector3(0, 0, 1);

		if(m_NeedsUpdateMatrix) {
			m_NeedsUpdateMatrix = false;
			auto rotation = glm::identity<Matrix4>();

			rotation *= glm::rotate(glm::radians(static_cast<float>(m_Rotation.z)), axisZ);
			rotation *= glm::rotate(glm::radians(static_cast<float>(m_Rotation.y)), axisY);
			rotation *= glm::rotate(glm::radians(static_cast<float>(m_Rotation.x)), axisX);

			m_LastMatrix = (glm::translate((Vector3)m_Location) * rotation) * glm::scale((Vector3)m_Scale);

			if (m_Parent != nullptr)
			{
				const Matrix4& parentTransform = m_Parent->GetTransformMatrix();
				m_LastMatrix = parentTransform * m_LastMatrix;
			}

			m_Body.Transform(m_LastMatrix);
		}

		return m_LastMatrix;
	}

	void SceneComponent::MarkTransformUpdate()
	{
		m_NeedsUpdateMatrix = true;

		for(auto* child : m_Components) {
			child->MarkTransformUpdate();
		}
	}

	Vector3D SceneComponent::GetForwardVector()
	{
		return GetRotationColumn(GetTransformMatrix() , 2);
	}

	Vector3D SceneComponent::GetUpVector()
	{
		return GetRotationColumn(GetTransformMatrix(), 1);
	}

	Vector3D SceneComponent::GetLeftVector()
	{
		return GetRotationColumn(GetTransformMatrix(), 0);
	}

	Vector3D SceneComponent::GetRotationColumn(const Matrix4 &mat, int i)
	{
		Matrix4 trns = glm::transpose(mat);

		Vector3D store;

		store.x = trns[i][0];
		store.y = trns[i][1];
		store.z = trns[i][2];
		return store;
	}
}