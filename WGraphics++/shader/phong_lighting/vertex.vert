/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: vertex.shader
Purpose: Vertex shader for phong lighting
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

uniform mat4 rotationMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 objColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 g_ambient;
uniform vec3 fogColor;
uniform float zNear;
uniform float zFar;


// diff
uniform sampler2D tex1;
// spec
uniform sampler2D tex2;

struct Material
{
	vec3 rgb;
	vec3 kd;
	vec3 ks;
};

struct DirLight
{
	vec3 direction;
	vec3 ambient;	
	vec3 diffuse;
	vec3 spec;

	vec3 color;
};

struct PointLight
{	
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 spec;

	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight
{
	vec3 position;	
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 spec;

	float constant;
	float linear;
	float quadratic;

	vec3 color;
	float cutOff;
	float outerCutOff;
	float fallOff;
};

uniform int numDir;
uniform int numPoint;
uniform int numSpot;

uniform DirLight dirLights[16];
uniform PointLight pointLights[16];
uniform SpotLight spotLights[16];
uniform int mapType;
uniform int entity_mode;
uniform int isGpu;

out vec3 outColor;

vec3 MyReflect(vec3 dir ,vec3 norm)
{
	vec3 result = 2 * dot(dir, norm) * norm - dir;
	return result;
}

vec2 CalcPlanarMap(vec3 vEntity)
{
	vec3 absVec = abs(vEntity);
	vec2 uv = vec2(0.0);

	// +-X
	if(absVec.x >= absVec.y && absVec.x >= absVec.z)
	{
		(vEntity.x < 0.0) ? (uv.x = vEntity.z) : (uv.x = -vEntity.z);
		uv.y = vEntity.y;
	}

	// +-Y
	else if(absVec.y >= absVec.x && absVec.y >= absVec.z)
	{
		(vEntity.y < 0.0) ? (uv.y = vEntity.z) : (uv.y = -vEntity.z);
		uv.x = vEntity.x;
	}

	// +-Z
	else if(absVec.z >= absVec.y && absVec.z >= absVec.x)
	{
		(vEntity.z < 0.0) ? (uv.x = -vEntity.x) : (uv.x = vEntity.x);
		uv.y = vEntity.y;
	}

	return (uv + vec2(1.0)) * 0.5;
}

vec2 CalcCyilindricalMap(vec3 vEntity)
{
	float theta = atan(vEntity.y / vEntity.x);
	float z = (vEntity.z - (-1.0)) / 2.0;
	vec2 uv = vec2(theta / radians(360.f), z);
	return (uv + vec2(1.0)) * 0.5;
}

vec2 CalcSphericalMap(vec3 vEntity)
{
	float theta = atan(vEntity.y / vEntity.x);
	float r = sqrt(vEntity.x * vEntity.x + vEntity.y * vEntity.y + vEntity.z * vEntity.z);
	float phi = acos(vEntity.z / r);

	vec2 uv = vec2(theta / radians(360.f), (radians(180.f) - phi)/ radians(180.f) );
	return (uv + vec2(1.0)) * 0.5;
}



vec3 CalcDirLights(DirLight light, vec3 norm, vec3 viewDir, vec3 fragPos, Material material)
{
	// Directional
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(norm, lightDir), 0.0);

	vec3 reflectDir = MyReflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30.0f);

	vec3 ambient = light.ambient * light.color;
	vec3 diffuse = light.diffuse * diff * material.kd * light.color;
	vec3 specular = light.spec * spec * material.ks * light.color;

	vec3 result = (ambient + diffuse + specular);
	return result;
}

vec3 CalcPointLights(PointLight light, vec3 norm, vec3 viewDir, vec3 fragPos, Material material)
{
	vec3 ambient = light.ambient * light.color;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.kd * light.color;

	vec3 reflectDir = MyReflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30.0f);
	vec3 specular = light.spec * spec * material.ks * light.color;

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = (ambient + diffuse + specular);
	return result;

}

vec3 CalcSpotLights(SpotLight light, vec3 norm, vec3 viewDir, vec3 fragPos, Material material)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 ambient = light.ambient * light.color;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.kd * light.color;

	vec3 reflectDir = MyReflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30.0f);
	vec3 specular = light.spec * spec * material.ks * light.color;

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	intensity = pow(intensity, light.fallOff);
	diffuse *= intensity;
	specular *= intensity;

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = ambient + diffuse + specular;
	return result;
}


void main()
{
	mat4 mvp = proj * view * model;
	vec4 normal		= rotationMatrix * vec4(normalVector, 1.0);
	vec3 fragPos	= vec3(model * vec4(modelPosition, 1.0));
	gl_Position		= mvp * vec4(modelPosition, 1.0);
	vec3 local = vec3(0.0f, 0.0f, 0.0f);

	vec3 norm = normalize(normal.xyz);
	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 entity;
	vec2 uv;
	if (isGpu == 1)
	{
		if (entity_mode == 1)
		{
			entity = normalize(modelPosition);
		}
		else if (entity_mode == 2)
		{
			entity = normalize(norm);
		}

		// planar
		if (mapType == 1)
		{
			uv = CalcPlanarMap(entity);
		}
		// cylindrical
		else if (mapType == 2)
		{
			uv = CalcCyilindricalMap(entity);
		}
		// spherical
		else if (mapType == 3)
		{
			uv = CalcSphericalMap(entity);
		}
	}
	else
	{
		uv = cpuUV;
	}

	Material material;
	material.kd = texture(tex1, uv).rgb;
	material.ks = texture(tex2, uv).rgb;
	float S = (zFar - distance(viewPos, fragPos)) / zFar - zNear;

	for (int i = 0; i < numDir; ++i)
	{
		local += CalcDirLights(dirLights[i], norm, viewDir, fragPos, material);
	}

	for (int i = 0; i < numPoint; ++i)
	{
		local += CalcPointLights(pointLights[i], norm, viewDir, fragPos, material);
	}

	for (int i = 0; i < numSpot; ++i)
	{
		local += CalcSpotLights(spotLights[i], norm, viewDir, fragPos, material);
	}

	local += g_ambient;
	outColor = S * local + (1 - S) * fogColor;
}