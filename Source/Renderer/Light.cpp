#include <Renderer/Light.h>

using namespace LearnTask;

void DirectionalLight::ProcessMouseMovement()
{
	if (deviceStatus->leftMouseActive && deviceStatus->leftAltActive)
	{
		theta += deviceStatus->delta.x;
		phi += deviceStatus->delta.y;
	}
}

void DirectionalLight::UpdateLightVectors()
{
	if (distance < 0.1f) distance = 0.1f;
	pos.x = -sin(Util::Angle2Radian(theta)) * cos(Util::Angle2Radian(phi)) * distance;
	pos.y = -sin(Util::Angle2Radian(phi)) * distance;
	pos.z = cos(Util::Angle2Radian(theta)) * cos(Util::Angle2Radian(phi)) * distance;
	dir = -pos.Normalize();
}