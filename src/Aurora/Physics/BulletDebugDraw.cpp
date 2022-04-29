#include "BulletDebugDraw.hpp"

#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Graphics/DShape.hpp"

namespace Aurora
{

	void BulletDebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
	{
		DShapes::Line(btToVec(from), btToVec(to), btToVec(color));
	}

	void BulletDebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
	{
		drawLine(PointOnB, PointOnB + normalOnB * distance, color);
		btVector3 ncolor(0, 0, 0);
		drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
	}

	void BulletDebugDraw::reportErrorWarning(const char *warningString)
	{
		AU_LOG_WARNING(warningString);
	}

	void BulletDebugDraw::draw3dText(const btVector3 &location, const char *textString)
	{
		DShapes::Text(btToVec(location), textString);
	}

	void BulletDebugDraw::setDebugMode(int debugMode)
	{
		m_DebugMode = debugMode;
	}

	int BulletDebugDraw::getDebugMode() const
	{
		return m_DebugMode;
	}
}