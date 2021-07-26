#pragma once
#include "Math.h"
#include <memory>

namespace LearnTask
{
	class DeviceStatus
	{
	public:
		bool messageBlock = false;

		float mouseSensitivity = 0.2f;

		Vec2 cursorLastPos = Vec2(0.f, 0.f);
		Vec2 delta = Vec2(0.f, 0.f);

		bool leftAltActive = false;
		bool leftShiftActive = false;
		bool leftMouseActive = false;
		bool rightMouseActive = false;

		void ProcessMouseMovement(float x, float y)
		{
			Vec2 pos{ x, y };
			delta = (pos - cursorLastPos) * mouseSensitivity;

			cursorLastPos = pos;
		}
	};
}