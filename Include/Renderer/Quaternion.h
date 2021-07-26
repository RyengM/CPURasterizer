#pragma once
#include "Vertor.h"
#include "Matrix.h"

namespace LearnTask
{
	struct Quaternion
	{
		float w, x, y, z;
		Vec3 v;

		Quaternion() {}
		Quaternion(float w, float x, float y, float z);
		Quaternion(float theta, const Vec3& axis);

		Quaternion operator*(const Quaternion& q) const;
		Quaternion operator/(float num) const;

		Quaternion Conjugate();
		Quaternion Inverse();

		Mat4 ToRotateMatrix();
	};
}