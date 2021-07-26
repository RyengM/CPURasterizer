#include <Renderer/Camera.h>
#include <algorithm>

using namespace LearnTask;

Mat4 Camera::Perspective()
{
	float angle = Util::Angle2Radian(fov);
	float top = tan(angle / 2.f) * nearPlane;
	float bottom = -top;
	float right = top * aspect;
	float left = -right;

	float arrOrthoTranslate[16] =		{ 1, 0, 0, -(left + right) / 2.f,
										  0, 1, 0, -(top + bottom) / 2.f,
										  0, 0, 1, -(nearPlane + farPlane) / 2.f,
										  0, 0, 0, 1 };
	Mat4 orthoTranslate(arrOrthoTranslate);
	float arrOrthoScale[16] =			{ 2.f / (right - left), 0, 0, 0,
										  0, 2.f / (top - bottom), 0, 0,
										  0, 0, 2.f / (farPlane - nearPlane), 0,
										  0, 0, 0, 1 };
	Mat4 orthoScale(arrOrthoScale);
	float arrOrtho2Perspective[16] =	{ nearPlane, 0, 0, 0,
										  0, nearPlane, 0, 0,
										  0, 0, nearPlane + farPlane, -nearPlane * farPlane,
										  0, 0, 1, 0 };
	Mat4 proj2ortho(arrOrtho2Perspective);
	Mat4 orthoMatrix = orthoScale * orthoTranslate;

	return orthoMatrix * proj2ortho;
}

Mat4 Camera::LookAt(Vec3 src, Vec3 dest)
{
	// Left hand
	Vec3 z = (dest - src).Normalize();
	Vec3 x;
	if (!free && (int(abs(phi)) % 360 < 90 || int(abs(phi)) % 360 > 270) || free && (int(abs(pitch)) % 360 < 90 || int(abs(pitch)) % 360 > 270))
		x = worldUp.Cross(z).Normalize();
	else
		x = (-worldUp).Cross(z).Normalize();
	Vec3 y = z.Cross(x).Normalize();

	Mat4 translate(1.f);
	translate.SetElement(0, 3, -src.x);
	translate.SetElement(1, 3, -src.y);
	translate.SetElement(2, 3, -src.z);

	// Base transformation
	float arrRotate[16] = { x.x, x.y, x.z, 0,
							y.x, y.y, y.z, 0,
							z.x, z.y, z.z, 0,
							0, 0, 0, 1 };
	Mat4 rotate(arrRotate);

	return rotate * translate;
}

void Camera::ProcessKeyboard(ECameraMovement direction, float deltaTime)
{
	if (free)
	{
		float velocity = movementSpeed * deltaTime;
		if (direction == ECameraMovement::FORWARD)
			position += front * velocity;
		if (direction == ECameraMovement::BACKWARD)
			position -= front * velocity;
		if (direction == ECameraMovement::LEFT)
			position -= right * velocity;
		if (direction == ECameraMovement::RIGHT)
			position += right * velocity;
	}
}

void Camera::ProcessMouseMovement()
{
	if (free)
	{
		float sign = up.y > 0 ? -1.0f : 1.0f;
		yaw += sign * deviceStatus->delta.x;
		pitch += deviceStatus->delta.y;

		UpdateFreeCameraVectors();
	}
	else
	{
		if (deviceStatus->rightMouseActive/* && deviceStatus->leftShiftActive*/)
		{
			anchorPos += (-right * deviceStatus->delta.x + up * deviceStatus->delta.y) * 0.01f;
			position += (-right * deviceStatus->delta.x + up * deviceStatus->delta.y) * 0.01f;
		}

		if (!deviceStatus->messageBlock && deviceStatus->leftMouseActive && !deviceStatus->leftAltActive)
		{
			
			float sign = up.y > 0 ? -1.0f : 1.0f;
			theta += sign * deviceStatus->delta.x;
			phi += deviceStatus->delta.y;
			UpdateCameraVectors();
		}
	}

	UpdateView();
}

void Camera::ProcessMouseScroll(float offset)
{
	//fov -= offset;
	//if (fov < 1.f) fov = 1.f;
	//if (fov > 120.f) fov = 120.f;
	//UpdateProj();
	position += front * offset * 0.1f;
	UpdateView();
}

void Camera::UpdateFreeCameraVectors()
{
	// Calculate the new front vector
	Vec3 newFront;
	newFront.x = sin(Util::Angle2Radian(yaw)) * cos(Util::Angle2Radian(pitch));
	newFront.y = sin(Util::Angle2Radian(pitch));
	newFront.z = -cos(Util::Angle2Radian(yaw)) * cos(Util::Angle2Radian(pitch));
	front = -newFront.Normalize();
	if (int(abs(pitch)) % 360 <= 90 || int(abs(pitch)) % 360 >= 270)
		right = worldUp.Cross(front).Normalize();
	else
		right = (-worldUp).Cross(front).Normalize();
	up = front.Cross(right).Normalize();
}

void Camera::UpdateCameraVectors()
{
	float distance = (position - anchorPos).Length();
	position.x = sin(Util::Angle2Radian(theta)) * cos(Util::Angle2Radian(phi)) * distance;
	position.y = sin(Util::Angle2Radian(phi)) * distance;
	position.z = -cos(Util::Angle2Radian(theta)) * cos(Util::Angle2Radian(phi)) * distance;
	front = (-position).Normalize();
	if (int(abs(phi)) % 360 <= 90 || int(abs(phi)) % 360 >= 270)
		right = worldUp.Cross(front).Normalize();
	else
		right = (-worldUp).Cross(front).Normalize();
	up = front.Cross(right).Normalize();
	position += anchorPos;
}

void Camera::ActivateFreeMode()
{
	if (free) return;
	stackPosition = position;
	yaw = theta;
	pitch = phi;
	free = true;
}

void Camera::DisableFreeMode()
{
	free = false;
	position = stackPosition;
	UpdateCameraVectors();
}