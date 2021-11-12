#include "ndContactCallback.hpp"

#include <ndContact.h>
#include <ndShapeInstance.h>
#include "CollisionMatrix.hpp"

namespace Aurora
{
	ndMaterial &ndContactCallback::RegisterMaterial(dUnsigned32 id0, dUnsigned32 id1)
	{
		ndMaterialKey key(id0, id1);
		dTree<ndMaterial, ndMaterialKey>::dNode* node = m_MaterialMap.Find(key);
		if (!node)
		{
			node = m_MaterialMap.Insert(ndMaterial(), key);
		}
		return node->GetInfo();
	}

	ndMaterial ndContactCallback::GetMaterial(const ndContact *const contactJoint, const ndShapeInstance &instance0, const ndShapeInstance &instance1) const
	{
		ndMaterialKey key(instance0.GetMaterial().m_userId, instance1.GetMaterial().m_userId);
		dTree<ndMaterial, ndMaterialKey>::dNode* const node = m_MaterialMap.Find(key);
		return node ? node->GetInfo() : ndMaterial();
	}

	bool ndContactCallback::OnAabbOverlap(const ndContact *const contactJoint, dFloat32 timestep)
	{
		const ndBodyKinematic* const body0 = contactJoint->GetBody0();
		const ndBodyKinematic* const body1 = contactJoint->GetBody1();

		const volatile ndShapeInstance& instanceShape0 = body0->GetCollisionShape();
		const volatile ndShapeInstance& instanceShape1 = body1->GetCollisionShape();

		if(instanceShape0.m_shapeMaterial.m_userId == 0 || instanceShape1.m_shapeMaterial.m_userId == 0)
		{
			return false;
		}

		LayerEnum l0 = (LayerEnum)(instanceShape0.m_shapeMaterial.m_userId - 1);
		LayerEnum l1 = (LayerEnum)(instanceShape1.m_shapeMaterial.m_userId - 1);

		return CollisionMatrix::CanCollide(l0, l1);
	}

	void ndContactCallback::OnContactCallback(dInt32 threadIndex, const ndContact *const contactJoint, dFloat32 timestep)
	{
		const ndMaterial& material = contactJoint->GetMaterial();


	}
}