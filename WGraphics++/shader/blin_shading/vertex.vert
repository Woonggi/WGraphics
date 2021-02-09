/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: vertex.shader
Purpose: Vertex shader for blin shading.
Language: GLSL, Visual studio 2019
Platform: openGL 4.3, GLSL, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#version 430 core

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec3 normalVector;
layout(location = 2) in vec2 cpuUV;

out vec4 normal;
out vec3 fragPos;
out vec3 entity;
out vec2 uvs;
flat out int doCalc;

uniform vec3 lightPos;
uniform mat4 rotationMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform int entity_mode;
uniform int isGpu;

void main()
{
	mat4 mvp = proj * view * model;
	gl_Position = mvp * vec4(modelPosition, 1.0);
	fragPos = vec3(model * vec4(modelPosition, 1.0));
	normal = rotationMatrix * vec4(normalVector, 1.0);

	if (isGpu == 1)
	{
		if (entity_mode == 1)
		{
			entity = normalize(modelPosition);
		}
		else if (entity_mode == 2)
		{
			entity = normalize(normal.xyz);
		}
	}
	else
	{
		uvs = cpuUV;
	}
	doCalc = isGpu;
}