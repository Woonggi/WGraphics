/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: TextureLoader.cpp
Purpose: For loading texture.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/
#include "TextureLoader.h"
#include <fstream>
#include <cstring>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb.image.h"
#include <GL/glew.h>

Texture* LoadTexture(const std::string& filename)
{
	std::ifstream tex(filename);
	std::string version;
	std::string creator;
	std::string size;
	std::string maxVal;

	Texture* result = new Texture();
	
	// Version 
	std::getline(tex, version);
	// creator(comment)
	std::getline(tex, creator);
	// size
	std::getline(tex, size);
	std::stringstream(size) >> result->width >> result->height;	
	// maximum value
	float max = 255;
	std::getline(tex, maxVal);
	std::stringstream(maxVal) >> max;

	int maxSize = result->width * result->height;
	result->rgb.reserve(50000);
	
	int index = 0;
	int i = 0;
	std::string line;
	while(std::getline(tex, line))
	{	
		std::stringstream ss(line);
		if (i % 3 == 0) {
			result->rgb.push_back(glm::vec3(0.0f));
			ss >> result->rgb[index].r;
			result->rgb[index].r /= max;
		}
		else if (i % 3 == 1) {
			ss >> result->rgb[index].g;
			result->rgb[index].g /= max;
		}
		else if (i % 3 == 2) {
			ss >> result->rgb[index].b;
			result->rgb[index].b /= max;
			++index;
		}
		++i;
	}

	// TODO : Deallocate memories.

	return result;
}

unsigned int LoadCubeMap(const std::string& face)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int width, height, nrChannels;

	unsigned char* data = stbi_load(face.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "cubemap texture failed to load" << std::endl;
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureID;
}