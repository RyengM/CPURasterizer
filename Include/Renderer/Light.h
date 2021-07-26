#pragma once

#include "Math.h"
#include "DeviceStatus.h"

namespace LearnTask
{
	class Light
	{
	public:
		Vec3 pos;
		Vec3 power;
	};

	class DirectionalLight : public Light
	{
	public:
		Vec3 dir = Vec3(0.f, 1.f, 0.f);

		float theta = 0.f;
		float phi = -90.f;
		float distance = 3.f;

		std::shared_ptr<DeviceStatus> deviceStatus;

		void ProcessMouseMovement();

		void UpdateLightVectors();
	};
}

