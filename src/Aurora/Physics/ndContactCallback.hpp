#pragma once
#ifdef NEWTON
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

			ndMaterialKey(ndUnsigned64 low, ndUnsigned64 high) : m_lowKey(ndUnsigned64(dMin(low, high))) ,m_highKey(ndUnsigned64(dMax(low, high))) { }

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
                    ndUnsigned64 m_lowKey;
                    ndUnsigned64 m_highKey;
				};
                ndUnsigned64 m_key;
			};
		};
	private:
		ndTree<ndMaterial, ndMaterialKey> m_MaterialMap;
	public:
		ndMaterial& RegisterMaterial(ndUnsigned64 id0, ndUnsigned64 id1);

		ndMaterial GetMaterial(const ndContact* contactJoint, const ndShapeInstance& instance0, const ndShapeInstance& instance1) const override;
		bool OnAabbOverlap(const ndContact* contactJoint, ndFloat32 timestep) override;
		void OnContactCallback(ndInt32 threadIndex, const ndContact* contactJoint, ndFloat32 timestep) override;
	};
}
#endif