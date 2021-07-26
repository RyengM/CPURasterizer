#include <Renderer/MeshGenerator.h>

using namespace LearnTask;

Mesh MeshGenerator::CreateGrid(float width, float depth, float m, float n)
{
	Mesh mesh;
	mesh.bTangent = true;
	mesh.vertices.resize(m * n);
	mesh.indices.resize((m - 1) * (n - 1) * 6);

	float halfWidth = width / 2.f;
	float halfDepth = depth / 2.f;

	float dx = width / (m - 1);
	float dz = width / (n - 1);

	// Vertex
	for (int i = 0; i < n; ++i)
	{
		float z = halfDepth - i * dz;	
		for (int j = 0; j < m; ++j)
		{
			float x = -halfWidth + j * dx;

			mesh.vertices[i * m + j].pos = Vec3(x, 0.f, z);
			mesh.vertices[i * m + j].normal = Vec3(0.f, 1.f, 0.f);
			mesh.vertices[i * m + j].tangentU = Vec3(1.f, 0.f, 0.f);

			// Stretch texture over grid.
			mesh.vertices[i * m + j].texCoord = Vec2(j, i);
		}
	}
	
	// Index
	int k = 0;
	for (int i = 0; i < n - 1; ++i)
	{
		for (int j = 0; j < m - 1; ++j)
		{
			mesh.indices[k] = i * m + j;
			mesh.indices[k + 1] = i * m + j + 1;
			mesh.indices[k + 2] = (i + 1) * m + j;

			mesh.indices[k + 3] = (i + 1) * m + j;
			mesh.indices[k + 4] = i * m + j + 1;
			mesh.indices[k + 5] = (i + 1) * m + j + 1;

			k += 6;
		}
	}

	return mesh;
}