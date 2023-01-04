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

layout(location = 0) rayPayloadInEXT vec4 pld;
hitAttributeEXT vec2 attribs;
layout(location = 1) rayPayloadEXT bool isShadowed;
layout(location = 2) rayPayloadEXT PayroadBlend prdBlend;

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
layout(binding = 7, set = 0) buffer GeometryIndices {uint gi[];} geometryIndices[];
layout(binding = 8, set = 0) uniform LIGHT {Light l;} light;
layout(binding = 9, set = 0) uniform Material {GltfMaterial gm;} Gm;
layout(binding = 10, set = 0) uniform CPOS {vec3 cp;} Cp;

layout(binding = 0, set = 1) uniform sampler2D texSampler[];

layout(push_constant) uniform Constants
{
  vec4 clearColor;
  int lightType;
}pushC;

//2^(n + 1) - 2
//const uint NODE = 126;
const uint NODE = 2;
//const uint NODE = 254;
//const uint NODE = 2;

    vec3[NODE] reflectPos;
    vec3[NODE] reflectNormal;
    //uint[NODE] reflectNormalIndex;
    bool[NODE] reflectMiss;
    vec3[NODE] reflectColor;
    bool[NODE] isAllReflect;
    bool[NODE] isPlane;
float[NODE] reflectance;
float dotNL;

float reflectionOld = ((nAir - nWater)/(nAir + nWater)) * ((nAir - nWater)/(nAir + nWater));
float waterAlpha = 0.1;

