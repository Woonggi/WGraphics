/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: normalVertex.shader
Purpose: Vertex shader for normal debug drawing.
Language: GLSL, Visual studio 2019
Platform: openGL 4.3, GLSL, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#version 430 core

layout(location = 0) in vec3 modelPosition;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	mat4 mvp = proj * view * model;
	gl_Position = mvp * vec4(modelPosition, 1.0);
}