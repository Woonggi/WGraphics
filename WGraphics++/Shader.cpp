/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Shader.cpp
Purpose: Load and compile shader from shader file.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/


#include <string>  // string
#include <fstream> // file input
#include <sstream> // stringstream
#include <iostream>// cout 
#include <vector>  // vector

#include <GL/glew.h> // opengl functions
#include "Shader.h"  // function/class declaration

void Use(GLuint id)
{
	glUseProgram(id);
}

GLuint LoadNCompileShader(const char* vsFilePath, const char* fsFilePath)
{
	// Create shaders.
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShaderID   = glCreateShader(GL_FRAGMENT_SHADER);

	// Read vertex shader file from file.
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vsFilePath);
	// if file successfully opend,
	if (vertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << vertexShaderStream.rdbuf();
		vertexShaderCode = sstr.str();
		vertexShaderStream.close();
	}
	// Otherwise, abort.
	else
	{
		std::cout << "cannot read shader file " 
			<< vsFilePath << std::endl;

		return false;
	}

	// Read fragment shader file from file.
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderStream(fsFilePath);
	if (fragmentShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << fragmentShaderStream.rdbuf();
		fragmentShaderCode = sstr.str();
		fragmentShaderStream.close();
	}
	else
	{
		std::cout << "Cannot read shader file "
			<< fsFilePath << std::endl;

		return false;
	}

	std::cout << "-------------------------------------------" << std::endl;
	std::cout << "Vertex shader : " << vsFilePath << std::endl;
	std::cout << "Fragment shader : " << fsFilePath << std::endl;
	// Compile vertex shader
	std::cout << "Compiling vertex shader..." << std::endl;
	const char* vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
	glCompileShader(vertexShaderID);
	PrintErrorMessages(vertexShaderID, "Shader");

	// Compile fragment shader
	std::cout << "Compiling fragment shader..." << std::endl;
	const char* fragSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragShaderID, 1, &fragSourcePointer, NULL);
	glCompileShader(fragShaderID);	
	PrintErrorMessages(fragShaderID, "Shader");

	// Link to program
	std::cout << "Linking program..." << std::endl;
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragShaderID);
	glLinkProgram(programID);
	PrintErrorMessages(programID, "Program");

	std::cout << "-------------------------------------------" << std::endl;

	// version check
	char* glVersion = {};
	glVersion = (char*)glGetString(GL_VERSION);
	std::cout << "openGL version :  " << glVersion << std::endl;

	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragShaderID);

	return programID;
}

void DeleteProgram(GLuint id)
{
	glDeleteProgram(id);
}

void PrintErrorMessages(GLuint id, const std::string& type)
{
	GLint result = GL_FALSE;
	int infoLogLength;

	if (type == "Shader")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);
			std::cout << &shaderErrorMessage[0] << std::endl;
		}
	}
	else if (type == "Program")
	{
		glGetProgramiv(id, GL_COMPILE_STATUS, &infoLogLength);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(id, infoLogLength, NULL, &programErrorMessage[0]);
			std::cout << &programErrorMessage[0] << std::endl;
		}

	}
	else { 
		return; 
	}

}

