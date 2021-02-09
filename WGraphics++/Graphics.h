/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Graphics.h
Purpose: Class declaration for Graphics.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <glfw3.h>
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;
struct OBJ;
struct Texture;
class Camera;
class Object;

const int MAX_LIGHTS = 16;

class Graphics
{
public:
	Graphics(int width, int height);
	~Graphics(void);

	bool Initialize	(void);
	void Update		(float dt);
	void Shutdown   (void);

private:
	struct ObjectBuffer;
	void SelectMode(void);
	void SetupObjBuffers(OBJ* obj, ObjectBuffer& buffer);
	void SetupUV(OBJ* obj, ObjectBuffer& buffer);
	void CompileShaders(void);

	enum LightType { L_EMPTY = -1, L_POINT = 0, L_DIR, L_SPOT };
	void RenderScene(float dt, int numLights, float rot, float orbitSize, float sphScale,
		glm::vec3 lightColorVector[MAX_LIGHTS], bool isRot, float rotSpeed, int numDir, int numSpot, int numPoint,
		glm::vec3& globalAmbient, bool isReflect, bool isRefract, bool mix, float add_scale, LightType lightTypes[MAX_LIGHTS],
		float cutOff[MAX_LIGHTS], float outerCutOff[MAX_LIGHTS], float fallOff[MAX_LIGHTS], bool& dataChanged,
		glm::mat4& proj, glm::mat4& view);

	std::vector<glm::vec2> CalcUv(const std::vector<glm::vec3>& vertices);

private:
	GLFWwindow* m_window;
	Camera*		m_camera;
	Camera*		m_fbCamera[6];
	int			m_width, m_height;	// width and height of the screen.
	float       m_zNear, m_zFar;

	// shader ids.
	GLuint      m_phongShadingID;	// id for phong shading program.
	GLuint		m_phongLightingID;  // id for phong lighting program.
	GLuint		m_blinShadingID;	// id for blin shading program.
	GLuint		m_normalProgramID;  // shader program for drawing normal.
	GLuint		m_skyboxProgramID;
	GLuint		m_objCurrentShader;

	GLuint      m_vao;				// vertex array object.

	struct ObjectBuffer 
	{
		GLuint      vbo    = 0;				// vertex buffer object.
		GLuint		vno    = 0;				// vertex normal object.
		GLuint		fno    = 0;				// face normal object;
		GLuint      cbf    = 0;				// color buffer.
		GLuint      idx	   = 0;				// index buffer.
		GLuint		pureVn = 0;			    // normal buffer for lighting
		GLuint		pureFn = 0;
		GLuint		uv     = 0; 
	};

	ObjectBuffer* m_buffers;

	GLuint		m_sphVbo;           // sphere vertex buffer
	GLuint		m_sphIbo;           // sphere indext buffer
	GLuint		m_sphVn;            // sphere vertex normal
	Object*     m_spheres;          // obj pointer to sphere.

	GLuint      m_orbVbo;
	size_t      m_orbVertSize;      // total vertcies of orbit.

	GLuint		m_texture1;
	GLuint		m_texture2;

	// For skybox
	typedef enum {FRONT = 0, RIGHT, BACK, LEFT, TOP, BOTTOM} SKYBOX;
	GLuint		m_skyBoxVbo[6];		// vertex buffer
	GLuint		m_skyBoxUV;
	GLuint		m_skyBoxVao;		// vertex array object
	GLuint		m_skyBoxidx;
	GLuint		m_cubeTexture[6];		// for texture 

	// For environment mapping 
	GLuint		m_fbo;
	GLuint		m_rbo;
	GLuint		m_fbTexture[6];

	// booleans for imgui control panel
	bool		m_isFill;           // determine drawing method.
	bool        m_showFn;           // show face normal
	bool        m_showVn;           // show vertex normal
	bool        m_showObj;          // show object
	bool		m_isPlanarMap  = true;
	bool		m_isCylMap     = false;
	bool		m_isSphMap     = false;
	bool		m_entityPos	   = true;
	bool		m_entityNormal = false;
	bool		m_isCPU		   = false;
	bool		m_isGPU		   = true;
};

#endif