#pragma once

namespace Aurora
{
	class GameModeBase
	{
	public:
		virtual ~GameModeBase() = default;

		virtual void BeginPlay() = 0;
		virtual void BeginDestroy() = 0;
		virtual void Tick(double delta) = 0;
	};
}