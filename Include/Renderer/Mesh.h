#pragma once
#include "Math.h"
#include <vector>
#include <string>

namespace LearnTask
{
	struct Vertex
	{
		Vec4 pos;
		Vec3 normal;
		Vec3 color;
		Vec3 tangentU;
		Vec2 texCoord;
	};

	struct Triangle
	{
		Vertex vertex[3];
	};

	struct Mesh
	{
		std::string name;
		bool bTangent = false;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	class MeshLoader
	{
	public:
		Mesh LoadObj(const std::string& path);
	};
}