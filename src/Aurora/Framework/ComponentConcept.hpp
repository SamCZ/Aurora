#pragma once

namespace Aurora
{
	class ActorComponent;

	template<typename T>
	concept ComponentType = requires(T a) {
		{ a } -> std::convertible_to<ActorComponent>;
	};
}