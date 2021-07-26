#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <string>

namespace LearnTask
{
	struct Material
	{
		std::string name = "";

		Texture* diffuseTex = nullptr;
		Texture* normalTex = nullptr;

		Material(std::string name) : name(name) {};
	};

	struct RenderItem
	{
		std::string name = "";

		Vec3 position;
		Vec3 rotation;
		Vec3 scale = Vec3(1.f);
		Mat4 world;

		Mesh* mesh = nullptr;
		Material* mat = nullptr;
		Shader* shader = nullptr;

		bool bDisplay = true;

		RenderItem(std::string name) : name(name) {};
		inline void UpdateWorld() 
		{ 
			world = Mat4(1.f).Scale(scale).Translate(position);
		}
	};
}
