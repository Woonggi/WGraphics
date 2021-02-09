/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: fragment.shader
Purpose: Fragment shader for phong shading
Language: GLSL, Visual studio 2019
Platform: openGL 4.3, GLSL, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#version 430 core

in vec4 normal;
in vec3 fragPos;
in vec3 entity;
in vec2 uvs;
flat in int doCalc;

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
// skybox
uniform sampler2D cube[6];

struct Material
{
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
uniform int mapType; 

uniform int isReflect;
uniform int isRefract;
uniform int isShading;
uniform float fresnel;

uniform float inputRatio;
uniform float mixRatio;

uniform DirLight dirLights[16];
uniform PointLight pointLights[16];
uniform SpotLight spotLights[16];

out vec4 fragColor;

int planeNum;

vec3 MyReflect(vec3 dir ,vec3 norm)
{
	vec3 result = 2 * dot(dir, norm) * norm - dir;
	return result;
}

vec3 MyRefract(vec3 I, vec3 N, float eta)
{
	vec3 R;
	float k = 1.0 - eta * eta * (1.0  - dot(N, I) * dot(N, I));
	if(k < 0.0)
		R = vec3(0.0);
	else
		R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;

	return R;
}

vec2 CalcEnvironmentUV(vec3 vEntity)
{	
	vec3 absVec = abs(vEntity);
	vec2 uv = vec2(0.0);
	float max;

	// +-X (3, 1)
	if(absVec.x >= absVec.y && absVec.x >= absVec.z)
	{	
		(vEntity.x < 0.0) ? (uv.x = vEntity.z) : (uv.x = -vEntity.z);
		uv.y = vEntity.y;
		if(vEntity.x < 0.0) {
			// right
			planeNum = 3;
		}
		else{
			// left
			planeNum = 1;
		}

		uv.x *= -1;
		max = absVec.x;
	}

	// +-Y (4, 5)
	else if(absVec.y >= absVec.x && absVec.y >= absVec.z)
	{
		(vEntity.y < 0.0) ? (uv.y = vEntity.z) : (uv.y = -vEntity.z);
		uv.x = vEntity.x;
		if(vEntity.y < 0.0) {
			// bottom
			planeNum = 5;
			uv.y *= -1;
		}
		else {
			// top
			planeNum = 4; 
			uv.y *= -1;
		}
		max = absVec.y;
	}

	// +-Z (0, 2)
	else if(absVec.z >= absVec.y && absVec.z >= absVec.x)
	{
		(vEntity.z < 0.0) ? (uv.x = -vEntity.x) : (uv.x = vEntity.x);
		uv.y = vEntity.y;
		if(vEntity.z < 0.0) {
			// front
			planeNum = 0;
		}
		else {
			// back
			planeNum = 2;
		}
		
		uv.x *= -1;
		max = absVec.z;
	}
	
	return ((uv / max)  + vec2(1.0)) * 0.5;
}

vec2 CalcPlanarMap(vec3 vEntity)
{
	vec3 absVec = abs(vEntity);
	vec2 uv = vec2(0.0);

	// +-X (3, 1)
	if(absVec.x >= absVec.y && absVec.x >= absVec.z)
	{	
		(vEntity.x < 0.0) ? (uv.x = vEntity.z) : (uv.x = -vEntity.z);
		uv.y = vEntity.y;
	}

	// +-Y (4, 5)
	else if(absVec.y >= absVec.x && absVec.y >= absVec.z)
	{
		(vEntity.y < 0.0) ? (uv.y = vEntity.z) : (uv.y = -vEntity.z);
		uv.x = vEntity.x;
	}

	// +-Z (0, 2)
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

vec3 CalcDirLights(DirLight light, vec3 norm, vec3 viewDir, Material material)
{
	// Directional
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(norm, lightDir), 0.0);

	vec3 reflectDir = MyReflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30.f);

	vec3 ambient = light.ambient * light.color;
	vec3 diffuse = light.diffuse * diff * material.kd * light.color;
	vec3 specular = light.spec * spec * material.ks * light.color;

