/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: normalVertex.shader
Purpose: Vertex shader for cubemap rendering 
Language: GLSL, Visual studio 2019
Platform: openGL 4.3, GLSL, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/12/05
End Header --------------------------------------------------------*/

#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoords;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	texCoords = aTexCoord;
	mat4 new_view = view;

	int i;
	for(i = 0; i < 3; ++i){
		new_view[3][i] = 0.0;
	}

	vec4 pos = projection * new_view * vec4(aPos - 1, 1.0);
	gl_Position = pos.xyww;
}