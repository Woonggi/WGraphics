/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Shader.h
Purpose: Header for Shader.h
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/


#ifndef SHADER_H
#define SHADER_H

void   PrintErrorMessages (GLuint id, const std::string& type);
GLuint LoadNCompileShader (const char* vsFilePath, const char* fsFilePath);
void   DeleteProgram      (GLuint id);
void   Use                (GLuint id);

// Setting values in shader.
//void setFloat3     (const char* name, float val1, float val2, float val3);
//void setMat4f      (const char* name, float* val);


#endif