#pragma once

#include "FrameBufferObject.h"
#include "Renderer/Rasterizer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <memory>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

namespace LearnTask
{
	class Editor
	{
	public:
		Editor() {}

		~Editor();

		void Init();

		void Draw();

		void Finish();

		bool SetupImgui();

		void PrepareCanvasResource();
		
		void RenderToScreen();

		void ProcessInput(GLFWwindow* window);

	private:
		unsigned int shaderProgram;
		unsigned int softRasterizerVao;
		unsigned int softRasterizerTexture;
		unsigned int debugTexture;

		std::unique_ptr<FrameBufferObject> fbDebugRasterizer;
		std::unique_ptr<Rasterizer> softRasterizer;

		GLFWwindow* window = nullptr;
		const unsigned int screenWidth = 1600;
		const unsigned int screenHeight = 900;

		float lastFrame = 0.f;
		float deltaTime = 0.f;
	};
}