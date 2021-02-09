/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: OBJLoader.cpp
Purpose: Load and store data from .obj file for rendering.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#include "OBJLoader.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <algorithm>

glm::vec3 ComputeFaceNormal(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
glm::vec3 ComputeFaceCenter(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
glm::vec3 ComputeVertexNormal(glm::vec3& v, const glm::vec3& objPos);

OBJ* LoadOBJFromFile(const std::string& filename)
{
	std::string line;
	std::ifstream objFile(filename);
	std::vector<glm::vec3>  vtx;
	std::vector<glm::ivec3> idx;
	std::vector<glm::vec3>  vn;
	std::vector<glm::vec3>  fn;
	std::vector<glm::vec3>  pvn;
	std::vector<glm::vec3>  pfn;
	
	vtx.reserve(50000);
	idx.reserve(50000);
	vn.reserve(50000);
	fn.reserve(50000);
	pvn.reserve(50000);

	glm::vec3 max(0.0f), min(0.0f);
	glm::vec3 maxScale(0.f);
	bool vnLoaded = false;
	std::cout << "Loading " << filename << "..." << std::endl;
	while(!objFile.eof())
	{
		std::getline(objFile, line);
		std::stringstream ss;

		// vertex normal info
		if (line.find("vn") != std::string::npos)
		{
			// NOTIFICATION : I made loading vertex normal from the file, but 
			// I thought it would be better to calculate myself, so I just commentized it.
			//vnLoaded = true;
			//line.erase(0, 2);
			//ss << line;
			//float normal[3];
			//for (int i = 0; !ss.eof(); ++i)
			//{
			//	ss >> line;
			//	std::stringstream(line) >> normal[i];
			//	line = "";
			//}
			//glm::vec3 normals(normal[0], normal[1], normal[2]);	

			//// TODO : some vertex normals are not being loaded.
			//if (vIndex < vtx.size())
			//{
			//	glm::vec3 v = vtx[vIndex];
			//	vn.push_back(v);
			//	vn.push_back(normals + v);
			//	pvn.push_back(normals);
			//	++vIndex;
			//}
		}
		else if (line.find("vt") != std::string::npos)
		{
			// do nothing for now...
		}
		// vertex info
		else if (line[0] == 'v')
		{
			line.erase(0, 1);
			ss << line;
			float vertex[3];
			for(int i = 0; !ss.eof(); ++i)
			{
				ss >> line;
				std::stringstream(line) >> vertex[i]; // avoid letter 'v'
				line = "";
			}
			glm::vec3 vertices(vertex[0], vertex[1], vertex[2]);
			if (max.x < vertices.x) max.x = vertices.x;
			if (max.y < vertices.y) max.y = vertices.y;
			if (max.z < vertices.z) max.z = vertices.z;

			if (min.x > vertices.x) min.x = vertices.x;
			if (min.y > vertices.y) min.y = vertices.y;
			if (min.z > vertices.z) min.z = vertices.z;

			//if (maxScale.x < std::fabsf(vertices.x)) maxScale.x = std::fabsf(vertices.x);
			//if (maxScale.y < std::fabsf(vertices.y)) maxScale.y = std::fabsf(vertices.y);
			//if (maxScale.z < std::fabsf(vertices.z)) maxScale.z = std::fabsf(vertices.z);

			vtx.push_back(vertices);
			pvn.push_back(glm::vec3(0.f));
		}
		// index info
		if (line[0] == 'f')
		{
			// Remain only face index.
			line.erase(0, 1);
			ss << line;
			unsigned int putin;
			std::vector<unsigned int> indexVector;
			int count;
			for (count = 0; !ss.eof(); ++count)
			{
				ss >> line;
				std::stringstream(line) >> putin;
				indexVector.push_back(putin - 1);
				line = "";
			}
			glm::ivec3 indicies(indexVector[0], indexVector[1], indexVector[2]);
			idx.push_back(indicies);
			glm::vec3 calcFn = ComputeFaceNormal(vtx[indicies.x], 
												 vtx[indicies.y], 
												 vtx[indicies.z]);

			glm::vec3 calcFc = ComputeFaceCenter(vtx[indicies.x],
												 vtx[indicies.y],
											     vtx[indicies.z]);
			pfn.push_back(calcFn);
			fn.push_back((calcFn + calcFc));
			fn.push_back(calcFc);
		
			// If there is more than 3 index data... deal with it.
			if (count > 3)
			{
				unsigned int vertexCount = (unsigned int)vtx.size();
				for (int i = 1; i <= count - 3; ++i)
				{
					const unsigned int lastIndex = count - i;
					if (lastIndex < vertexCount)
					{
						glm::ivec3 restIndicies(indexVector[0],
							indexVector[lastIndex - 1],
							indexVector[lastIndex]);
						idx.push_back(restIndicies);
						calcFn = ComputeFaceNormal(vtx[restIndicies.x], 
												   vtx[restIndicies.y], 
												   vtx[restIndicies.z]);

						calcFc = ComputeFaceCenter(vtx[restIndicies.x],
												   vtx[restIndicies.y],
												   vtx[restIndicies.z]);

						pfn.push_back(calcFn);
						fn.push_back(calcFn + calcFc);
						fn.push_back(calcFc);
					}
				}
			}
		}
	}
	
	maxScale = glm::abs(max - min);
	float scaleConstant = glm::max(glm::max(maxScale.x, maxScale.y), maxScale.z);

	glm::vec3 midPoint((max.x + min.x) / 2.f, 
		               (max.y + min.y) / 2.f, 
					   (max.z + min.z) / 2.f);

	if (vnLoaded == false)
	{
		const unsigned int faceTotal = (unsigned int)idx.size();
		for (unsigned int i = 0; i < faceTotal; ++i)
		{
			// current face index.
			const glm::ivec3 currIndex = idx[i];
			pvn[idx[i].x] += pfn[i];
			pvn[idx[i].y] += pfn[i];
			pvn[idx[i].z] += pfn[i];
			fn[i * 2] = fn[(i * 2) + 1] + (pfn[i] * scaleConstant * 0.1f);
		}
		const unsigned int vertexNormal = (unsigned int)pvn.size();
		for (unsigned int i = 0; i < vertexNormal; ++i)
		{
			glm::vec3 currVertex = vtx[i];
			pvn[i] = glm::normalize(pvn[i]);
			vn.push_back(currVertex + (pvn[i] * scaleConstant * 0.1f));
			vn.push_back(currVertex);
		}
	}
	for (auto& it : vtx)
	{
		it -= midPoint;
		it /= scaleConstant;
	}

	for (auto& it : vn)
	{
		it -= midPoint;
		it /= scaleConstant;
	}
	for (auto& it : fn)
	{
		it -= midPoint;
		it /= scaleConstant;
	}

	std::cout << "LOADED" << std::endl;
	OBJ * result = new OBJ(vtx, idx, vn, fn);
	result->pureVertexNormal = pvn;
	result->pureFaceNormal = pfn;
	result->vertexCount = (unsigned int)vtx.size();
	result->indexCount = (unsigned int)idx.size();
	result->max = maxScale; 
	result->midPoint = midPoint;

	return result;
}

OBJ* LoadSphere(int LOD, float radius)
{
	const float PI = 3.141592f;
	std::vector<glm::vec3> vtx;
	std::vector<glm::vec3> vn;

	glm::vec3 v;
	glm::vec3 n;
	
	float hStep = 2 * PI / LOD;
	float vStep = PI / LOD;
	float stackAngle, sectorAngle;

	for (int i = 0; i <= LOD; ++i)
	{
		stackAngle = PI / 2 - i * vStep;
		float xy = radius * cosf(stackAngle);
		v.z = radius * sinf(stackAngle);

		for (int j = 0; j <= LOD; ++j)
		{
			sectorAngle = j * hStep;

			v.x = xy * cosf(sectorAngle);
			v.y = xy * sinf(sectorAngle);
			vtx.push_back(v);
			vn.push_back(glm::normalize(v));
		}
	}

	std::vector<glm::ivec3> idx;
	int k1, k2;
	for(int i = 0; i < LOD; ++i)
	{
		k1 = i * (LOD + 1);
		k2 = k1 + LOD + 1;
		for (int j = 0; j < LOD; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				idx.push_back(glm::ivec3(k1, k2, k1 + 1));
			}

			if (i != (LOD - 1))
			{
				idx.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
			}
		}
	}

	OBJ* sphere = new OBJ();
	sphere->vertices = vtx;
	sphere->indices = idx;
	sphere->pureVertexNormal = vn;
	sphere->vertexCount = (unsigned int)vtx.size();
	sphere->indexCount = (unsigned int)idx.size();
	return sphere;
}

glm::vec3 ComputeFaceNormal(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
{
	glm::vec3 v1 = p2 - p1;
	glm::vec3 v2 = p3 - p1;
	glm::vec3 result = glm::cross(v1, v2);
	return glm::normalize(result);
}

glm::vec3 ComputeVertexNormal(glm::vec3& v, const glm::vec3& objPos)
{
	return glm::normalize(v - objPos);
}

glm::vec3 ComputeFaceCenter(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
{
	return glm::vec3((p1 + p2 + p3) / 3.f);
}