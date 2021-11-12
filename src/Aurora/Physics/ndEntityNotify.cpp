#include "ndEntityNotify.hpp"

namespace Aurora
{

	ndEntityNotify::ndEntityNotify(TransformComponent* transformComponent) : ndBodyNotify(dVector(0.0f, -10.0f, 0.0f, 0.0f)), m_Transform(transformComponent)
	{

	}

	void ndEntityNotify::OnApplyExternalForce(dInt32 threadIndex, dFloat32 timestep)
	{
		ndBodyKinematic* const body = GetBody()->GetAsBodyKinematic();
		dAssert(body);
		if (body->GetInvMass() > 0.0f)
		{
			dVector massMatrix(body->GetMassMatrix());
			dVector force(GetGravity().Scale(massMatrix.m_w));
			body->SetForce(force);
			body->SetTorque(dVector::m_zero);

			//dVector L(body->CalculateAngularMomentum());
			//dTrace(("%f %f %f\n", L.m_x, L.m_y, L.m_z));
		}
	}

	void ndEntityNotify::OnTransform(dInt32 threadIndex, const dMatrix &matrix)
	{
		/*m_Transform->Translation = Vector3(matrix.m_posit.m_x, matrix.m_posit.m_y, matrix.m_posit.m_z);

		dQuaternion rot(matrix);

		dVector euler0;
		dVector euler1;
		matrix.CalcPitchYawRoll(euler0, euler1);

		m_Transform->Rotation = Vector3(glm::degrees(euler1.m_x), glm::degrees(euler1.m_y), glm::degrees(euler1.m_z));*/

		m_Transform->SetFromMatrix(glm::make_mat4(&matrix.m_front.m_x));
	}
}