#include "Scene.hpp"
#include "Aurora/Core/Common.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Physics/BulletDebugDraw.hpp"

namespace Aurora
{
	struct btLayerOverlapFilterCallback : public btOverlapFilterCallback
	{
		bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override
		{
			btCollisionObject* obj0 = static_cast<btCollisionObject*>(proxy0->m_clientObject);
			btCollisionObject* obj1 = static_cast<btCollisionObject*>(proxy1->m_clientObject);

			Actor* actor0 = static_cast<Actor*>(obj0->getUserPointer());
			Actor* actor1 = static_cast<Actor*>(obj1->getUserPointer());

			return CollisionMatrix::CanCollide(actor0->CollisionLayer, actor1->CollisionLayer);
		}
	};

	Scene::Scene() :
		m_ActorMemory(),
		m_CollisionConfiguration(new btDefaultCollisionConfiguration()),
		m_Dispatcher(new btCollisionDispatcher(m_CollisionConfiguration)),
		m_Broadphase(new btDbvtBroadphase()),
		m_Solver(new btSequentialImpulseConstraintSolver()),
		m_DynamicsWorld(new btDiscreteDynamicsWorld(m_Dispatcher, m_Broadphase, m_Solver, m_CollisionConfiguration)),
		m_OverlapCallback(new btLayerOverlapFilterCallback()),
		m_DebugDraw(new BulletDebugDraw())
	{
		m_DynamicsWorld->setDebugDrawer(m_DebugDraw);
		m_DynamicsWorld->getPairCache()->setOverlapFilterCallback(m_OverlapCallback);
		m_DynamicsWorld->setGravity(btVector3(0, -10, 0));
	}

	Scene::~Scene()
	{
		// Destroy actors
		std::vector<Actor*> actors = m_Actors;
		for(Actor* actor : actors)
		{
			if (not m_ActorMemory.CheckMemory(actor))
			{
				//__debugbreak();
				AU_LOG_FATAL("Memory corrupted!");
			}

			DestroyActor(actor);
		}

		// Destroy physics
		if (m_DynamicsWorld)
		{
			int i;
			for (i = m_DynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
			{
				m_DynamicsWorld->removeConstraint(m_DynamicsWorld->getConstraint(i));
			}
			for (i = m_DynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
			{
				btCollisionObject* obj = m_DynamicsWorld->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				/*if (body && body->getMotionState())
				{
					delete body->getMotionState();
				}*/
				m_DynamicsWorld->removeCollisionObject(obj);
			}
		}

		delete m_OverlapCallback;
		delete m_DebugDraw;
		delete m_DynamicsWorld;
		delete m_Solver;
		delete m_Broadphase;
		delete m_Dispatcher;
		delete m_CollisionConfiguration;
	}

	void Scene::FinishSpawningActor(Actor* actor)
	{
		if(!actor) {
			return;
		}

		m_Actors.push_back(actor);

		actor->SetActive(true);
		actor->BeginPlay();
	}

	void Scene::DestroyActor(Actor* actor)
	{
		if(!actor)
		{
			return;
		}

		for (size_t i = actor->m_Components.size(); i --> 0;)
		{
			actor->DestroyComponent(actor->m_Components[i]);
		}

		actor->BeginDestroy();

		VectorRemove<Actor*>(m_Actors, actor);

		m_ActorMemory.DeAllocAndUnload<Actor>(actor);
	}

	void Scene::Update(double delta)
	{
		// Iterate from end to enable destroy while updating
		for (size_t i = m_Actors.size(); i --> 0;)
		{
			m_Actors[i]->Tick(delta);
		}

		for(ActorComponent* actorComponent : GetComponents<ActorComponent>())
		{
			actorComponent->Tick(delta);
		}

		int maxSimSubSteps = 2;
		m_DynamicsWorld->stepSimulation((float)delta, maxSimSubSteps);
		m_DynamicsWorld->debugDrawWorld();
	}

	void Scene::RegisterComponent(SceneComponent* component)
	{
		if (RigidBodyComponent* rigidBodyComponent = RigidBodyComponent::SafeCast(component))
		{
			if (rigidBodyComponent->GetBody())
			{
				m_DynamicsWorld->addRigidBody(rigidBodyComponent->GetBody());
			}
		}
	}

	void Scene::UnRegisterComponent(SceneComponent* component)
	{
		if (RigidBodyComponent* rigidBodyComponent = RigidBodyComponent::SafeCast(component))
		{
			if (rigidBodyComponent->GetBody())
			{
				m_DynamicsWorld->removeRigidBody(rigidBodyComponent->GetBody());
			}
		}
	}
}