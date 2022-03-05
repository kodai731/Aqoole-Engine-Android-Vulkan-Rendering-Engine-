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

/*
 * this file is modified by Shigeoka Kodai
 * original is in https://github.com/googlesamples/android-vulkan-tutorials
 */

#ifndef __VULKANMAIN_HPP__
#define __VULKANMAIN_HPP__
#define _USE_MATH_DEFINES
#define _GLM_FORCE_RADIANS
#include <android_native_app_glue.h>
#include "myVulkanInclude/AEDevice.hpp"
#include "myVulkanInclude/AEDeviceQueue.hpp"
#include "myVulkanInclude/AEWindow.hpp"
#include "myVulkanInclude/AEPipeline.hpp"
#include "myVulkanInclude/AEBuffer.hpp"
#include "myVulkanInclude/AEImage.hpp"
#include "myVulkanInclude/AEBuffer.hpp"
#include "myVulkanInclude/AECommand.hpp"
#include "myVulkanInclude/descriptorSet.hpp"
#include "myVulkanInclude/AESyncObjects.hpp"
#include "myVulkanInclude/AEMatrix.hpp"
#include "myVulkanInclude/AEUBO.hpp"
#include "myVulkanInclude/AEDrawObjects.hpp"

const int MAX_IN_FLIGHT = 2;


// Initialize vulkan device context
// after return, vulkan is ready to draw
bool InitVulkan(android_app* app);

// delete vulkan device context when application goes away
void DeleteVulkan(void);

// Check if vulkan is ready to draw
bool IsVulkanReady(void);

// Ask Vulkan to Render a frame
bool VulkanDrawFrame(uint32_t currentFrame, bool& isTouched, bool& isFocused, glm::vec2* touchPosition,
                     glm::vec3* gravityData, glm::vec3* lastGravityData);

#endif // __VULKANMAIN_HPP__


