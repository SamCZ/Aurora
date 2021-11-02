#include "Scene.hpp"

#include <tuple>
#include "Entity.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Physics/PhysicsWorld.hpp"

namespace Aurora
{
	Scene::Scene()
	{
		m_Registry.on_construct<BodyComponent>().connect<&Scene::OnBodyComponentAdded>(this);
		m_Registry.on_destroy<BodyComponent>().connect<&Scene::OnBodyComponentDestroyed>(this);
	}

	Scene::~Scene()
	{
		m_Registry.clear();
	}

	void Scene::OnBodyComponentAdded(entt::registry &registry, entt::entity entity)
	{
		const BodyComponent& bodyComponent = registry.get<BodyComponent>(entity);

		if(bodyComponent.Body == nullptr)
		{
			AU_LOG_ERROR("Body pointer is null !");
			return;
		}

		//GetEngine()->GetPhysicsWorld()->AddBody(bodyComponent.Body);
	}

	void Scene::OnBodyComponentDestroyed(entt::registry &registry, entt::entity entity)
	{
		const BodyComponent& bodyComponent = registry.get<BodyComponent>(entity);

		if(bodyComponent.Body == nullptr)
		{
			AU_LOG_ERROR("Body pointer is null !");
			return;
		}

		//GetEngine()->GetPhysicsWorld()->DeleteBody(bodyComponent.Body);
	}

	void Scene::SetEntityCollider(Entity entity, const BaseColliderComponent &colliderComponent)
	{
		BodyComponent* bodyComponent = nullptr;

		if(!entity.HasComponent<BodyComponent>())
		{
			bodyComponent = &entity.AddComponent<BodyComponent>();
			//bodyComponent->Body = new ndBodyDynamic();
		}
		else
		{
			bodyComponent = &entity.GetComponent<BodyComponent>();
		}


	}

	void Scene::Tick(double delta)
	{
		for(uint i = m_Actors.size(); i --> 0;)
		{
			Actor* actor = m_Actors[i];
			actor->Tick(delta);
		}

		{
			auto view = m_Registry.view<TransformComponent>();
			for (entt::entity entity : view)
			{
				TransformComponent& transformComponent = view.get<TransformComponent>(entity);
				Entity e = Entity(entity, this);
				glm::mat4 transform = GetTransformRelativeToParent(e);
				glm::vec3 translation;
				glm::vec3 rotation;
				glm::vec3 scale;
				DecomposeTransform(transform, translation, rotation, scale);

				auto rotationQuat = Quaternion(rotation);
				transformComponent.Up = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 1.0f, 0.0f)));
				transformComponent.Right = glm::normalize(glm::rotate(rotationQuat, glm::vec3(1.0f, 0.0f, 0.0f)));
				transformComponent.Forward = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 0.0f, -1.0f)));
			}
		}
	}

	Matrix4 Scene::GetTransformRelativeToParent(Entity entity)
	{
		return entity.GetModelMatrix();
	}

	Entity Scene::CreateEntity(const std::string &name)
	{
		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<UUID64>();
		idComponent = UUID64::Generate();

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		return entity;
	}

	void Scene::DestroyActor(Actor *actor)
	{
		if(actor->m_EntityHandle != entt::null)
		{
			m_Registry.release(actor->m_EntityHandle);
		}

		m_ActorMemory.DeAllocAndUnload<Actor>(actor);
		m_Actors.erase(std::find(m_Actors.begin(), m_Actors.end(), actor));
	}
}