#include "AnimationCurve.hpp"
#include <cmath>

namespace Aurora
{
	AnimationCurve::AnimationCurve() : MinTime(0), MaxTime(0) { }

	int AnimationCurve::AddKey(float time, float value)
	{
		return AddKey(ACKeyFrame(time, value));
	}

	int AnimationCurve::AddKey(const ACKeyFrame &keyFrame)
	{
		MinTime = std::min(MinTime, keyFrame.Time);
		MaxTime = std::max(MaxTime, keyFrame.Time);

		Keys.push_back(keyFrame);
		return (int)Keys.size() - 1;
	}

	void AnimationCurve::RemoveKey(int index)
	{
		Keys.erase(Keys.begin() + index);
	}

	float AnimationCurve::Evaluate(float time)
	{
		{ // Does not allow below zero values = needs to be as settings
			time = std::abs(time);
		}

		{ // This does inverting when going out of max time = needs to be as settings
			float maxTime2 = MaxTime * 2.0f;
			float mt = std::fmod(time, MaxTime);
			float mt2 = std::fmod(time, maxTime2);

			if (mt2 >= MaxTime && mt2 <= maxTime2)
			{
				time = MaxTime - mt;
			}
			else
			{
				time = mt;
			}
		}

		if (Keys.size() < 2)
		{
			return 0;
		}

		for (size_t i = 0; i < Keys.size() - 1; ++i)
		{
			ACKeyFrame &keyFrame0 = Keys[i];

			if (time >= keyFrame0.Time)
			{
				return Evaluate(time, keyFrame0, Keys[i + 1]);
			}
		}

		return 0;
	}

	float AnimationCurve::Evaluate(float time, const ACKeyFrame &keyFrame0, const ACKeyFrame &keyFrame1)
	{
		float dt = keyFrame1.Time - keyFrame0.Time;

		float m0 = keyFrame0.OutTangent * dt;
		float m1 = keyFrame1.InTangent * dt;

		float t2 = time * time;
		float t3 = t2 * time;

		float a = 2 * t3 - 3 * t2 + 1;
		float b = t3 - 2 * t2 + time;
		float c = t3 - t2;
		float d = -2 * t3 + 3 * t2;

		return a * keyFrame0.Value + b * m0 + c * m1 + d * keyFrame1.Value;

		/*float p1x = keyFrame0.Time;
		float p1y = keyFrame0.Value;
		float tp1 = keyFrame0.OutTangent;

		float p2x = keyFrame1.Time;
		float p2y = keyFrame1.Value;
		float tp2 = keyFrame1.InTangent;

		float a = (p1x * tp1 + p1x * tp2 - p2x * tp1 - p2x * tp2 - 2 * p1y + 2 * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x);
		float b = ((-p1x * p1x * tp1 - 2 * p1x * p1x * tp2 + 2 * p2x * p2x * tp1 + p2x * p2x * tp2 - p1x * p2x * tp1 + p1x * p2x * tp2 + 3 * p1x * p1y - 3 * p1x * p2y + 3 * p1y * p2x - 3 * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));;
		float c = ((p1x * p1x * p1x * tp2 - p2x * p2x * p2x * tp1 - p1x * p2x * p2x * tp1 - 2 * p1x * p2x * p2x * tp2 + 2 * p1x * p1x * p2x * tp1 + p1x * p1x * p2x * tp2 - 6 * p1x * p1y * p2x + 6 * p1x * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));
		float d = ((p1x * p2x * p2x * p2x * tp1 - p1x * p1x * p2x * p2x * tp1 + p1x * p1x * p2x * p2x * tp2 - p1x * p1x * p1x * p2x * tp2 - p1y * p2x * p2x * p2x + p1x * p1x * p1x * p2y + 3 * p1x * p1y * p2x * p2x - 3 * p1x * p1x * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));

		return a * time * time * time + b * time * time + c * time + d;*/
	}
}