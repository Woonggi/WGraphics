/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Graphics.cpp
Purpose: Main renderer for obj files and GUI.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#include <iostream>
#include <GL/glew.h>

#include "Graphics.h"
#include "Camera.h"
#include "InputManager.h"
#include "Shader.h"
#include "OBJLoader.h"
#include "TextureLoader.h"
#include "Object.h"
#include "ObjectManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdlib>
#include <time.h>


namespace
{
	const int ORBIT_LOD = 180;
	const float PI = 3.141592f;
	
	OBJ* skyboxOBJ = nullptr;
}
void FrameBufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
	glViewport(0, 0, width, height);
}

Graphics::Graphics(int /*width*/, int /*height*/)
	: m_window(nullptr)
	, m_camera(nullptr)
	, m_width(1280), m_height(720)
	, m_zNear(0.1f), m_zFar(15.f)
	, m_vao(0)
	, m_sphVbo(0)
	, m_objCurrentShader(0)
	, m_phongShadingID(0)
	, m_blinShadingID(0)
	, m_phongLightingID(0)
	, m_normalProgramID(0)
	, m_sphIbo(0)
	, m_sphVn(0)
	, m_isFill(true)
	, m_showFn(false)
	, m_showVn(false)
	, m_showObj(true)
	, m_buffers(nullptr)
	, m_spheres(nullptr)
	, m_orbVbo(0)
	, m_orbVertSize(0)
	, m_texture1(0)
	, m_texture2(0)
{
	
}

Graphics::~Graphics(void)
{

}

