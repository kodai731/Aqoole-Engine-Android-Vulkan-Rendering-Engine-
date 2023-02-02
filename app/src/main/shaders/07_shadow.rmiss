#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "07_raycommon.glsl"

layout(location = 1) rayPayloadInEXT bool isShadowed;

layout(push_constant) uniform Constants
{
  vec4 clearColor;
  float lightIntensity;
}pushC;

void main()
{
  isShadowed = false;
}