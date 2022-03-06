cmake_minimum_required(VERSION 3.4.1)
#this file is modified by Shigeoka Kodai
#original file is in https://github.com/googlesamples/android-vulkan-tutorials

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

#import Aqoole Engine
add_library(AqooleEngine SHARED IMPORTED)
####
#please edit your install location
####
set_target_properties(AqooleEngine PROPERTIES IMPORTED_LOCATION
        "C:/Users/kodai/AndroidStudioProjects/vulkanTutorial/android-vulkan-tutorials_myRepo/Aqoole_project00/AqooleEngine/build/intermediates/library_jni/debug/jni/${CMAKE_ANDROID_ARCH_ABI}/libAqooleEngine.so")

# build vulkan app
set(SRC_DIR src/main/jni)
set(WRAPPER_DIR ../../common/vulkan_wrapper)
set(VK_VAL_LAYER_SRC_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src)
set(MY_VULKAN_SRC_DIR ${SRC_DIR}/myVulkanSrc)
set(IMGUI_DIR ${SRC_DIR}/myVulkanInclude/imgui/imgui)
set(GLM_DIR ${SRC_DIR}/glm)
add_library(main SHARED
            ${MY_VULKAN_SRC_DIR}/AEWindow.cpp
            ${SRC_DIR}/VulkanMain.cpp
            ${SRC_DIR}/AndroidMain.cpp
            ${WRAPPER_DIR}/vulkan_wrapper.cpp)

include_directories(${VK_VAL_LAYER_SRC_DIR}/include ${WRAPPER_DIR} ${SRC_DIR}/myVulkanInclude ${IMGUI_DIR} ${GLM_DIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall \
#                     -DVK_USE_PLATFORM_ANDROID_KHR")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Werror       \
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 \
                     -Wno-unused-variable -DUSE_DEBUG_EXTENTIONS \
                     -DVK_USE_PLATFORM_ANDROID_KHR")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
target_link_libraries(main AqooleEngine app-glue log android)
