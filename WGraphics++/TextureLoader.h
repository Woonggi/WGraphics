/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjectManager.h
Purpose: Function and struct definition for texture loading.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#ifndef _TEXTURE_LOADER_H
#define _TEXTURE_LOADER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Texture 
{
	std::vector<glm::vec3> rgb;
	int width;
	int height;
};

Texture* LoadTexture(const std::string& filename);
unsigned int LoadCubeMap(const std::string& face);

#endif