#pragma once

#ifdef NEWTON

#include <ndBodyDynamic.h>
#include "../Framework/BaseComponents.hpp"

namespace Aurora
{
	class ndEntityNotify : public ndBodyNotify
	{
	private:
		TransformComponent* m_Transform;
	public:
		ndEntityNotify(TransformComponent* transformComponent);

		void OnApplyExternalForce(ndInt32 threadIndex, ndFloat32 timestep) override;
		void OnTransform(ndInt32 threadIndex, const ndMatrix& matrix) override;
	};
}
#endif