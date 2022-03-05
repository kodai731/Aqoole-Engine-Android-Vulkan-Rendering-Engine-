//Copyright 2022 Shigeoka Kodai
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//        limitations under the License.
#ifndef _AE_UBO
#define _AE_UBO

#ifdef __ANDROID__
#include <vulkan_wrapper.h>
#include "glm/glm.hpp"
#else
#include <vulkan/vulkan.hpp>
#include <glm.hpp>
#endif
/*
uniform
 */
struct Vertex
{
	//----------variables----------
	glm::vec4 pos;
	glm::vec4 color;
	glm::vec4 texCoord;
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		//texture coordinates
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex &other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

struct Vertex2D
{
	//----------variables----------
	glm::vec2 pos;
	glm::vec3 color;
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, pos);
		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, color);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex2D &other) const
	{
		return pos == other.pos && color == other.color;
	}
};

struct Vertex3D
{
	//----------variables----------
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, pos);
		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, color);
		//normal
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, normal);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex3D &other) const
	{
		return pos == other.pos && color == other.color && normal == other.normal;
	}
};

struct Vertex3DTEST
{
	//----------variables----------
	float pos[3];
	float color[3];
	float normal[3];
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3DTEST);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3DTEST, pos);
		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3DTEST, color);
		//normal
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3DTEST, normal);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex3DTEST &other) const
	{
		return pos == other.pos && color == other.color && normal == other.normal;
	}
	//set
	void SetPos(float x, float y, float z)
	{
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
	}
	void SetColor(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}
	void SetNormal(float x, float y, float z)
	{
		normal[0] = x;
		normal[1] = y;
		normal[2] = z;
	}
};

struct Vertex3DTexture
{
	//----------variables----------
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texcoord;
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3DTexture);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3DTexture, pos);
		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3DTexture, color);
		//normal
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3DTexture, normal);
		//texture coordinates
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex3DTexture, texcoord);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex3DTexture &other) const
	{
		return pos == other.pos && color == other.color && normal == other.normal && texcoord == other.texcoord;
	}
};

struct Vertex3DObj
{
	//----------variables----------
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec4 vertexTangent;
	//----------functions----------
	//binding
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3DObj);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	//attributes
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		//position
		attributeDescriptions[0].binding = 0;						//which binding the per-vertex data comes
		attributeDescriptions[0].location = 0;						//location directive of the input in the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3DObj, pos);
		//normal
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3DObj, normal);
		//texture coordinates
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3DObj, texcoord);
		//vertex tangent
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex3DObj, vertexTangent);
		return attributeDescriptions;
	}
	//operator
	bool operator ==(const Vertex3DObj &other) const
	{
		return pos == other.pos && normal == other.normal && texcoord == other.texcoord && vertexTangent == other.vertexTangent;
	}
};

struct ModelView
{
	//modelview = proj * view * model
	//model = translate * rotate * scale	
	glm::mat4 translate;
	glm::mat4 rotate;
	glm::mat4 scale;
	//view
	glm::mat4 proj;
	glm::mat4 view;
	//normal matrix
	glm::mat3 normalMatrix;
};

struct Light
{
	glm::vec3 lightPosition;
	glm::vec3 eyeDirection;
	glm::vec3 ambientColor;
};

struct LightStrength
{
	glm::vec3 La;	//light ambient
	glm::vec3 Ld;	//light diffuse
	glm::vec3 Ls;	//light specular
};

struct MaterialInfo
{
	glm::vec3 Ka;	//ambient reflection
	glm::vec3 Kd;	//diffuse reflection
	glm::vec3 Ks;	//specular reflection
	float shininess;
};

//for ray trace
struct VertexRT
{
	float pos[3];
};

struct UBORT
{
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
	glm::mat4 modelViewProj;
	glm::mat4 normalMatrix;
};

struct ConstantsRT
{
	glm::vec4 clearColor;
	glm::vec3 lightPosition;
	float loghtIntensity;
	int lightType;
};

struct CameraProp
{
	glm::vec3 cameraPos;
	glm::vec3 cameraDirection;
	glm::vec3 cameraUp;
};

struct DebugGLSL
{
	float x;
	float y;
	float z;
};

#endif