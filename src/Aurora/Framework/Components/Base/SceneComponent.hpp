#pragma once

#include "ActorComponent.hpp"

#include "Aurora/Physics/PhysicsBody.hpp"

namespace Aurora
{
	class SceneComponent : public ActorComponent
	{
	public:
		friend class Actor;
	protected:
		Vector3D m_Location;
		Vector3D m_Rotation;
		Vector3D m_Scale;
	protected:
		Vector3D m_Acceleration;
		Vector3D m_Velocity;
		bool m_IsSimulatingPhysics;
	private:
		SceneComponent* m_Parent;
		std::vector<SceneComponent*> m_Components;
	private:
		Matrix4 m_LastMatrix;
		bool m_NeedsUpdateMatrix;
	protected:
		PhysicsBody m_Body;
	public:
		inline SceneComponent()
		: ActorComponent(),
		m_Location(),
		m_Rotation(),
		m_Scale(1, 1, 1),
		m_Parent(nullptr),
		m_Components(),
		m_LastMatrix(),
		m_NeedsUpdateMatrix(true),
		m_Acceleration(),
		m_Velocity(),
		m_IsSimulatingPhysics(false),
		m_Body() {}

		~SceneComponent() override = default;
	public:
		bool AttachToComponent(SceneComponent* InParent);
		void DetachFromComponent();

		const Matrix4& GetTransformMatrix() override;
		virtual Vector3D GetForwardVector();
		virtual Vector3D GetUpVector();
		virtual Vector3D GetLeftVector();
	public:
		inline bool IsRootComponent() const
		{
			return m_Parent == nullptr;
		}

		inline SceneComponent* GetParent()
		{
			return m_Parent;
		}

		[[nodiscard]] inline const Vector3D& GetLocation() { return m_Location; }
		[[nodiscard]] inline const Vector3D& GetRotation() { return m_Rotation; }
		[[nodiscard]] inline const Vector3D& GetScale() { return m_Scale; }

		inline void SetLocation(const Vector3D& location) { m_Location = location; MarkTransformUpdate(); }
		inline void SetRotation(const Vector3D& rotation) { m_Rotation = rotation; MarkTransformUpdate(); }
		inline void SetScale(const Vector3D& scale) { m_Scale = scale; MarkTransformUpdate(); }

		inline void SetLocation(double x, double y, double z) { m_Location.x = x, m_Location.y = y, m_Location.z = z; MarkTransformUpdate(); }
		inline void SetRotation(double x, double y, double z) { m_Rotation.x = x, m_Rotation.y = y, m_Rotation.z = z; MarkTransformUpdate(); }
		inline void SetScale(double x, double y, double z) { m_Scale.x = x, m_Scale.y = y, m_Scale.z = z; MarkTransformUpdate(); }

		inline void AddLocation(const Vector3D& location) { m_Location += location; MarkTransformUpdate(); }
		inline void AddRotation(const Vector3D& rotation) { m_Rotation += rotation; MarkTransformUpdate(); }
		inline void AddScale(const Vector3D& scale) { m_Scale += scale; MarkTransformUpdate(); }

		inline void AddLocation(double x, double y, double z) { m_Location.x += x, m_Location.y += y, m_Location.z += z; MarkTransformUpdate(); }
		inline void AddRotation(double x, double y, double z) { m_Rotation.x += x, m_Rotation.y += y, m_Rotation.z += z; MarkTransformUpdate(); }
		inline void AddScale(double x, double y, double z) { m_Scale.x += x, m_Scale.y += y, m_Scale.z += z; MarkTransformUpdate(); }
	public:
		inline void SetSimulatePhysics(bool enabled) { m_IsSimulatingPhysics = enabled; }
		inline bool IsSimulatingPhysics() const { return m_IsSimulatingPhysics; }

		inline void SetAcceleration(const Vector3D& acceleration) { m_Acceleration = acceleration; }
		inline void SetVelocity(const Vector3D& velocity) { m_Velocity = velocity; }

		inline void AddAcceleration(const Vector3D& acceleration) { m_Acceleration += acceleration; }
		inline void AddVelocity(const Vector3D& velocity) { m_Velocity += velocity; }

		inline const Vector3D& GetAcceleration() { return m_Acceleration; }
		inline const Vector3D& GetVelocity() { return m_Velocity; }
	protected:
		virtual void MarkTransformUpdate();
	protected:
		static Vector3D GetRotationColumn(const Matrix4& mat, int i);
	public:
		PhysicsBody& GetBody() { return m_Body; };
	};
}