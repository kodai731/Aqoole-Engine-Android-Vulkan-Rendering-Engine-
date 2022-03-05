// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;

layout (location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform ModelView
{
 	mat4 translate;
 	mat4 rotate;
 	mat4 scale;
 	//view
 	mat4 proj;
 	mat4 view;
 	//normal matrix
 	mat3 normalMatrix;
} modelview;


void main() {
   gl_Position = (modelview.proj * modelview.view * modelview.translate * modelview.rotate * modelview.scale) * vec4(pos, 1.0);
   outColor = vec4(color, 1.0);
}
