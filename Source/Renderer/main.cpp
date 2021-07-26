#include "Renderer/Math.h"
#include "Renderer/Mesh.h"
#include "Renderer/Texture.h"
#include <string>
#include <iostream>

using namespace LearnTask;

int main()
{
	Vec3 v1;
	Vec3 v2(1, 2, 3);
	Vec4 v3(v2);
	Mat4 m1(1);
	Mat4 m2(2);
	Vec4 v4 = m1 * m2 * v3;
	Triangle t;
	Texture tex("test.png");
	Texture tex2 = std::move(tex);
	std::cout << v4.x << std::endl;
	getchar();
	return 0;
}