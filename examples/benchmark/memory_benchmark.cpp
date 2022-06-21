#include <iostream>

#include <string>
#include <vector>
#include <unordered_map>

#include <chrono>

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <Aurora/Memory/Aum.hpp>
#include <Aurora/Core/Object.hpp>
#include <Aurora/Framework/ComponentStorage.hpp>
#include <Aurora/Framework/ActorComponent.hpp>
using namespace Aurora;

//#define CLASSIC_IMPL

#define COUNT_ENTITY    200
#define COUNT_COMPONENT 1000

#define COUNT_TICKS 1000

class ScopedTimer {
public:
	ScopedTimer(const std::string& name){
		m_name = name;
		m_begin = std::chrono::steady_clock::now();
	}
	virtual ~ScopedTimer(){
		auto end = std::chrono::steady_clock::now();

		auto count =std::chrono::duration_cast<std::chrono::milliseconds>(end - m_begin).count();
		std::cout << "[" << m_name << "] Elapsed: " << count << "ms\n";
	}
protected:
	std::string m_name;
	std::chrono::steady_clock::time_point m_begin;
};

/*===========================================================================//
//===========================================================================//
//===========================================================================*/

#ifndef CLASSIC_IMPL
class Memory
{
public:

	template<class T>
	static void New()
	{

	}

	static void Delete(void* ptr)
	{

	}
};
#else // !CLASSIC_IMPL

namespace Memory{
    template <typename T>
    T* New(){
        return new T();
    }

    template <typename T>
    void Delete(T* ptr){
        delete ptr;
    }
};

#endif  // !CLASSIC_IMPL






class Component : public ActorComponent {
public:
	CLASS_OBJ(Component, ActorComponent);
	Component()= default;
	~Component() override{}

	virtual void Update(){}
};

class BadassComponent : public Component{
public:
	CLASS_OBJ(BadassComponent, Component);
	BadassComponent()= default;
	~BadassComponent() override{}

	void Update() override{
		x += 1 + rand() % 100;
	}

	int x = 0;
};

class Entity : public ObjectBase {
public:
	static ComponentStorage m_ComponentStorage;

	CLASS_OBJ(Entity, ObjectBase);
	Entity()= default;
	virtual ~Entity(){
		for (auto c: components){
			m_ComponentStorage.DestroyComponent(c);
		}
	}

	void Update(){
		for (auto c : components){
			c->Update();
		}
	}

	template <typename T>
	T* Add(){
		auto out = m_ComponentStorage.CreateComponent<T>("Obj");
		components.push_back(out);
		return out;
	}

	template <typename T>
	T* Get(){
		for (auto c : components){
			auto casted = dynamic_cast<T*>(c);
			if (casted){
				return casted;
			}
		}
		return nullptr;
	}

	std::vector<Component*> components;
};

ComponentStorage Entity::m_ComponentStorage;


int main(int argc, char** argv){
#ifdef CLASSIC_IMPL
	std::cout << "Classic implementation...\n";
#else
	std::cout << "New implementation...\n";
#endif


	std::vector<Entity*> entities;

	Aum entityMemory;

	{
		ScopedTimer timer("Memory Allocation");

		for (int i=0; i<COUNT_ENTITY; i++){
			auto ent = entityMemory.AllocAndInit<Entity>();
			for (int j=0; j<COUNT_COMPONENT; j++){
				auto cmp = ent->Add<BadassComponent>();
			}
			entities.push_back(ent);
		}
	}

	{
		ScopedTimer timer("Ticks");

		for (size_t i=0; i<COUNT_TICKS; i++){
			for (auto ent : entities){
				ent->Update();
			}
		}
	}

	{
		ScopedTimer timer("Memory Deallocation");

		for (auto ent : entities){
			entityMemory.DeAllocAndUnload<Entity>(ent);
		}
		entities.clear();
	}

	return 0;
}