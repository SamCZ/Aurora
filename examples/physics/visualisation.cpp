#include <iostream>
#include <Aurora/Aurora.hpp>
#include <Aurora/Core/Profiler.hpp>
#include <Aurora/Graphics/VgRender.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>
#include <Aurora/Physics/AABB.hpp>

#include <imgui.h>

using namespace Aurora;

class PhysicsVisualContext : public AppContext
{
private:
	AABB m_Bounds0;
	AABB m_Bounds1;

	Vector3 m_RayStart;
	Vector3 m_RayEnd;

	Vector3 m_Velocity;
public:
	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameMode<GameModeBase>();

		m_RayStart = {0, 0, 0};
		m_RayEnd = {0, 0, 0};

		m_Velocity = {0, 0, 0};

		m_Bounds0 = AABB::FromExtent({500, 0, 500}, Vector3(50.0f));
		m_Bounds1 = AABB::FromExtent({100, 0, 100}, Vector3(50.0f));
	}

	void RenderAABB(const AABB& aabb)
	{
		auto* vg = GEngine->GetVgRender();
		Vector3 min = aabb.GetMin();
		Vector3 max = aabb.GetMax();
		Vector3 origin = aabb.GetOrigin();

		//vg->DrawString(glm::to_string(min), {5, 5}, Color::white(), 13);
		//vg->DrawString(glm::to_string(max), {5, 19}, Color::white(), 13);

		vg->DrawRect(min.x, min.z, max.x - min.x, max.z - min.z, Color::red(), true, 1.0f);

		vg->DrawRect(origin.x, origin.z, 1.0f, 1.0f, Color::red(), false, 0.0f);
		vg->DrawOval(origin.x, origin.z, 10, 10, Color::red(), true, 1.0f);
	}

	static bool RayAABB(const Vector3& rayOrigin, const Vector3& rayDirection, const AABB& aabb, Vector3& contactPoint, Vector3& contactNormal, float& t_hit_near)
	{
		Vector3 invDir = 1.0f / rayDirection;

		Vector3 t_near = (aabb.GetMin() - rayOrigin) * invDir;
		Vector3 t_far = (aabb.GetMax() - rayOrigin) * invDir;

		if (glm::any(glm::isnan(t_near))) return false;
		if (glm::any(glm::isnan(t_far))) return false;

		// Sort distances
		if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
		if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
		if (t_near.z > t_far.z) std::swap(t_near.z, t_far.z);

		// Early rejection, TODO: Z axis
		if (t_near.x > t_far.y || t_near.y > t_far.x) return false;
		if (t_near.x > t_far.z || t_near.z > t_far.x) return false;

		// Closest 'time' will be the first contact
		t_hit_near = std::max(t_near.x, std::max(t_near.y, t_near.z));

		// Furthest 'time' is contact on opposite side of target
		float t_hit_far = std::min(t_far.x, std::min(t_far.y, t_far.z));

		// Reject if ray direction is pointing away from object
		if (t_hit_far < 0)
			return false;

		// Contact point of collision from parametric line equation
		contactPoint = rayOrigin + t_hit_near * rayDirection;

		if (t_near.x > t_near.y && t_near.x > t_near.z)
		{
			if (invDir.x < 0)
				contactNormal = { 1, 0, 0 };
			else
				contactNormal = { -1, 0, 0 };
		}
		else if (t_near.x < t_near.y && t_near.x < t_near.z)
		{
			if (invDir.y < 0)
				contactNormal = { 0, 1, 0 };
			else
				contactNormal = { 0, -1, 0 };
		} else
		{
			if (invDir.z < 0)
				contactNormal = { 0, 0, 1 };
			else
				contactNormal = { 0, 0, -1 };
		}

		return true;
	}

	bool AABBVsAABB(const AABB& firstAABB, const AABB& secondAABB, const Vector3& velocity, Vector3& contactPoint, Vector3& contactNormal, float& t_hit_near)
	{
		if (glm::length2(velocity) == 0)
			return false;

		AABB expandedAABB(secondAABB.GetMin() - firstAABB.GetSize() / 2.0f, secondAABB.GetMax() + firstAABB.GetSize() / 2.0f);

		RenderAABB(expandedAABB);

		Vector3 origin = firstAABB.GetOrigin();
		bool hit = RayAABB(origin, velocity, expandedAABB, contactPoint, contactNormal, t_hit_near) && t_hit_near >= 0.0f && t_hit_near < 1.0f;

		if (hit)
		{
			auto* vg = GEngine->GetVgRender();
			vg->DrawLine(origin.x, origin.z, contactPoint.x, contactPoint.z, 1.0f, Color::green());
			return true;
		}

		return false;
	}

	float g_Delta = 0;

	void Update(double delta) override
	{
		g_Delta = float(delta);
	}

	void RenderVg() override
	{
		auto* vg = GEngine->GetVgRender();

		auto cursorPos = GEngine->GetInputManager()->CursorPosition_Pixels().value();

		//m_Velocity = {0, 0, 0};

		if (ImGui::GetIO().MouseDown[0])
		{
			//m_Bounds1 = AABB::FromExtent({0, 0, 0}, Vector3(50.0f));

			m_Velocity += glm::normalize(Vector3(cursorPos.x, 0, cursorPos.y) - m_Bounds1.GetOrigin()) * 100.0f * g_Delta;
		}

		m_RayEnd = {cursorPos.x, 0, cursorPos.y};
		Vector3 rayDir = m_RayEnd - m_RayStart;

		//rayDir = m_Velocity;

		vg->DrawLine(m_RayStart.x, m_RayStart.z, m_RayEnd.x, m_RayEnd.z, 1.0f, Color::green());

		Vector3 contactPoint, contactNormal;
		float t_hit_near = 0.0f;
		if (RayAABB(m_RayStart, rayDir, m_Bounds0, contactPoint, contactNormal, t_hit_near) && t_hit_near >= 0.0f && t_hit_near < 1.0f)
		{
			contactNormal = m_Bounds0.GetRayHitNormal(contactPoint);

			vg->DrawOval(contactPoint.x, contactPoint.z, 10, 10, Color::blue(), false, 0.0f);
			vg->DrawLine(contactPoint.x, contactPoint.z, contactPoint.x + contactNormal.x * 30, contactPoint.z + contactNormal.z * 30, 1.0f, Color(20, 100, 200));
		}

		/*Vector3 b1Origin = m_Bounds1.GetOrigin();
		vg->DrawLine(b1Origin.x, b1Origin.y, b1Origin.x + m_Velocity.x, b1Origin.z + m_Velocity.z, 1.0f, Color::green());

		if (AABBVsAABB(m_Bounds1, m_Bounds0, m_Velocity * g_Delta, contactPoint, contactNormal, t_hit_near))
		{
			vg->DrawString(std::to_string(t_hit_near), {5, 5}, Color::white(), 13);

			m_Velocity += contactNormal * glm::abs(m_Velocity) * (1.0f - t_hit_near);
		}

		m_Bounds1.SetOffset(m_Velocity * g_Delta);*/

		RenderAABB(m_Bounds0);
		RenderAABB(m_Bounds1);

		//std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
};

int main(int argc, char *argv[])
{

	Aurora::AuroraEngine engine;

	WindowDefinition windowDefinition = {};
	windowDefinition.Width = 1270;
	windowDefinition.Height = 800;
	windowDefinition.HasOSWindowBorder = true;
	windowDefinition.Maximized = false;
	windowDefinition.Title = "Physics Visualization helper";

	engine.Init(new PhysicsVisualContext(), windowDefinition, false);
	engine.Run();

	return 0;
}