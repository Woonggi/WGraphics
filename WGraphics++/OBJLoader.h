/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJLoader.h
Purpose: Header for OBJLoader.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct OBJ
{
	// Default constructor
	OBJ(void) 
	: vertices(0), indices(0),
	  vertexCount(0), indexCount(0),
	  max(glm::vec3(1.0f)),
	  midPoint(glm::vec3(0.0f))

	{}
	// Constructor passing vertex and index vector.
	OBJ(std::vector<glm::vec3>& vtx, std::vector<glm::ivec3>& idx,
		std::vector<glm::vec3>& vn, std::vector<glm::vec3>& fn)
	: vertices(vtx), indices(idx),
	  vertexNormal(vn), faceNormal(fn),
	  vertexCount(0), indexCount(0),
	  max(glm::vec3(1.0f)),
	  midPoint(glm::vec3(0.0f))

	{}
	std::vector<glm::vec3>        vertices;
	std::vector<glm::ivec3>       indices;	
	std::vector<glm::vec3>		  vertexNormal;
	std::vector<glm::vec3>		  faceNormal;
	std::vector<glm::vec3>		  pureVertexNormal;
	std::vector<glm::vec3>		  pureFaceNormal;
	glm::vec3                     max;
	glm::vec3					  midPoint;
	unsigned int vertexCount;
	unsigned int indexCount;
};

// deprecated, using fscanf
//OBJ* LoadOBJFromFile(const std::string& filename);
OBJ* LoadOBJFromFile(const std::string& filename);
OBJ* LoadSphere(int LOD, float radius);
#endif