bool Graphics::Initialize(void)
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize glfw" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(m_width, m_height, "WGraphics++", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(m_window);
	glewExperimental = true;
	glfwSetFramebufferSizeCallback(m_window, FrameBufferSizeCallback);
	glEnable(GL_DEPTH_TEST);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize glad." << std::endl;
		return false;
	}

	// camera allocation
	m_camera = new Camera(m_window, m_width, m_height);
	m_fbCamera[SKYBOX::FRONT]  = new Camera(m_window, glm::vec3(0.0f, 0.0f, -1.0f));
	m_fbCamera[SKYBOX::RIGHT]  = new Camera(m_window, glm::vec3(1.0f, 0.0f, 0.0f));
	m_fbCamera[SKYBOX::BACK]   = new Camera(m_window, glm::vec3(0.0f, 0.0f, 1.0f));
	m_fbCamera[SKYBOX::LEFT]   = new Camera(m_window, glm::vec3(-1.0f, 0.0f, 0.0f));
	m_fbCamera[SKYBOX::TOP]    = new Camera(m_window, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_fbCamera[SKYBOX::BOTTOM] = new Camera(m_window, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	
	m_camera->SetPosition(glm::vec3(0.0f, 1.0f, 8.f));
	
	// Imgui initialize
	IMGUI_CHECKVERSION();
	const char* glslVersion = "#version 430 core";
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	ImGui::StyleColorsDark();

	// version check
	char* glVersion = {};
	glVersion = (char*)glGetString(GL_VERSION);
	std::cout << "openGL version :  " << glVersion << std::endl;

	CompileShaders();
	m_objCurrentShader = m_phongShadingID;
	// vertex array object
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	ObjectManager* objManager = &ObjectManager::getInstance();
	const size_t numObjects = objManager->GetObjects().size();
	m_buffers = new ObjectBuffer[numObjects];
	int i = 0;
	for (auto& it : objManager->GetObjects())
	{
		SetupObjBuffers(it->GetOBJ(), m_buffers[i]);
		SetupUV(it->GetOBJ(), m_buffers[i]);
		++i;
	}
	m_spheres = new Object(LoadSphere(18, 1.0f));
	OBJ* sphOBJ = m_spheres->GetOBJ();
	Texture* tex_spec = LoadTexture("textures/metal_roof_diff_512x512.ppm");
	Texture* tex_diff = LoadTexture("textures/metal_roof_spec_512x512.ppm");
	
	std::string skyboxPath = "textures/skybox";
	std::vector<std::string> faces =
	{
		"textures/skybox/front.jpg",
		"textures/skybox/right.jpg",
		"textures/skybox/back.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg"
	};
	for (int i = 0; i < 6; ++i) {
		m_cubeTexture[i] = LoadCubeMap(faces[i]);
	}

	skyboxOBJ  = LoadOBJFromFile("obj/quad.obj");
	size_t skyboxVertices = skyboxOBJ->vertexCount * sizeof(glm::vec3);

	std::vector<glm::vec3> skyboxVerticesData[6];
	std::vector<glm::vec2> skyboxUV;

	skyboxUV.push_back(glm::vec2(0, 1));
	skyboxUV.push_back(glm::vec2(1, 1));
	skyboxUV.push_back(glm::vec2(1, 0));
	skyboxUV.push_back(glm::vec2(0, 0));

	skyboxVerticesData[SKYBOX::FRONT].push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	skyboxVerticesData[SKYBOX::FRONT].push_back(glm::vec3(2.0f, 0.0f, 0.0f));
	skyboxVerticesData[SKYBOX::FRONT].push_back(glm::vec3(2.0f, 2.0f, 0.0f));
	skyboxVerticesData[SKYBOX::FRONT].push_back(glm::vec3(0.0f, 2.0f, 0.0f));

	skyboxVerticesData[SKYBOX::RIGHT].push_back(glm::vec3(2.0f, 0.0f, 0.0f));
	skyboxVerticesData[SKYBOX::RIGHT].push_back(glm::vec3(2.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::RIGHT].push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	skyboxVerticesData[SKYBOX::RIGHT].push_back(glm::vec3(2.0f, 2.0f, 0.0f));

	skyboxVerticesData[SKYBOX::LEFT].push_back(glm::vec3(0.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::LEFT].push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	skyboxVerticesData[SKYBOX::LEFT].push_back(glm::vec3(0.0f, 2.0f, 0.0f));
	skyboxVerticesData[SKYBOX::LEFT].push_back(glm::vec3(0.0f, 2.0f, 2.0f));

	skyboxVerticesData[SKYBOX::BACK].push_back(glm::vec3(2.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::BACK].push_back(glm::vec3(0.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::BACK].push_back(glm::vec3(0.0f, 2.0f, 2.0f));
	skyboxVerticesData[SKYBOX::BACK].push_back(glm::vec3(2.0f, 2.0f, 2.0f));

	skyboxVerticesData[SKYBOX::TOP].push_back(glm::vec3(0.0f, 2.0f, 0.0f));
	skyboxVerticesData[SKYBOX::TOP].push_back(glm::vec3(2.0f, 2.0f, 0.0f));
	skyboxVerticesData[SKYBOX::TOP].push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	skyboxVerticesData[SKYBOX::TOP].push_back(glm::vec3(0.0f, 2.0f, 2.0f));

	skyboxVerticesData[SKYBOX::BOTTOM].push_back(glm::vec3(0.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::BOTTOM].push_back(glm::vec3(2.0f, 0.0f, 2.0f));
	skyboxVerticesData[SKYBOX::BOTTOM].push_back(glm::vec3(2.0f, 0.0f, 0.0f));
	skyboxVerticesData[SKYBOX::BOTTOM].push_back(glm::vec3(0.0f, 0.0f, 0.0f));

	glGenVertexArrays(1, &m_skyBoxVao);
	for (int i = 0; i < 6; ++i)
	{
		glGenBuffers(1, &m_skyBoxVbo[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), skyboxVerticesData[i].data(), GL_STATIC_DRAW);
		glGenBuffers(1, &m_skyBoxUV);
		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxUV);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), skyboxUV.data(), GL_STATIC_DRAW);

	}

	glGenBuffers(1, &m_skyBoxidx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skyBoxidx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyboxOBJ->indexCount * sizeof(glm::ivec3), skyboxOBJ->indices.data(), GL_STATIC_DRAW);

	size_t sphVertexSize = sphOBJ->vertexCount * sizeof(glm::vec3);
	glGenBuffers(1, &m_sphVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_sphVbo);
	glBufferData(GL_ARRAY_BUFFER, sphVertexSize, sphOBJ->vertices.data(), GL_STATIC_DRAW);

	size_t sphNormalSize = sphOBJ->pureVertexNormal.size() * sizeof(glm::vec3);
	glGenBuffers(1, &m_sphVn);
	glBindBuffer(GL_ARRAY_BUFFER, m_sphVn);
	glBufferData(GL_ARRAY_BUFFER, sphNormalSize, sphOBJ->pureVertexNormal.data(), GL_STATIC_DRAW);

	size_t sphIndicesSize = sphOBJ->indexCount * sizeof(glm::ivec3);
	glGenBuffers(1, &m_sphIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphIndicesSize, sphOBJ->indices.data(), GL_STATIC_DRAW);

	// Setting up orbit
	std::vector<glm::vec3> orbVertices;
	float angle = 0.0f;
	while(angle < 2 * PI)
	{
		orbVertices.push_back(glm::vec3(cosf(angle), sinf(angle), 0.0f));
		angle += (glm::radians(360.f / ORBIT_LOD));
	}

	m_orbVertSize = orbVertices.size();
	size_t orbIndicesSize = m_orbVertSize * sizeof(glm::vec3);
	glGenBuffers(1, &m_orbVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_orbVbo);
	glBufferData(GL_ARRAY_BUFFER, orbIndicesSize, orbVertices.data(), GL_STATIC_DRAW);

	glGenTextures(1, &m_texture1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture1);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_diff->width, tex_diff->height, 0, GL_RGB, GL_FLOAT, tex_spec->rgb.data());

	glGenTextures(1, &m_texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, tex_diff->width, tex_diff->height, 0, GL_RGB, GL_FLOAT, tex_diff->rgb.data());

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	for (int i = 0; i < 6; ++i)
	{
		glGenTextures(1, &m_fbTexture[i]);
		glBindTexture(GL_TEXTURE_2D, m_fbTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbTexture[i], 0);
	}

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_width);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void Graphics::Update(float dt)
{
	SelectMode();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 proj = glm::perspective(
		glm::radians(90.0f),
		(float)m_width / (float)m_height,
		m_zNear,
		100.0f	
	);
	glm::mat4 view = m_camera->GetViewMatrix();


	if (InputManager::getInstance().isKeyDown(KEY_W))
	{
		m_camera->Move(glm::vec3(0.0f, 0.0f, -1.0f), 10.0f * dt);
	}

	if (InputManager::getInstance().isKeyDown(KEY_S))
	{
		m_camera->Move(glm::vec3(0.0f, 0.0f, 1.0f), 10.0f * dt);
	}

	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                             IMGUI setup							   //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////

	// These will be controlled in imgui control panel.
	static float rot = 0.01f;
	static float orbitSize = 5.f;
	static float sphScale = 0.1f;
	static float add_scale = 2.5f;
	static int   numLights = 16;	
	static float cutOff[MAX_LIGHTS];
	static float outerCutOff[MAX_LIGHTS];
	static float fallOff[MAX_LIGHTS];
	static bool  isRot = true;
	static bool  dataChanged = false;
	static glm::vec3 globalAmbient = {0.01f, 0.01f, 0.01f};
	float rotSpeed = 1.5f;	
	static glm::vec3 lightColorVector[MAX_LIGHTS] = {glm::vec3(1.0f)};
	static LightType lightTypes[MAX_LIGHTS] = {L_POINT};
	static int lightTypeCurrent[MAX_LIGHTS] = { 0 };
	static bool specialEffect = false;

	static bool environmentMapping = false;
	static bool isReflect = true;
	static bool isRefract = false;
	static bool shading = false;

	static float fresnel = 0.6f;

	static bool init = false;
	if (init == false)
	{
		for (int i = 0; i < MAX_LIGHTS; ++i)
		{
			cutOff[i] = 30.0f;
			outerCutOff[i] = 45.0f;
			fallOff[i] = 10.0f;
			lightColorVector[i] = glm::vec3(1.0f);
		}
		init = true;
	}
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Control Panel");

	srand(time(NULL));
	if (ImGui::Button("Scenario 1"))
	{
		// All same color, all same types
		numLights = MAX_LIGHTS;
		int randomType = rand() % 3;
		float randomFloat_1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomFloat_2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomFloat_3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		glm::vec3 randomColor = { randomFloat_1, randomFloat_2, randomFloat_3 };
		for (int i = 0; i < numLights; ++i)
		{
			lightTypes[i] = (LightType)randomType;
			lightTypeCurrent[i] = (LightType)randomType;
			lightColorVector[i] = randomColor;
		}
		specialEffect = false;
	}
	if (ImGui::Button("Scenario 2"))
	{	
		numLights = MAX_LIGHTS;
		int randomType = rand() % 3;
		for (int i = 0; i < numLights; ++i)
		{
			float randomFloat_1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float randomFloat_2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float randomFloat_3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			lightTypes[i] = (LightType)randomType;
			lightTypeCurrent[i] = (LightType)randomType;
			lightColorVector[i] = glm::vec3(randomFloat_1, randomFloat_2, randomFloat_3);
		}
		specialEffect = false;
	}
	if (ImGui::Button("Scenario 3"))
	{
		specialEffect = true;
		numLights = MAX_LIGHTS;
		int sp = 0;
		for (int i = 0; i < numLights; ++i)
		{
			int randomType = rand() % 3;
			float randomFloat_1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float randomFloat_2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float randomFloat_3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			lightTypes[i] = (LightType)randomType;
			if (lightTypes[i] == L_SPOT)
			{
				fallOff[sp] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 100.f)) + 1.f;
				outerCutOff[sp] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 100.f));
				cutOff[sp] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 100.f));
				++sp;
			}
			lightTypeCurrent[i] = (LightType)randomType;
			lightColorVector[i] = glm::vec3(randomFloat_1, randomFloat_2, randomFloat_3);
	
		}
	}

	static float ratio = 1.0f;
	ImGui::Checkbox("Shading", &shading);
	ImGui::Checkbox("Reflection", &isReflect);
	ImGui::Checkbox("Refraction", &isRefract);
	ImGui::DragFloat("Ratio", &ratio, 0.05f, 0.0f, 100.0f, "%f");
	ImGui::DragFloat("Fresnel Constant", &fresnel, 0.001f, 0.01f, 1.0f, "%f");

	const char* refractionMaterials[] =
	{
		"Air", "Hydrogen", "Water",
		"Olive Oil", "Ice", "Quartz",
		"Diamond", "Acrylic", "Plexiglas","Lucite",
	};
	static int refractionMateiralCurr = 0;
	if (ImGui::Combo("Refraction Materials\n", &refractionMateiralCurr, refractionMaterials, IM_ARRAYSIZE(refractionMaterials)))
	{
		switch (refractionMateiralCurr)
		{
		case 0:
			ratio = 1.000293f;
			break;
		case 1:
			ratio = 1.000132f;
			break;
		case 2:
			ratio = 1.333f;
			break;
		case 3:
			ratio = 1.47f;
			break;
		case 4:
			ratio = 1.31f;
			break;
		case 5:
			ratio = 1.46f;
			break;
		case 6:
			ratio = 2.42f;
			break;
		default:
			ratio = 1.49f;
			break;
		}
	}


	static float mixRatio = 0.5f;
	ImGui::DragFloat("Mix Ratio", &mixRatio, 0.01f, 0.0f, 1.0f, "%f");

	const float orbitMax = 7.0f;
	const float orbitMin = 2.5f;
	ImGui::Checkbox("Draw Object", &m_showObj);
	ImGui::Checkbox("Draw Fill", &m_isFill);
	ImGui::Checkbox("Draw Face Normal", &m_showFn);
	ImGui::Checkbox("Draw Vertex Normal", &m_showVn);
	ImGui::DragFloat("Object Scale", &add_scale, 0.1f, 0.0f, 4.0f, "%f");
	ImGui::DragFloat("Sphere Scale", &sphScale, 0.01f, 0.0f, 4.0f, "%f");
	ImGui::DragFloat("Orbit Radius", &orbitSize, 0.1f, orbitMin, orbitMax, "%f");

	static bool inc = true;
	static bool dec = false;
	if (specialEffect == true)
	{			
		if (orbitSize >= (orbitMax - 0.5f)) { 
			dec = true; inc = false; 
		}
		if (orbitSize <= (orbitMin + 0.5f)) { inc = true; dec = false; }

		if (inc == true)
		{
			orbitSize += dt * (orbitMax - orbitSize);
		}
		if (dec == true)
		{
			orbitSize -= dt * (orbitSize - orbitMin);
		}
	}
	
	const char* objFiles[] = 
	{ "4Sphere.obj", "bunny.obj",
	  "bunny_high_poly.obj", "cube.obj", "cube2.obj",
	  "cup.obj", "lucy_princeton.obj",
	  "quad.obj", "rhino.obj", 
	  "sphere.obj","sphere_modified.obj", 
	  "starwars1.obj","triangle.obj"};
	static int objFilesCurrent = 1;
	if (ImGui::Combo(".obj Files\n", &objFilesCurrent, objFiles, IM_ARRAYSIZE(objFiles)))
	{
		Object* target = ObjectManager::getInstance().GetObjects()[0];
		std::string objFilesString(objFiles[objFilesCurrent]);
		// re-load obj file and draw.
		std::string filepath = "obj/" + objFilesString;
		target->SetOBJFile(*(LoadOBJFromFile(filepath)));
		SetupObjBuffers(target->GetOBJ(), m_buffers[0]);
		SetupUV(target->GetOBJ(), m_buffers[0]);
	}

	static int shaderFileCurrent = 0;
	if (ImGui::Button("Recompile Shader"))
	{
		CompileShaders();
		m_objCurrentShader = m_phongShadingID;
		shaderFileCurrent = 0;
	}
	const char* shaderFiles[] =
	{
		"Phong Shading",
		"Phong Lighting",
		"Blin Shading"
	};
	if (ImGui::Combo(".shader Files\n", &shaderFileCurrent, shaderFiles, IM_ARRAYSIZE(shaderFiles)))
	{
		switch (shaderFileCurrent)
		{
		// Phong Shading
		case 0:
			m_objCurrentShader = m_phongShadingID;
			break;
		// Phong Lighting
		case 1:
			m_objCurrentShader = m_phongLightingID;
			break;

		// Blin Shading
		case 2: 
			m_objCurrentShader = m_blinShadingID;
			break;
		}	
	}

	ImGui::NewLine();
	
	if (ImGui::Checkbox("Toggle CPU", &m_isCPU))
	{
		m_isGPU = false;
		dataChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Toggle GPU", &m_isGPU))
	{
		m_isCPU = false;
	}

	if (ImGui::Checkbox("Planar", &m_isPlanarMap))
	{
		m_isCylMap = false;
		m_isSphMap = false;
		dataChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Cylindrical", &m_isCylMap))
	{
		m_isPlanarMap = false;
		m_isSphMap = false;
		dataChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Spherical", &m_isSphMap))
	{
		m_isPlanarMap = false;
		m_isCylMap = false;
		dataChanged = true;
	}
	if (ImGui::Checkbox("Entity Pos", &m_entityPos))
	{
		m_entityNormal = false;
		dataChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Entity Normal", &m_entityNormal))
	{
		m_entityPos = false;
		dataChanged = true;
	}
	
	ImGui::ColorEdit3("Global Ambient", (float*)&globalAmbient);
	ImGui::SliderInt("Num Lights", &numLights, 1, 16);	
	const char* lightTypesList[] = {
		"Point",
		"Directional",
		"Spot"
	};
	ImGui::Checkbox("Toggle Rotation", &isRot);
	// detrermine light type
	int numDir = 0, numPoint = 0, numSpot = 0;
	int spot = 0;
	for (int i = 0; i < numLights; ++i)
	{
		// Each light's options. (color, type);
		std::string label = "Light " + std::to_string(i);
		ImGui::Text(label.c_str());
		ImGui::ColorEdit3(label.c_str(), (float*)&lightColorVector[i]);	

		if (ImGui::Combo(label.c_str(), &lightTypeCurrent[i], lightTypesList, IM_ARRAYSIZE(lightTypesList)))
		{
			lightTypes[i] = (LightType)lightTypeCurrent[i];
		}
		if (lightTypes[i] == L_SPOT)
		{
			ImGui::DragFloat("Cut Off", &cutOff[spot], 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Outer Cut Off", &outerCutOff[spot], 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Fall Off", &fallOff[spot], 0.1f, 1.0f, 100.0f);
			spot++;
		}
	
		if (lightTypes[i] == L_DIR) { numDir++; }
		else if (lightTypes[i] == L_POINT) { numPoint++; }
		else if (lightTypes[i] == L_SPOT) { numSpot++; }
		ImGui::NewLine();
	}
		
	
	ImGui::End();
	///////////////////////////////////////////////////////////////////////////
	////                                                                     //
	////                                                                     //
	////                         Skybox rendering                            //               
	////                                                                     //
	////                                                                     //
	///////////////////////////////////////////////////////////////////////////
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glUseProgram(m_skyboxProgramID);
	// remove translation from the view matrix
	glm::mat4 skyboxView = glm::mat4(glm::mat3(m_camera->GetViewMatrix()));
	
	GLuint skyboxViewLocation = glGetUniformLocation(m_skyboxProgramID, "view");
	GLuint skyboxProjectionLocation = glGetUniformLocation(m_skyboxProgramID, "projection");
	glUniformMatrix4fv(skyboxViewLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(skyboxProjectionLocation, 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(m_skyBoxVao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	for (int i = 0; i < 6; ++i)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxVbo[i]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxUV);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_cubeTexture[i]);

		glUniform1i(glGetUniformLocation(m_skyboxProgramID, "skybox"), 3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skyBoxidx);
		glDrawElements(GL_TRIANGLES, skyboxOBJ->indexCount * 3, GL_UNSIGNED_INT, 0);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                        Sphere(Light) Rendering                      //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////
	glUseProgram(m_normalProgramID);
	m_camera->Control(0.000f, dt);

	GLuint normColorLocation = glGetUniformLocation(m_normalProgramID, "normColor");	
	GLuint normalMLocation   = glGetUniformLocation(m_normalProgramID, "model");
	GLuint normalVLocation   = glGetUniformLocation(m_normalProgramID, "view");
	GLuint normalPLocation   = glGetUniformLocation(m_normalProgramID, "proj");

	glm::vec3 lightPosVector[MAX_LIGHTS];
	for (int i = 0; i < numLights ; ++i)
	{
		float angle = i * (glm::radians(360.f / MAX_LIGHTS)) + rot;

		m_spheres->SetPosition(glm::vec3(cosf(angle) * orbitSize, 0.0f, sinf(angle) * orbitSize));
		glm::mat4 sphModel = m_spheres->GetObjectMatrix();

		// for imgui
		sphModel = glm::scale(sphModel, glm::vec3(sphScale));

		
		glUniform3f(normColorLocation, lightColorVector[i].x, lightColorVector[i].y, lightColorVector[i].z);
	
		glUniformMatrix4fv(normalMLocation, 1, GL_FALSE, glm::value_ptr(sphModel));
		glUniformMatrix4fv(normalVLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(normalPLocation, 1, GL_FALSE, glm::value_ptr(proj));

		lightPosVector[i] = m_spheres->GetPosition();

		glBindVertexArray(m_vao); 
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, m_sphVbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_sphVn);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphIbo);
		glDrawElements(GL_TRIANGLES, m_spheres->GetOBJ()->indexCount * 3, GL_UNSIGNED_INT, 0);
	}
	if (isRot == true)
	{
		rot += dt * rotSpeed;
	}


	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                        Object Rendering                             //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////

	int count = 0; 
	const int numObjects = (int)ObjectManager::getInstance().GetObjects().size();
	for (auto& it : ObjectManager::getInstance().GetObjects())
	{
		glUseProgram(m_objCurrentShader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture1);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_texture2);

		GLuint numDirLoc	= glGetUniformLocation(m_objCurrentShader, "numDir");
		GLuint numPointLoc	= glGetUniformLocation(m_objCurrentShader, "numPoint");
		GLuint numSpotLoc	= glGetUniformLocation(m_objCurrentShader, "numSpot");
		GLuint gAmbientLoc	= glGetUniformLocation(m_objCurrentShader, "g_ambient");
		GLuint fogColorLoc	= glGetUniformLocation(m_objCurrentShader, "fogColor");
		GLuint zFarLoc		= glGetUniformLocation(m_objCurrentShader, "zFar");
		GLuint zNearLoc		= glGetUniformLocation(m_objCurrentShader, "zNear");
		GLuint tex1Loc		= glGetUniformLocation(m_objCurrentShader, "tex1");
		GLuint tex2Loc		= glGetUniformLocation(m_objCurrentShader, "tex2");
		GLuint mapTypeLoc	= glGetUniformLocation(m_objCurrentShader, "mapType");
		GLuint entityModeLoc = glGetUniformLocation(m_objCurrentShader, "entity_mode");
		GLuint isGpuLoc		 = glGetUniformLocation(m_objCurrentShader, "isGpu");

		GLuint isReflectID = glGetUniformLocation(m_phongShadingID, "isReflect");
		GLuint isRefractID = glGetUniformLocation(m_phongShadingID, "isRefract");
		GLuint isMixID = glGetUniformLocation(m_phongShadingID, "isShading");

		glUniform1i(numDirLoc, numDir);
		glUniform1i(numPointLoc, numPoint);
		glUniform1i(numSpotLoc, numSpot);
		glUniform3f(gAmbientLoc, globalAmbient.x, globalAmbient.y, globalAmbient.z);
		glUniform3f(fogColorLoc, 0.15f, 0.15f, 0.15f);
		glUniform1f(zFarLoc, m_zFar);
		glUniform1f(zNearLoc, m_zNear);
		glUniform1i(tex1Loc, 0);
		glUniform1i(tex2Loc, 1);

		GLuint cubeMapsID[6] =
		{
			glGetUniformLocation(m_phongShadingID, "cube[0]"),
			glGetUniformLocation(m_phongShadingID, "cube[1]"),
			glGetUniformLocation(m_phongShadingID, "cube[2]"),
			glGetUniformLocation(m_phongShadingID, "cube[3]"),
			glGetUniformLocation(m_phongShadingID, "cube[4]"),
			glGetUniformLocation(m_phongShadingID, "cube[5]"),
		};

		if (isReflect == true) {
			glUniform1i(isReflectID, 1);
		}
		else if (isReflect == false) {
			glUniform1i(isReflectID, 0);
		}

		if (isRefract == true) {
			glUniform1i(isRefractID, 1);
		}
		else if (isRefract == false) {
			glUniform1i(isRefractID, 0);
		}

		if (shading == true) {
			glUniform1i(isMixID, 1);
		}
		else if (shading == false) {
			glUniform1i(isMixID, 0);
		}

		if (m_isPlanarMap == true)
		{
			glUniform1i(mapTypeLoc, 1);
		}
		else if (m_isCylMap == true)
		{
			glUniform1i(mapTypeLoc, 2);
		}
		else if (m_isSphMap == true)
		{
			glUniform1i(mapTypeLoc, 3);
		}

		if (m_entityPos == true)
		{
			glUniform1i(entityModeLoc, 1);
		}
		else if (m_entityNormal == true)
		{
			glUniform1i(entityModeLoc, 2);
		}

		if (m_isGPU == true)
		{
			glUniform1i(isGpuLoc, 1);
		}
		else if (m_isCPU == true)
		{
			glUniform1i(isGpuLoc, 0);
		}

		GLuint numLightLocation = glGetUniformLocation(m_objCurrentShader, "numLight");
		glUniform1i(numLightLocation, numLights);

		glm::vec3 scale = it->GetScale();
		glm::vec3 color = it->GetColor();
		glm::mat4 objModel = it->GetObjectMatrix();
		GLuint rotationMatrixLocation = glGetUniformLocation(m_objCurrentShader, "rotationMatrix");
		glUniformMatrix4fv(rotationMatrixLocation, 1, GL_FALSE, glm::value_ptr(it->GetRotationMatrix()));
		GLuint mLocation = glGetUniformLocation(m_objCurrentShader, "model");
		GLuint vLocation = glGetUniformLocation(m_objCurrentShader, "view");
		GLuint pLocation = glGetUniformLocation(m_objCurrentShader, "proj");
		GLuint viewPosLocation = glGetUniformLocation(m_objCurrentShader, "viewPos");

		// scale only center object.
		if (count == 0)
		{
			objModel = glm::scale(objModel, glm::vec3(add_scale));
		}

		glUniformMatrix4fv(mLocation, 1, GL_FALSE, glm::value_ptr(objModel));
		glUniformMatrix4fv(vLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(pLocation, 1, GL_FALSE, glm::value_ptr(proj));

		glm::vec3 camPos = m_camera->GetPosition();
		glUniform3f(viewPosLocation, camPos.x, camPos.y, camPos.z);

		// Object uniforms
		GLuint objColor = glGetUniformLocation(m_objCurrentShader, "objColor");
		glUniform3f(objColor, color.x, color.y, color.z);

		std::string lightName[] = {"pointLights", "dirLights", "spotLights"};
		int dir = 0, pt = 0, spt = 0;
		for (int i = 0; i < numLights; ++i)
		{

			if (lightTypes[i] == LightType::L_DIR)
			{
				std::string dirLightIndex = "[" + std::to_string(dir) + "]";
				std::string dirLightInfo[] = { ".direction", ".ambient", ".diffuse", ".spec", ".color" };

				std::string dirLightLocations[] = {
					lightName[LightType::L_DIR] + dirLightIndex + dirLightInfo[0],
					lightName[LightType::L_DIR] + dirLightIndex + dirLightInfo[1],
					lightName[LightType::L_DIR] + dirLightIndex + dirLightInfo[2],
					lightName[LightType::L_DIR] + dirLightIndex + dirLightInfo[3],
					lightName[LightType::L_DIR] + dirLightIndex + dirLightInfo[4]
				};

				GLuint dirLightDirection = glGetUniformLocation(m_objCurrentShader, dirLightLocations[0].c_str());
				GLuint dirAmbientLocation = glGetUniformLocation(m_objCurrentShader, dirLightLocations[1].c_str());
				GLuint dirDiffuseLocation = glGetUniformLocation(m_objCurrentShader, dirLightLocations[2].c_str());
				GLuint dirSpecLocation = glGetUniformLocation(m_objCurrentShader, dirLightLocations[3].c_str());
				GLuint dirLightColor = glGetUniformLocation(m_objCurrentShader, dirLightLocations[4].c_str());

				glUniform3f(dirLightDirection,
					-lightPosVector[i].x,
					-lightPosVector[i].y,
					-lightPosVector[i].z);

				glUniform3f(dirAmbientLocation, 0.05f, 0.05f, 0.05f);
				glUniform3f(dirDiffuseLocation, 0.8f, 0.8f, 0.8f);
				glUniform3f(dirSpecLocation, 1.0f, 1.0f, 1.0f);
				glUniform3f(dirLightColor, lightColorVector[i].x, lightColorVector[i].y, lightColorVector[i].z);

				dir++;
			}
			else if (lightTypes[i] == LightType::L_POINT)
			{
				std::string pointLightIndex = "[" + std::to_string(pt) + "]";
				std::string pointLightInfo[] = { 
					".position", 
					".ambient", 
					".diffuse", 
					".spec", 
					".color", 
					".constant", 
					".linear", 
					".quadratic" 
				};

				std::string pointLightLocations[] = {
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[0],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[1],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[2],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[3],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[4],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[5],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[6],
					lightName[LightType::L_POINT] + pointLightIndex + pointLightInfo[7],
				};

				GLuint pointLightPosition   = glGetUniformLocation(m_objCurrentShader, pointLightLocations[0].c_str());
				GLuint pointAmbientLocation = glGetUniformLocation(m_objCurrentShader, pointLightLocations[1].c_str());
				GLuint pointDiffuseLocation = glGetUniformLocation(m_objCurrentShader, pointLightLocations[2].c_str());
				GLuint pointSpecLocation    = glGetUniformLocation(m_objCurrentShader, pointLightLocations[3].c_str());
				GLuint pointLightColor      = glGetUniformLocation(m_objCurrentShader, pointLightLocations[4].c_str());
				GLuint pointLightConstant   = glGetUniformLocation(m_objCurrentShader, pointLightLocations[5].c_str());
				GLuint pointLightLinear     = glGetUniformLocation(m_objCurrentShader, pointLightLocations[6].c_str());
				GLuint pointLightQuadratic  = glGetUniformLocation(m_objCurrentShader, pointLightLocations[7].c_str());

				glUniform3f(pointLightPosition,
					lightPosVector[i].x,
					lightPosVector[i].y,
					lightPosVector[i].z);
				
				glUniform3f(pointAmbientLocation, 0.05f, 0.05f, 0.05f);
				glUniform3f(pointDiffuseLocation, 0.8f, 0.8f, 0.8f);
				glUniform3f(pointSpecLocation, 1.0f, 1.0f, 1.0f);
				
				glUniform3f(pointLightColor, lightColorVector[i].x, lightColorVector[i].y, lightColorVector[i].z);
				glUniform1f(pointLightConstant, 1.0f);
				glUniform1f(pointLightLinear, 0.09f);
				glUniform1f(pointLightQuadratic, 0.032f);

				pt++;
			}
			else if (lightTypes[i] == LightType::L_SPOT)
			{
				std::string spotLightIndex = "[" + std::to_string(spt) + "]";
				std::string spotLightInfo[] = { 
					".position", 
					".direction", 
					".ambient", 
					".diffuse", 
					".spec", 
					".constant",
					".linear",
					".quadratic",
					".color", 
					".cutOff", 
					".outerCutOff",
					".fallOff"
				};

				std::string spotLightLocations[] = {
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[0],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[1],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[2],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[3],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[4],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[5],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[6],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[7],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[8],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[9],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[10],
					lightName[LightType::L_SPOT] + spotLightIndex + spotLightInfo[11]
				};

				GLuint spotLightPosition    = glGetUniformLocation(m_objCurrentShader, spotLightLocations[0].c_str());
				GLuint spotLightDirection	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[1].c_str());
				GLuint spotAmbientLocation	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[2].c_str());
				GLuint spotDiffuseLocation	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[3].c_str());
				GLuint spotSpecLocation		= glGetUniformLocation(m_objCurrentShader, spotLightLocations[4].c_str());
				GLuint spotConstantLocation	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[5].c_str());
				GLuint spotLinearLocation	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[6].c_str());
				GLuint spotQuadratLocation	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[7].c_str());
				GLuint spotLightColor		= glGetUniformLocation(m_objCurrentShader, spotLightLocations[8].c_str());
				GLuint spotLightCutOff		= glGetUniformLocation(m_objCurrentShader, spotLightLocations[9].c_str());
				GLuint spotLightOuterCutOff	= glGetUniformLocation(m_objCurrentShader, spotLightLocations[10].c_str());
				GLuint spotLightfallOff     = glGetUniformLocation(m_objCurrentShader, spotLightLocations[11].c_str());

				glUniform3f(spotLightDirection,
					  -lightPosVector[i].x,
					  -lightPosVector[i].y,
					  -lightPosVector[i].z);
				glUniform3f(spotLightPosition,
					lightPosVector[i].x,
					lightPosVector[i].y,
					lightPosVector[i].z);

				glUniform3f(spotAmbientLocation, 0.05f, 0.05f, 0.05f);
				glUniform3f(spotDiffuseLocation, 0.8f, 0.8f, 0.8f);
				glUniform3f(spotSpecLocation, 1.0f, 1.0f, 1.0f);

				glUniform3f(spotLightColor, lightColorVector[i].x, lightColorVector[i].y, lightColorVector[i].z);
				glUniform1f(spotLightCutOff, glm::cos(glm::radians(cutOff[spt])));
				glUniform1f(spotLightOuterCutOff,glm::cos(glm::radians(cutOff[spt] + outerCutOff[spt])));
				glUniform1f(spotConstantLocation, 1.0f);
				glUniform1f(spotLinearLocation, 0.09f);
				glUniform1f(spotQuadratLocation, 0.032f);
				glUniform1f(spotLightfallOff, fallOff[spt]);
			
				spt++;
			}
	
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[count].vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[count].pureVn);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		if (dataChanged == true)
		{
			SetupUV(it->GetOBJ(), m_buffers[count]);
			dataChanged = false;
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[count].uv);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		

		if (m_showObj == true)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[count].idx);
			glDrawElements(GL_TRIANGLES, it->GetOBJ()->indexCount * 3, GL_UNSIGNED_INT, 0);
		}

		/////////////////////////////////////////////////////////////////////////
		//                                                                     //
		//                                                                     //
		//                     Environment mapping using fbo                   //                
		//                                                                     //
		//                                                                     //
		/////////////////////////////////////////////////////////////////////////
		if (isReflect == true || isRefract == true)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			glUseProgram(m_phongShadingID);

			GLuint ratioLocation = glGetUniformLocation(m_phongShadingID, "inputRatio");
			GLuint mixRatioLocation = glGetUniformLocation(m_phongShadingID, "mixRatio");
			GLuint fresnelLocation = glGetUniformLocation(m_phongShadingID, "fresnel");

			glUniform1f(ratioLocation, ratio);
			glUniform1f(mixRatioLocation, mixRatio);
			glUniform1f(fresnelLocation, fresnel);

			for (int i = 0; i < 6; ++i)
			{
				glActiveTexture(GL_TEXTURE10 + i);
				glBindTexture(GL_TEXTURE_2D, m_fbTexture[i]);
				glUniform1i(cubeMapsID[i], 10 + i);
			}
				
			for (int i = 0; i < 6; ++i)
			{		
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbTexture[i], 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glm::mat4 envProj = glm::perspective
				(
					glm::radians(90.0f),
					1.0f,
					0.1f,
					100.0f
				);
				glm::mat4 envView = m_fbCamera[i]->GetViewMatrix();
				glm::mat4 envModel(1.0f);

				GLuint envMLocation = glGetUniformLocation(m_phongShadingID, "model");
				GLuint envVLocation = glGetUniformLocation(m_phongShadingID, "view");
				GLuint envPLocation = glGetUniformLocation(m_phongShadingID, "proj");
				
				glUniformMatrix4fv(envMLocation, 1, GL_FALSE, glm::value_ptr(envModel));
				glUniformMatrix4fv(envVLocation, 1, GL_FALSE, glm::value_ptr(envView));
				glUniformMatrix4fv(envPLocation, 1, GL_FALSE, glm::value_ptr(envProj));

				RenderScene(dt, numLights, rot, orbitSize, sphScale,
				lightColorVector, isRot, rotSpeed, numDir, numSpot, numPoint,
				globalAmbient, isReflect, isRefract, shading, add_scale, lightTypes,
				cutOff, outerCutOff, fallOff, dataChanged, envProj, envView);

			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		}

		/////////////////////////////////////////////////////////////////////////
		//                                                                     //
		//                                                                     //
		//                        Normal Rendering                             //               
		//                                                                     //
		//                                                                     //
		/////////////////////////////////////////////////////////////////////////
		glUseProgram(m_normalProgramID);
		OBJ* obj = it->GetOBJ();
		// Normal uniforms
		glUniformMatrix4fv(normalMLocation, 1, GL_FALSE, glm::value_ptr(objModel));
		glUniformMatrix4fv(normalVLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(normalPLocation, 1, GL_FALSE, glm::value_ptr(proj));
		glEnableVertexAttribArray(0);
		glUniform3f(normColorLocation, 0.0f, 1.0f, 0.0f);

		if (m_showVn == true)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers[count].vno);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glDrawArrays(GL_LINES, 0, (GLsizei)obj->vertexNormal.size());
		}

		if (m_showFn == true)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers[count].fno);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glDrawArrays(GL_LINES, 0, (GLsizei)obj->faceNormal.size());
		}
		count++;

	}

	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                        Orbit  Rendering                             //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////
	glm::mat4 otransform = glm::mat4(1.0f);
	glm::mat4 oscaler    = glm::mat4(1.0f);
	glm::mat4 orotation  = glm::mat4(1.0f);

	glm::vec3 oscaling = glm::vec3(orbitSize);
	oscaler    = glm::scale(oscaler, oscaling);
	orotation  = glm::rotate(oscaler, glm::radians(90.f), glm::vec3(1.0f, 0.0f, 0.0f));
	otransform = glm::translate(orotation, glm::vec3(0.0f, 0.0f, 0.0f));

	glm::mat4 omodel = otransform;

	glUniform3f(normColorLocation, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(normalMLocation, 1, GL_FALSE, glm::value_ptr(omodel));
	glUniformMatrix4fv(normalVLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(normalPLocation, 1, GL_FALSE, glm::value_ptr(proj));

	glBindBuffer(GL_ARRAY_BUFFER, m_orbVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glDrawArrays(GL_LINES, 0, (GLsizei)m_orbVertSize);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void Graphics::Shutdown(void)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// TODO: Delete buffers according to number of objects.
	glDeleteBuffers(1, &m_buffers->vbo);
	glDeleteBuffers(1, &m_buffers->vno);
	glDeleteBuffers(1, &m_buffers->pureVn);
	glDeleteBuffers(1, &m_buffers->fno);
	glDeleteBuffers(1, &m_buffers->cbf);
	glDeleteBuffers(1, &m_buffers->idx);
	glDeleteBuffers(1, &m_buffers->uv);
	glDeleteBuffers(1, &m_sphVbo);
	glDeleteBuffers(1, &m_sphIbo);
	glDeleteBuffers(1, &m_sphVn);
	glDeleteBuffers(1, &m_orbVbo);
	glDeleteVertexArrays(1, &m_vao);
	DeleteProgram(m_phongShadingID);
	DeleteProgram(m_phongLightingID);
	DeleteProgram(m_blinShadingID);
	DeleteProgram(m_normalProgramID);
	delete m_camera;
	for (int i = 0; i < 6; ++i) {
		delete m_fbCamera[i];
	}
		
	delete m_spheres;
	glfwTerminate();
}

// Private methods

void Graphics::SelectMode(void)
{
	if (m_isFill == true) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }	
	else { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
		

	if (m_showFn == true) {}
	else {}

	if (m_showVn == true) {}
	else {}
	
}

void Graphics::SetupObjBuffers(OBJ* obj, ObjectBuffer& buffer)
{
	size_t verticesSize = obj->vertices.size() * sizeof(glm::vec3);
	// vertex buffer object
	glGenBuffers(1, &buffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, &obj->vertices[0], GL_STATIC_DRAW);

	size_t indicesSize = obj->indices.size() * sizeof(glm::ivec3);
	// index buffer object
	glGenBuffers(1, &buffer.idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, &obj->indices[0], GL_STATIC_DRAW);

	size_t vnSize = obj->vertexNormal.size() * sizeof(glm::vec3);
	// vertex normal object.
	glGenBuffers(1, &buffer.vno);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vno);
	glBufferData(GL_ARRAY_BUFFER, vnSize, &obj->vertexNormal[0], GL_DYNAMIC_DRAW);

	size_t fnSize = obj->faceNormal.size() * sizeof(glm::vec3);
	// face normal object
	glGenBuffers(1, &buffer.fno);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.fno);
	glBufferData(GL_ARRAY_BUFFER, fnSize, &obj->faceNormal[0], GL_DYNAMIC_DRAW);

	size_t pvnSize = obj->pureVertexNormal.size() * sizeof(glm::vec3);
	glGenBuffers(1, &buffer.pureVn);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.pureVn);
	glBufferData(GL_ARRAY_BUFFER, pvnSize, &obj->pureVertexNormal[0], GL_STATIC_DRAW);

}

