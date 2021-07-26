#pragma once

#include "Math.h"
#include "DeviceStatus.h"

///////////////////////////
//       LEFT HAND       //
///////////////////////////
//       |y              //
//       | / z           //
//       |/__________x   //
///////////////////////////

namespace LearnTask
{
	enum class ECameraMovement {
		FORWARD = 0,
		BACKWARD = 1,
		LEFT = 2,
		RIGHT = 3
	};

	class Camera
	{
	public:
		Camera(Vec3 position, float fov, float aspect, float nearPlane, float farPlane) : position(position), fov(fov), aspect(aspect), nearPlane(nearPlane), farPlane(farPlane)
		{
			UpdateProj();
			UpdateView();
		}
		
		Mat4 Perspective();

		Mat4 LookAt(Vec3 src, Vec3 dest);

		inline void UpdateProj()
		{
			projMatrix = Perspective();
		}

		inline void UpdateView()
		{
			viewMatrix = LookAt(position, position + front);
			viewProjMatrix = projMatrix * viewMatrix;
		}

		inline Mat4 GetViewMatrix() noexcept
		{
			return viewMatrix;
		}

		inline Mat4 GetProjMatrix() noexcept
		{
			return projMatrix;
		}

		inline Mat4 GetViewProjMatrix() noexcept
		{
			return viewProjMatrix;
		}

		void ProcessKeyboard(ECameraMovement direction, float deltaTime);

		void ProcessMouseMovement();

		void ProcessMouseScroll(float offset);

		void ActivateFreeMode();

		void DisableFreeMode();

	private:
		void UpdateFreeCameraVectors();

		void UpdateCameraVectors();

	public:
		std::shared_ptr<DeviceStatus> deviceStatus;

		Mat4 viewMatrix = Mat4(1.f);
		Mat4 projMatrix = Mat4(1.f);
		Mat4 viewProjMatrix = Mat4(1.f);
		// Camera Attributes
		Vec3 position = Vec3(0.f, 0.f, 0.f);
		Vec3 front = Vec3(0.f, 0.f, 1.f);
		Vec3 up = Vec3(0.f, 1.f, 0.f);
		Vec3 right = Vec3(1.f, 0.f, 0.f);
		Vec3 worldUp = Vec3(0.f, 1.f, 0.f);
		// Euler Angles
		float theta = 0.f;
		float phi = 0.f;
		// Camera options
		float movementSpeed = 2.5f;
		float fov = 60.f;
		float aspect = 16.f / 9.f;
		float nearPlane = 0.1;
		float farPlane = 1000.f;
		// Free mode
		bool free = false;
		float yaw = 0.f;
		float pitch = 0.f;
		// Rotate mode stack data
		Vec3 stackPosition = Vec3(0.f, 0.f, 0.f);
		// Rotate mode
		Vec3 anchorPos = Vec3(0.f);
	};
}