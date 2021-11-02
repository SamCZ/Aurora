#pragma once

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

		void OnApplyExternalForce(dInt32 threadIndex, dFloat32 timestep) override;
		void OnTransform(dInt32 threadIndex, const dMatrix& matrix) override;
	};
}