void Graphics::SetupUV(OBJ* obj, ObjectBuffer& buffer)
{
	size_t uvSize = obj->vertices.size() * sizeof(glm::vec2);
	glGenBuffers(1, &buffer.uv);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.uv);

	std::vector<glm::vec2> uvVector;
	if (m_entityPos == true)
	{
		uvVector = CalcUv(obj->vertices);
	}
	else if (m_entityNormal == true)
	{
		uvVector = CalcUv(obj->pureVertexNormal);
	}

	glBufferData(GL_ARRAY_BUFFER, uvSize, uvVector.data(), GL_STATIC_DRAW);
}

void Graphics::CompileShaders(void)
{
	m_phongShadingID  = LoadNCompileShader("shader/phong_shading/vertex.vert", "shader/phong_shading/fragment.frag");
	m_phongLightingID = LoadNCompileShader("shader/phong_lighting/vertex.vert", "shader/phong_lighting/fragment.frag");
	m_blinShadingID   = LoadNCompileShader("shader/blin_shading/vertex.vert", "shader/blin_shading/fragment.frag");
	m_normalProgramID = LoadNCompileShader("shader/normalVertex.vert", "shader/normalFrag.frag");
	m_skyboxProgramID = LoadNCompileShader("shader/cubemap.vert", "shader/cubemap.frag");
}
std::vector<glm::vec2> Graphics::CalcUv(const std::vector<glm::vec3>& entity)
{
	std::vector<glm::vec2> result;
	for (auto& it : entity)
	{
		glm::vec3 normEntity = glm::normalize(it);
		glm::vec3 absVec = glm::abs(normEntity);
		glm::vec2 uv = glm::vec2(0.0);
		if (m_isPlanarMap == true)
		{
			// +-X
			if (absVec.x >= absVec.y && absVec.x >= absVec.z)
			{
				(normEntity.x < 0.0) ? (uv.x = normEntity.z) : (uv.x = -normEntity.z);
				uv.y = normEntity.y;
			}

			// +-Y
			else if (absVec.y >= absVec.x && absVec.y >= absVec.z)
			{
				(normEntity.y < 0.0) ? (uv.y = normEntity.z) : (uv.y = -normEntity.z);
				uv.x = normEntity.x;
			}

			// +-Z
			else if (absVec.z >= absVec.y && absVec.z >= absVec.x)
			{
				(normEntity.z < 0.0) ? (uv.x = -normEntity.x) : (uv.x = normEntity.x);
				uv.y = normEntity.y;
			}

			result.push_back((uv + glm::vec2(1.0)) / 2.f);
		}
		else if(m_isCylMap == true)
		{
			float theta = atan(normEntity.y / normEntity.x);
			float z = (normEntity.z - (-1.0f)) / 2.0f;
			uv = glm::vec2(theta / glm::radians(360.f), z);
			
			result.push_back((uv + glm::vec2(1.0)) / 2.f);
		}
		else if (m_isSphMap == true)
		{
			float theta = atan(normEntity.y / normEntity.x);
			float r = sqrt(normEntity.x * normEntity.x + normEntity.y * normEntity.y + normEntity.z * normEntity.z);
			float phi = acos(normEntity.z / r);

			uv = glm::vec2(theta / glm::radians(360.f), (glm::radians(180.f) - phi) / glm::radians(180.f));
			result.push_back((uv + glm::vec2(1.0)) / 2.f);
		}
	}

	return result;
}

