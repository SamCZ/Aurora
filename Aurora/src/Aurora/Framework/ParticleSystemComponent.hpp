#pragma once

#include "SceneComponent.hpp"
#include "ParticleEffect.hpp"

namespace Aurora
{
	struct Particle
	{
		Vector4 Pos = {0, 0, 0, 0};
		float LifeTime = 0;
		bool InBuffer = false;
	};

	class ParticleSystemComponent : public SceneComponent
	{
	public:
		Buffer_ptr m_GPUPosBuffer;
		std::vector<Particle> m_Particles;
		uint32_t m_MaxParticles = 100;
		uint32_t m_CurrentParticles = 0;
	public:
		CLASS_OBJ(ParticleSystemComponent, SceneComponent);

		ParticleSystemComponent() : SceneComponent()
		{
			m_Particles.resize(m_MaxParticles);
			m_GPUPosBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("PosBuff", sizeof(Vector4) * m_MaxParticles, EBufferType::ShaderStorageBuffer));
		}

		void Tick(double delta) override
		{
			Vector4* positions = GEngine->GetRenderDevice()->MapBuffer<Vector4>(m_GPUPosBuffer, EBufferAccess::WriteOnly);

			static double timer = 0;

			if (timer > 0.1)
			{
				SpawnParticle(Vector3(0.0), 2.0f);
				timer = 0;
			}
			timer += delta;

			m_CurrentParticles = 0;

			for (Particle& particle : m_Particles)
			{
				if (particle.LifeTime <= 0.0f)
				{
					break;
				}

				if (not particle.InBuffer)
				{
					positions[m_CurrentParticles] = particle.Pos;
					particle.InBuffer = true;
				}

				particle.LifeTime -= (float)delta;
				m_CurrentParticles++;
			}

			GEngine->GetRenderDevice()->UnmapBuffer(m_GPUPosBuffer);
		}

		void SpawnParticle(const Vector3& location, float lifetime)
		{
			for (Particle& particle : m_Particles)
			{
				if (particle.LifeTime > 0.0f)
				{
					continue;
				}

				particle.LifeTime = lifetime;
				particle.InBuffer = false;
				particle.Pos = Vector4(GetWorldPosition() + location, 1.0f);
				break;
			}
		}
	};
}