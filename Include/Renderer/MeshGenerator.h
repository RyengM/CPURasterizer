#pragma once

#include "Math.h"
#include "Mesh.h"

namespace LearnTask
{
	class MeshGenerator
	{
	public:
		// The number of vertex is m * n
		static Mesh CreateGrid(float width, float depth, float m, float n);
	};
}