void Graphics::RenderScene(float dt, int numLights, float rot, float orbitSize, float sphScale,
	glm::vec3 lightColorVector[MAX_LIGHTS], bool isRot, float rotSpeed, int numDir, int numSpot, int numPoint,
	glm::vec3& globalAmbient, bool isReflect, bool isRefract, bool mix, float add_scale, LightType lightTypes[MAX_LIGHTS],
	float cutOff[MAX_LIGHTS], float outerCutOff[MAX_LIGHTS], float fallOff[MAX_LIGHTS], bool& dataChanged,
	glm::mat4& proj, glm::mat4& view) 
{
	//glm::mat4 proj = glm::perspective(
	//	glm::radians(100.0f),
	//	(float)m_width / (float)m_height,
	//	m_zNear,
	//	100.0f
	//);
	//glm::mat4 view = m_camera->GetViewMatrix();

	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                         Skybox rendering                            //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glUseProgram(m_skyboxProgramID);
	// remove translation from the view matrix
	glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
	
	GLuint skyboxViewLocation = glGetUniformLocation(m_skyboxProgramID, "view");
	GLuint skyboxProjectionLocation = glGetUniformLocation(m_skyboxProgramID, "projection");
	glUniformMatrix4fv(skyboxViewLocation, 1, GL_FALSE, glm::value_ptr(skyboxView));
	glUniformMatrix4fv(skyboxProjectionLocation, 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(m_skyBoxVao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	for (int i = 0; i < 6; ++i)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxVbo[i]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_skyBoxUV);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_cubeTexture[i]);

		glUniform1i(glGetUniformLocation(m_skyboxProgramID, "skybox"), 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skyBoxidx);
		glDrawElements(GL_TRIANGLES, skyboxOBJ->indexCount * 3, GL_UNSIGNED_INT, 0);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	/////////////////////////////////////////////////////////////////////////
	//                                                                     //
	//                                                                     //
	//                        Sphere(Light) Rendering                      //               
	//                                                                     //
	//                                                                     //
	/////////////////////////////////////////////////////////////////////////
	glUseProgram(m_normalProgramID);
	m_camera->Control(0.000f, dt);

	GLuint normColorLocation = glGetUniformLocation(m_normalProgramID, "normColor");
	GLuint normalMLocation = glGetUniformLocation(m_normalProgramID, "model");
	GLuint normalVLocation = glGetUniformLocation(m_normalProgramID, "view");
	GLuint normalPLocation = glGetUniformLocation(m_normalProgramID, "proj");
	
	glm::vec3 lightPosVector[MAX_LIGHTS];
	for (int i = 0; i < numLights; ++i)
	{
		float angle = i * (glm::radians(360.f / MAX_LIGHTS)) + rot;

		m_spheres->SetPosition(glm::vec3(cosf(angle) * orbitSize, 0.0f, sinf(angle) * orbitSize));
		glm::mat4 sphModel = m_spheres->GetObjectMatrix();

		// for imgui
		sphModel = glm::scale(sphModel, glm::vec3(sphScale));
		glUniform3f(normColorLocation, lightColorVector[i].x, lightColorVector[i].y, lightColorVector[i].z);

		glUniformMatrix4fv(normalMLocation, 1, GL_FALSE, glm::value_ptr(sphModel));
		glUniformMatrix4fv(normalVLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(normalPLocation, 1, GL_FALSE, glm::value_ptr(proj));

		lightPosVector[i] = m_spheres->GetPosition();

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_sphVbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_sphVn);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphIbo);
		glDrawElements(GL_TRIANGLES, m_spheres->GetOBJ()->indexCount * 3, GL_UNSIGNED_INT, 0);
	}
	if (isRot == true)
	{
		rot += dt * rotSpeed;
	}

}