vec3[6] normals = vec3[]
(
  vec3(1.0, 0.0, 0.0),
  vec3(-1.0, 0.0, 0.0),
  vec3(0.0, 1.0, 0.0),
  vec3(0.0, -1.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(0.0, 0.0, -1.0)
);

uint GetNormalIndex(vec3 normal)
{
  for(uint i = 0; i < 6; i++)
  {
    if(normal == normals[i])
      return i;
  }
  return 10;
}

void InitPayLoad(vec3 pos, vec3 color)
{
  prdBlend.pos = pos;
  prdBlend.normal = vec3(0.0);
  prdBlend.color = color;
  prdBlend.hit = false;
  prdBlend.isMiss = true;
}

void Caustics(vec3 planePos, inout vec3 refractColor)
{
  uint causticFlags = gl_RayFlagsNoneEXT | gl_RayFlagsTerminateOnFirstHitEXT;
  InitPayLoad(vec3(0.0), WATER_COLOR);
  traceRayEXT(topLevelAS, causticFlags, 0xFF, 1, 0, 2, planePos, 0.00001, vec3(0.0, -1.0, 0.0), 10.0, 2);
  vec3 waterSurface = prdBlend.pos;
  if(true){
    vec3 waterNormal = prdBlend.normal;
    vec3 waterV = refract(vec3(0.0, -1.0, 0.0), -waterNormal, nWater/nAir);
    vec3 normalWaterV = normalize(waterV);
    //parallel rays
    /*
    float dotLight = dot(normalize(waterV), normalize(vec3(0.0, -1.0, 1.0)));
    if( dotLight > 0.76)
    {
      refractColor += mix(vec3(dotLight), WATER_COLOR, 0.85);
    }
    */
    //sun map
    /*
    if(length(waterV) > 0.1)
    {
      float t = (light.l.lightPosition.y - waterSurface.y) / waterV.y;
      vec3 sunMapPoint = waterSurface + t * waterV;
      float dotLight = light.l.lightIntensity / (1.0 + length(sunMapPoint - light.l.lightPosition));
      float sunmapIntensity = dotLight / length(t * waterV);
      if(sunmapIntensity > 1.0)
      {
        refractColor += mix(vec3(sunmapIntensity),WATER_COLOR,0.1);
      }
    }
    */
    //dot point light

    if(length(waterV) > 0.1)
    {
      float t = (light.l.lightPosition.y - waterSurface.y) / normalWaterV.y;
      float dotLight = clamp(0.0, 1.0, dot(normalWaterV, normalize(light.l.lightPosition - waterSurface)));
      float sunmapIntensity = light.l.intensity * dotLight / length(t * normalWaterV);
      sunmapIntensity = smoothstep(0.0, 1.0, sunmapIntensity);
      refractColor += mix(vec3(sunmapIntensity),WATER_COLOR,0.8);
    }

    //water alpha
    //waterAlpha *= 1.0 + (length(refractPos - waterSurface) / 10000.0);
  }
}

void traceReflectRefract(vec3 pos, vec3 incident, vec3 normal, float refractIndex, out vec3 reflectPos, out bool reflectMiss, out vec3 reflectNormal, out vec3 reflectColor,
  out bool reflectPlane, out vec3 refractPos, out bool refractMiss, out vec3 refractNormal, out vec3 refractColor, out bool refractPlane, out bool isAllReflect, out float reflectance)
{
  float n0 = nAir;
  float n1 = nWater;
  reflectance = 1.0;
  if(dot(incident, normal) > 0)
  {
    normal *= -1.0;
    refractIndex = 1.0 / refractIndex;
    n0 = nWater;
    n1 = nAir;
  }
  uint flags = gl_RayFlagsNoneEXT;
  float tMin = 0.01;
  float tMax = 20.0;
  //reflect
  InitPayLoad(pos, vec3(1.0, 1.0, 1.0));
  vec3 reflectD = reflect(incident, normal);
  traceRayEXT(topLevelAS, flags, 0xFF, 1, 0, 2, pos, tMin, reflectD, tMax, 2);
  reflectPos = prdBlend.pos;
  reflectNormal = prdBlend.normal;
  reflectColor = prdBlend.color;
  reflectMiss = prdBlend.isMiss;
  reflectPlane = prdBlend.hit;
  //refract
  InitPayLoad(pos, WATER_COLOR);
  vec3 refractD = refract(incident, normal, refractIndex);
  isAllReflect = false;
  if(length(refractD) == 0.0)
  {
    isAllReflect = true;
    refractColor = vec3(0.0, 0.0, 1.0);
    refractPlane = false;
    refractMiss = true;
    refractPos = pos;
  }
  if(!isAllReflect)
  {
    isShadowed = true;
    traceRayEXT(topLevelAS, flags, 0xFF, 1, 0, 2, pos, tMin, refractD, tMax, 2);
    refractPos = prdBlend.pos;
    refractMiss = prdBlend.isMiss;
    refractNormal = prdBlend.normal;
    refractColor = prdBlend.color;
    refractPlane = prdBlend.hit;
    if(refractPlane){
        Caustics(refractPos, refractColor);
    }
    reflectance = (ReflectanceP(incident, normal, n0, n1) * 0.5) + (ReflectanceS(incident, normal, n0, n1) * 0.5);
  }
}

vec3 ColorBlend(float reflection, vec3 ownColor, bool isPlane, bool reflectMiss, vec3 reflectColor, bool refractMiss, vec3 refractColor, bool isAllReflect)
{
  if(isPlane)
    return ownColor;
  if(isAllReflect)
    return vec3(1.0, 0.0, 0.0);
  if(reflectMiss && refractMiss)
    return WATER_COLOR;
  if(reflectMiss){
    return refractColor;
  }
  if(refractMiss)
    return reflectColor;
  //both hit
  else
  {
    vec3 traceColor = mix(refractColor,reflectColor,reflection);
    return traceColor;
  }
}

vec3 ColorBlendALL(vec3 surfaceColor, float reflectanceOrigin, inout bool[NODE] isPlane, inout bool[NODE] isMiss, inout vec3[NODE] colors, inout bool[NODE] isAllReflect,
   inout float[NODE] reflectance)
{
  /*
  count are  2, 6, 14, 30, ... (2^(n + 1) - 2)
  */
  uint e = uint(sqrt(NODE + 2) - 1) - 1;
  uint endindex = NODE;
  uint startindex = uint(pow(2, e) - 2);
  while(e > 0){
    for(uint i = startindex; i < endindex; i++){
        uint reflectIndex = 2 * i + 2;
        uint refractIndex = 2 * i + 3;
        colors[i] = ColorBlend(reflectance[i], colors[i], isPlane[i], isMiss[reflectIndex], colors[reflectIndex], isMiss[refractIndex], colors[refractIndex], isAllReflect[refractIndex]);
    }
    e = e - 1;
    endindex = uint(pow(2, e + 1) - 2);
    startindex = uint(pow(2, e) - 2);
  }
  vec3 traceColor = ColorBlend(reflectanceOrigin, surfaceColor, false, isMiss[0], colors[0], isMiss[1], colors[1], isAllReflect[1]);
  return mix(traceColor, surfaceColor, 0.05);
}

void Lighting(inout vec3 color, vec3 pos, vec3 normal)
{
    color *= dot(normal, normalize(light.l.lightPosition - pos));
    float li = light.l.intensity / (1.0 + length(light.l.lightPosition - pos));
    color *= clamp(0.0, 1.0, li);
}

void InitArrays()
{
    for(uint i = 0; i < NODE; i++){
        reflectPos[i] = vec3(0.0);
        reflectNormal[i] = vec3(0.0);
        reflectMiss[i] = false;
        reflectColor[i] = WATER_COLOR;
        isAllReflect[i] = false;
        isPlane[i] = false;
    }
}

void main()
{
  InitArrays();
  //obj Id
  //uint objId = scnDesc.i[gl_InstanceCustomIndexEXT].objId;
  uint objId = gl_InstanceCustomIndexEXT;
  const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  vec3 color = vec3(0.0, 1.0, 0.0);
  vec4 color4 = vec4(1.0);
  float alpha = 1.0;
  if(objId == 0)
  {
    //plane
    ivec3 ind = ivec3(indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 0],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 1],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 2]);  //
    Vertex3D v0 = vertices[nonuniformEXT(objId)].v[ind.x];
    Vertex3D v1 = vertices[nonuniformEXT(objId)].v[ind.y];
    Vertex3D v2 = vertices[nonuniformEXT(objId)].v[ind.z];
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
    //color = v0.color * barycentricCoords.x + v1.color * barycentricCoords.y + v2.color * barycentricCoords.z;
    color = texture(texSampler[1], vec2(barycentricCoords.x, barycentricCoords.y)).xyz;
  }
  if(objId == 1)
  {
    //cube or water
    ivec3 ind = ivec3(indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 0],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 1],   //
                      indices[nonuniformEXT(objId)].i[3 * gl_PrimitiveID + 2]);  //
    Vertex3D v0 = vertices[nonuniformEXT(objId)].v[ind.x];
    Vertex3D v1 = vertices[nonuniformEXT(objId)].v[ind.y];
    Vertex3D v2 = vertices[nonuniformEXT(objId)].v[ind.z];
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
    float refractRatio = nAir / nWater;
    prdBlend.hit = false;
    prdBlend.color = vec3(1.0, 1.0, 1.0);
    vec4 cameraPos = cam.viewInverse * vec4(0, 0, 0, 1);
    vec3 direction = normalize(worldPos - cameraPos.xyz);
    float reflectanceOrigin;
    vec3 normal = normalize(cross(v1.pos - v0.pos, v2.pos - v0.pos));
    //depth = 0
    traceReflectRefract(worldPos, direction, normal, refractRatio, reflectPos[0], reflectMiss[0], reflectNormal[0], reflectColor[0], isPlane[0],
      reflectPos[1], reflectMiss[1], reflectNormal[1], reflectColor[1], isPlane[1], isAllReflect[1], reflectanceOrigin);
    for(uint i = 0; i < (NODE / 2) - 1; i++)
    {
      vec3 incident;
      if(i < 2)
      {
        incident = normalize(reflectPos[i] - worldPos);
      }
      else
      {
        uint lastIndex = (i - 2) / 2;
        incident = normalize(reflectPos[i] - reflectPos[lastIndex]);
      }
      traceReflectRefract(reflectPos[i], incident, reflectNormal[i], refractRatio, reflectPos[2 * i + 2], reflectMiss[2 * i + 2],
        reflectNormal[2 * i + 2], reflectColor[2 * i + 2], isPlane[2 * i + 2], reflectPos[2 * i + 3], reflectMiss[2 * i + 3], reflectNormal[2 * i + 3],
        reflectColor[2 * i + 3], isPlane[2 * i + 3], isAllReflect[2 * i + 3], reflectance[i]);
    }
    //color = ColorBlend(reflection, color, false, reflectMiss[0], color0, reflectMiss[1], color1, isAllReflect[1]);
    color = ColorBlendALL(WATER_COLOR, reflectanceOrigin, isPlane, reflectMiss, reflectColor, isAllReflect, reflectance);
    Lighting(color, worldPos, normal);
  }
  else if(objId == 2)
  {
    //woman
    ivec3 ind = ivec3(indicesobj[0].iobj[3 * gl_PrimitiveID + 0],   //
                      indicesobj[0].iobj[3 * gl_PrimitiveID + 1],   //
                      indicesobj[0].iobj[3 * gl_PrimitiveID + 2]);  //
    Vertex3DObj v0 = verticesobj[0].vobj[ind.x];
    Vertex3DObj v1 = verticesobj[0].vobj[ind.y];
    Vertex3DObj v2 = verticesobj[0].vobj[ind.z];
    uint offset = 3 * gl_PrimitiveID;
    vec3 worldPos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
    vec3 normal = v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z;
    for(uint i = 0; i < 1; i++){
        //TODO : multi texture in gltf model
        //if(offset < geometryIndices[0].gi[i]){
            color4 = texture(texSampler[i], v0.texcoord * barycentricCoords.x + v1.texcoord * barycentricCoords.y + v2.texcoord * barycentricCoords.z);
            color = color4.xyz;
            //Lighting(color, worldPos, normal);
            alpha = color4.w;
        //}
    }
  }
  pld = vec4(color, alpha);
}