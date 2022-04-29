#pragma once

#include "Aurora/Core/Vector.hpp"
#include <btBulletDynamicsCommon.h>

namespace Aurora
{
	ATTRIBUTE_ALIGNED16(class) AU_API BulletDebugDraw : public btIDebugDraw
	{
	private:
		int m_DebugMode;
		DefaultColors m_ourColors;
	public:
		BT_DECLARE_ALIGNED_ALLOCATOR();

		BulletDebugDraw() : m_DebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb) {}
		~BulletDebugDraw() override = default;

		void setDebugMode(int debugMode) override;

		[[nodiscard]] int getDebugMode() const override;

		[[nodiscard]] DefaultColors getDefaultColors() const override { return m_ourColors; }
		void setDefaultColors(const DefaultColors& colors) override { m_ourColors = colors; }

		void drawLine(const btVector3& from1, const btVector3& to1, const btVector3& color1) override;
		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

		void reportErrorWarning(const char* warningString) override;
		void draw3dText(const btVector3& location, const char* textString) override;

		void flushLines() override {}
	};
}
