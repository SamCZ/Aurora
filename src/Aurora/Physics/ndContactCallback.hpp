#pragma once

#include <ndBodyKinematic.h>
#include <ndContactNotify.h>

namespace Aurora
{
	class ndContactCallback : public ndContactNotify
	{
	public:
		class ndMaterialKey
		{
		public:
			ndMaterialKey()
			:m_key(0) { }

			ndMaterialKey(dUnsigned64 low, dUnsigned64 high) : m_lowKey(dUnsigned32(dMin(low, high))) ,m_highKey(dUnsigned32(dMax(low, high))) { }

			bool operator<(const ndMaterialKey& other) const
			{
				return (m_key < other.m_key);
			}

			bool operator>(const ndMaterialKey& other) const
			{
				return (m_key > other.m_key);
			}

			union
			{
				struct
				{
					dUnsigned32 m_lowKey;
					dUnsigned32 m_highKey;
				};
				dUnsigned64 m_key;
			};
		};
	private:
		dTree<ndMaterial, ndMaterialKey> m_MaterialMap;
	public:
		ndMaterial& RegisterMaterial(dUnsigned32 id0, dUnsigned32 id1);

		ndMaterial GetMaterial(const ndContact* contactJoint, const ndShapeInstance& instance0, const ndShapeInstance& instance1) const override;
		bool OnAabbOverlap(const ndContact* contactJoint, dFloat32 timestep) override;
		void OnContactCallback(dInt32 threadIndex, const ndContact* contactJoint, dFloat32 timestep) override;
	};
}