	vec3 result = (ambient + diffuse + specular);
	return result;
}

vec3 CalcPointLights(PointLight light, vec3 norm, vec3 viewDir, Material material)
{
	vec3 ambient = light.ambient * light.color;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * material.kd * light.color;

	vec3 reflectDir = MyReflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30.f);
	vec3 specular = light.spec * spec * material.ks * light.color;

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = (ambient + diffuse + specular);
	return result;

}

vec3 CalcSpotLights(SpotLight light, vec3 norm, vec3 viewDir, Material material)
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
	specular *= intensity ;

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
	vec3 local = vec3(0.0);
	vec3 norm = normalize(normal.xyz);
	vec3 viewDir = normalize(viewPos - fragPos);
	
	float S = (zFar - distance(viewPos, fragPos)) / zFar - zNear;

	vec2 uv;
	vec3 color = vec3(0.0);
	vec3 reflectColor = vec3(0.0);
	vec3 refractColor = vec3(0.0);

	if(isReflect == 1 && isRefract == 0)
	{
		vec3 R = MyReflect(viewDir, norm);
		vec2 envUV = CalcEnvironmentUV(R);
		color = texture(cube[planeNum], envUV).rgb;
		// fragColor = vec4(texture(cube[planeNum], envUV).rgb, 1.0);
	}
	else if(isReflect == 0 && isRefract == 1)
	{
		float ratio = 1.00 / inputRatio; 
		vec3 R = MyRefract(-viewDir, norm, ratio);
		vec2 envUV = CalcEnvironmentUV(R);
		color = texture(cube[planeNum], envUV).rgb;
		// fragColor = vec4(texture(cube[planeNum], envUV).rgb, 1.0);
	}
	else if(isReflect == 1 && isRefract == 1)
	{
		vec3 R_reflect = MyReflect(viewDir, norm);
		vec2 uv_reflect = CalcEnvironmentUV(R_reflect);
		reflectColor = texture(cube[planeNum], uv_reflect).rgb;

		float ratio = 1.00 / inputRatio;
		vec3 R_refract[3]; 
		R_refract[0] = MyRefract(-viewDir, norm , ratio * fresnel * 1.1);
		R_refract[1] = MyRefract(-viewDir, norm , ratio * fresnel *  1.2);
		R_refract[2] = MyRefract(-viewDir, norm , ratio * fresnel * 1.3);

		vec2 uv_refract[3]; 
		uv_refract[0] = CalcEnvironmentUV(R_refract[0]);
		uv_refract[1] = CalcEnvironmentUV(R_refract[1]);
		uv_refract[2] = CalcEnvironmentUV(R_refract[2]);

		refractColor.r = texture(cube[planeNum], uv_refract[0]).r;
		refractColor.g = texture(cube[planeNum], uv_refract[1]).g;
		refractColor.b = texture(cube[planeNum], uv_refract[2]).b;

		color = mix(reflectColor, refractColor, mixRatio);
		
		// fragColor = vec4(mixColor, 1.0);
	}
	
	vec3 result = vec3(0.0);
	if(isShading == 1 || (isReflect == 0 && isRefract == 0))
	{
		if (doCalc == 1)
		{
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
			uv = uvs;
		}
		
		Material material;
		material.kd = texture(tex1, uv).rgb;
		material.ks = texture(tex2, uv).rgb;
		
		for (int i = 0; i < numDir; ++i)
		{
			local += CalcDirLights(dirLights[i], norm, viewDir, material);
		}
		
		for (int i = 0; i < numPoint; ++i)
		{
			local += CalcPointLights(pointLights[i], norm, viewDir, material);
		}
		
		for (int i = 0; i < numSpot; ++i)
		{
			local += CalcSpotLights(spotLights[i], norm, viewDir, material);
		}
		
		local += g_ambient;
		result = S * local + (1 - S) * fogColor;
		
		if(isReflect == 0 && isRefract == 0)
			fragColor = vec4(result, 1.0);
		else
			fragColor = vec4(mix(color, result, 0.5), 1.0);
	}
	else	
		fragColor = vec4(color, 1.0);
}