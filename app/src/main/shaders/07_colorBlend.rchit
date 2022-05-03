/* Copyright (c) 2019-2020, Sascha Willems
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "07_raycommon.glsl"

layout(location = 2) rayPayloadInEXT PayroadBlend prdBlend;
hitAttributeEXT vec2 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2, set = 0) uniform CameraProperties 
{
	mat4 viewInverse;
	mat4 projInverse;
  mat4 modelViewProj;
  mat4 normalMatrix;
} cam;
layout(binding = 3, set = 0, scalar) buffer Vertices {Vertex3D v[];} vertices[];
layout(binding = 4, set = 0) buffer Indices {uint i[];} indices[];
layout(binding = 5, set = 0, scalar) buffer Verticesobj {Vertex3DObj vobj[];} verticesobj[];
layout(binding = 6, set = 0) buffer Indicesobj {uint iobj[];} indicesobj[];
layout(binding = 7, set = 0) buffer IndicesOffset {uint ioff[];} indicesoff[];

/*
layout(binding = 0, set = 1) uniform sampler2D texSampler0;
layout(binding = 1, set = 1) uniform sampler2D texSampler1;
layout(binding = 2, set = 1) uniform sampler2D texSampler2;
layout(binding = 3, set = 1) uniform sampler2D texSampler3;
*/
layout(binding = 0, set = 1) uniform sampler2D texSampler0;

layout(push_constant) uniform Constants
{
  vec4 clearColor;
  vec3 lightPosition;
  float lightIntensity;
  int lightType;
}pushC;

void main()
{
  //obj Id
  uint objId = gl_InstanceCustomIndexEXT;
  const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  if(objId == 0)
  {
    ivec3 ind = ivec3(indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 0],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 1],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 2]);  //
    Vertex3D v0 = vertices[nonuniformEXT(objId)].v[ind.x];
    Vertex3D v1 = vertices[nonuniformEXT(objId)].v[ind.y];
    Vertex3D v2 = vertices[nonuniformEXT(objId)].v[ind.z];
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;                         //object coordinates
    //worldPos = vec3(cam.modelViewProj * vec4(worldPos, 1.0));
    //vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    //normal = normalize(vec3(cam.normalMatrix * vec4(normal, 1.0)));
    vec3 normal = cross(v1.pos - v0.pos, v2.pos - v0.pos);
    normal = normalize(normal);
    vec3 color = v0.color * barycentricCoords.x + v1.color * barycentricCoords.y + v2.color * barycentricCoords.z;
    //branch cube or plane
    prdBlend.pos = worldPos;
    prdBlend.hit = true;
    //prdBlend.color = texture(texSampler, vec2(attribs.x, attribs.y)).xyz;
    prdBlend.color = color;
    prdBlend.normal = normal;
  }
  else if(objId == 1)          //cube
  {
    ivec3 ind = ivec3(indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 0],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 1],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 2]);  //
    Vertex3D v0 = vertices[nonuniformEXT(objId)].v[ind.x];
    Vertex3D v1 = vertices[nonuniformEXT(objId)].v[ind.y];
    Vertex3D v2 = vertices[nonuniformEXT(objId)].v[ind.z];
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;                         //object coordinates
    vec3 normal = cross(v1.pos - v0.pos, v2.pos - v0.pos);
    normal = normalize(normal);
    vec3 color = v0.color * barycentricCoords.x + v1.color * barycentricCoords.y + v2.color * barycentricCoords.z;
    prdBlend.pos = worldPos;
    prdBlend.normal = normal;
    prdBlend.color = color;
    prdBlend.hit = false;
    //test
    //prdBlend.color = vec3(0.0, 0.0, 1.0);
  }
  else //woman
  {
    ivec3 ind = ivec3(indicesobj[nonuniformEXT(objId)].iobj[3 * gl_PrimitiveID + 0],   //
                      indicesobj[nonuniformEXT(objId)].iobj[3 * gl_PrimitiveID + 1],   //
                      indicesobj[nonuniformEXT(objId)].iobj[3 * gl_PrimitiveID + 2]);  //
    Vertex3DObj v0 = verticesobj[nonuniformEXT(objId)].vobj[ind.x];
    Vertex3DObj v1 = verticesobj[nonuniformEXT(objId)].vobj[ind.y];
    Vertex3DObj v2 = verticesobj[nonuniformEXT(objId)].vobj[ind.z];
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;                         //object coordinates
    vec3 normal = cross(v1.pos - v0.pos, v2.pos - v0.pos);
    normal = normalize(normal);
    vec3 color;
    /*
    //color = texture(texSampler, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
    //color = texture(texSampler, vec2(0.1, 0.1)).xyz;
    uint offset = 3 * gl_PrimitiveID;
    if(offset < 44106)
      color = texture(texSampler0, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
      //color = texture(texSampler, vec2(0.1, 0.1)).xyz;
      //color = vec3(0.0, 1.0, 0.0);
    else if(44106 <= offset && offset < 58974)
      color = texture(texSampler1, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
      //color = vec3(1.0, 0.0, 0.0);
    else if(58974 <= offset && offset < 69990)
      color = texture(texSampler2, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
      //color = vec3(0.0, 0.0, 1.0);
    else
      color = texture(texSampler3, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
      //color = vec3(1.0, 1.0, 1.0);
    */
    color = texture(texSampler0, v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z).xyz;
    prdBlend.pos = worldPos;
    prdBlend.normal = normal;
    prdBlend.color = color;
    prdBlend.hit = true;
  }
  
}