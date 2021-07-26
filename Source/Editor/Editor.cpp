#include <Editor/Editor.h>
#include <Renderer/Texture.h>

#include <stdio.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using namespace LearnTask;

static void* address = nullptr;

Editor::~Editor()
{
	address = nullptr;
}

void Editor::Init()
{
	SetupImgui();

	softRasterizer = std::make_unique<Rasterizer>();
	softRasterizer->Init();
	fbDebugRasterizer = std::make_unique<FrameBufferObject>(screenWidth, screenHeight);
	fbDebugRasterizer->BuildFrameBuffer();

	PrepareCanvasResource();

	// Get address for static callback
	address = (void*)(softRasterizer.get());
}

void Editor::Draw()
{
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		ProcessInput(window);
		
		// Update rasterizer
		softRasterizer->Update();

		// Call OpenGL API to render rasterizer data
		RenderToScreen();
	}
}

void Editor::RenderToScreen()
{
	glfwPollEvents();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static int selected = 0;

	softRasterizer->deviceStatus->messageBlock = false;
	if (ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered())
		softRasterizer->deviceStatus->messageBlock = true;
	
	{
		ImGui::Begin("Camera");
		{
			ImGui::LabelText("label", "Value");
			{
				ImGui::DragFloat3("pos", reinterpret_cast<float*>(&softRasterizer->camera->position), 0.1f);
				ImGui::DragFloat("fov", reinterpret_cast<float*>(&softRasterizer->camera->fov), 0.1f);
			}
		}
		ImGui::End();
	}

	{
		ImGui::Begin("DirectionalLight");
		{
			ImGui::LabelText("label", "Value");
			{
				ImGui::DragFloat3("power", reinterpret_cast<float*>(&softRasterizer->dirLight->power), 0.1f);
				ImGui::DragFloat3("dir", reinterpret_cast<float*>(&softRasterizer->dirLight->dir), 0.1f);
				ImGui::DragFloat("distance", reinterpret_cast<float*>(&softRasterizer->dirLight->distance), 0.1f);
			}
		}
		ImGui::End();
	}

	{
		ImGui::Begin("Objects");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("CullMode: %s", softRasterizer->cullMode[softRasterizer->faceCull]);
		ImGui::Text("ShadowMode: %s", softRasterizer->shaders["phong"]->shadowMode[softRasterizer->shaders["phong"]->shadowType]);
		for (int i = 0; i < softRasterizer->renderItems.size(); i++)
		{
			if (ImGui::Selectable(softRasterizer->renderItems[i]->name.c_str(), selected == i))
			{
				selected = i;
				if (i >= softRasterizer->candidateItemStartPos && i < softRasterizer->candidateItemStartPos + softRasterizer->candidateSwitchItems.size())
				{
					softRasterizer->candidateSwitchItems[softRasterizer->curModelNum]->bDisplay = false;
					softRasterizer->candidateSwitchItems[i - softRasterizer->candidateItemStartPos]->bDisplay = true;
					softRasterizer->curModelNum = i - softRasterizer->candidateItemStartPos;
				}
			}
		}
		ImGui::End();
	}

	{
		ImGui::Begin("Details");
		{
			ImGui::Text(softRasterizer->renderItems[selected]->name.c_str());
			ImGui::LabelText("label", "Value");
			{
				ImGui::DragFloat3("translate", reinterpret_cast<float*>(&softRasterizer->renderItems[selected]->position), 0.1f);
				ImGui::DragFloat3("scale", reinterpret_cast<float*>(&softRasterizer->renderItems[selected]->scale), 0.1f);
			}
		}
		ImGui::End();
	}

	{
		ImGui::Begin("Debug");
		{
			ImGui::BeginChild("RT");
			glBindFramebuffer(GL_FRAMEBUFFER, fbDebugRasterizer->GetFrameBufferID());
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(shaderProgram);
			glBindVertexArray(softRasterizerVao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, debugTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, softRasterizer->shadowBuffer->GetWidth(), softRasterizer->shadowBuffer->GetHeight(), 0, GL_LUMINANCE, GL_FLOAT, softRasterizer->shadowBuffer->depth);
			glGenerateMipmap(GL_TEXTURE_2D);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			ImVec2 wsize = ImGui::GetWindowSize();
			ImGui::Image((ImTextureID)fbDebugRasterizer->GetFrameBufferID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			ImGui::EndChild();
		}
		ImGui::End();
	}

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT); GLenum format = GL_RGBA;

	// Draw main view
	glUseProgram(shaderProgram);
	glBindVertexArray(softRasterizerVao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, softRasterizerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, softRasterizer->frameBuffer->GetWidth(), softRasterizer->frameBuffer->GetHeight(), 0, format, GL_UNSIGNED_BYTE, softRasterizer->frameBuffer->GetBuffer());
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, softRasterizer->frameBuffer->GetWidth(), softRasterizer->frameBuffer->GetHeight(), 0, GL_LUMINANCE, GL_FLOAT, softRasterizer->frameBuffer->depth);
	glGenerateMipmap(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Editor::PrepareCanvasResource()
{
	// Pos: vec3, TexCoord: vec2
	float vertices[] =
	{
		-1.f, -1.f, 0.f, 0.f, 0.f,
		 1.f, -1.f, 0.f, 1.f, 0.f,
		-1.f,  1.f, 0.f, 0.f, 1.f,
		 1.f,  1.f, 0.f, 1.f, 1.f
	};

	// Shader
	const char* vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 vPos;\n"
		"layout (location = 1) in vec2 vTexCoord;\n"
		"out vec2 pTexCoord;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(vPos, 1.0);\n"
		"	pTexCoord = vTexCoord;\n"
		"}\n";

	const char* fragmentShaderSource = 
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 pTexCoord;\n"
		"uniform sampler2D tex;\n"
		"void main()\n"
		"{\n"
		"	FragColor = vec4(texture(tex, pTexCoord).xyz, 1.0);\n"
		"}\n";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Vertex data
	unsigned int vbo;
	glGenVertexArrays(1, &softRasterizerVao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(softRasterizerVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Texture
	glGenTextures(1, &softRasterizerTexture);
	glBindTexture(GL_TEXTURE_2D, softRasterizerTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &debugTexture);
	glBindTexture(GL_TEXTURE_2D, debugTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto addr = (Rasterizer*)address;
	addr->camera->ProcessMouseScroll(yoffset);
}

bool Editor::SetupImgui()
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return false;

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	window = glfwCreateWindow(screenWidth, screenHeight, "Learn Task", NULL, NULL);
	if (window == NULL)
		return false;
	glfwSetScrollCallback(window, glfw_scroll_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	bool err = gladLoadGL() == 0;
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return false;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return true;
}

void Editor::Finish()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Editor::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (softRasterizer->camera->free)
			softRasterizer->camera->DisableFreeMode();
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		softRasterizer->camera->ProcessKeyboard(ECameraMovement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		softRasterizer->camera->ProcessKeyboard(ECameraMovement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		softRasterizer->camera->ProcessKeyboard(ECameraMovement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		softRasterizer->camera->ProcessKeyboard(ECameraMovement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		softRasterizer->SwitchCullMode();
		softRasterizer->bFaceCullSwitching = true;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
		softRasterizer->bFaceCullSwitching = false;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		softRasterizer->shaders["phong"]->SwitchShadow();
		softRasterizer->shaders["phong"]->bShadowSwitch = true;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
		softRasterizer->shaders["phong"]->bShadowSwitch = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		softRasterizer->camera->ActivateFreeMode();
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		softRasterizer->deviceStatus->leftShiftActive = true;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		softRasterizer->deviceStatus->leftShiftActive = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		softRasterizer->deviceStatus->leftAltActive = true;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
		softRasterizer->deviceStatus->leftAltActive = false;
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		softRasterizer->renderMode = ERenderMode::WireFrame;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		softRasterizer->renderMode = ERenderMode::Render;
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		softRasterizer->SwitchModel();
		softRasterizer->bModelSwitching = true;
	}
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
		softRasterizer->bModelSwitching = false;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		softRasterizer->deviceStatus->leftMouseActive = true;
	if (glfwGetMouseButton(window, 0) == GLFW_RELEASE)
		softRasterizer->deviceStatus->leftMouseActive = false;
	if (glfwGetMouseButton(window, 1) == GLFW_PRESS)
		softRasterizer->deviceStatus->rightMouseActive = true;
	if (glfwGetMouseButton(window, 1) == GLFW_RELEASE)
		softRasterizer->deviceStatus->rightMouseActive = false;

	softRasterizer->deviceStatus->ProcessMouseMovement(x, y);
	softRasterizer->camera->ProcessMouseMovement();
	softRasterizer->dirLight->ProcessMouseMovement();
}