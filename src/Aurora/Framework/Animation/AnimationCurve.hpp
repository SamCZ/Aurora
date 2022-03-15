#pragma once

#include <cstring>
#include <vector>

namespace Aurora
{
	enum class WeightedMode
	{
		None,
		In,
		Out,
		Both
	};

	enum class OverflowKeyFrameMode
	{
		Stay,
		Invert
	};

	struct ACKeyFrame
	{
		float InTangent;
		float InWeight;

		float OutTangent;
		float OutWeight;

		float Time;
		float Value;

		WeightedMode WeightMode;

		ACKeyFrame()
			: InTangent(0), InWeight(0), OutTangent(0), OutWeight(0), Time(0), Value(0), WeightMode(WeightedMode::None) {}

		ACKeyFrame(float time, float value)
			: InTangent(0), InWeight(0), OutTangent(0), OutWeight(0), Time(time), Value(value), WeightMode(WeightedMode::None) { }

		ACKeyFrame(float time, float value, float inTangent, float inWeight, float outTangent, float outWeight)
			: InTangent(inTangent), InWeight(inWeight), OutTangent(outTangent), OutWeight(outWeight), Time(time), Value(value), WeightMode(WeightedMode::None) { }

		ACKeyFrame(float time, float value, float inTangent, float inWeight, float outTangent, float outWeight, const WeightedMode& weightedMode)
			: InTangent(inTangent), InWeight(inWeight), OutTangent(outTangent), OutWeight(outWeight), Time(time), Value(value), WeightMode(weightedMode) { }
	};

	class AnimationCurve
	{
	public:
		std::vector<ACKeyFrame> Keys;
	private:
		float MinTime;
		float MaxTime;
	public:
		AnimationCurve();

		int AddKey(float time, float value);
		int AddKey(const ACKeyFrame& keyFrame);
		void RemoveKey(int index);

		float Evaluate(float time);
	private:
		static float Evaluate(float time, const ACKeyFrame& keyFrame0, const ACKeyFrame& keyFrame1);
	};
}