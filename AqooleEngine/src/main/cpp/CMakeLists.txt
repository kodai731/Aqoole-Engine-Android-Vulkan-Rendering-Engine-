
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)
#cmake_minimum_required(VERSION 3.4.1)

# Declares and names the project.

project("AqooleEngine")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(SRC_DIR .)
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
set(WRAPPER_DIR ../../../../../common/vulkan_wrapper)
set(VK_VAL_LAYER_SRC_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src)
set(VK_INCLUDE_DIR ${ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include)
set(MY_VULKAN_SRC_DIR ${SRC_DIR}/AESrc)
set(IMGUI_DIR ${SRC_DIR}/AEInclude/imgui/imgui)
set(GLM_DIR ${SRC_DIR}/glm)
set(STB_DIR ${SRC_DIR}/stb/stb)
set(BOOST_DIR ${SRC_DIR}/boost)
set(TINY_GLTF_DIR ${SRC_DIR}/tinygltf)
#include_directories(${VK_INCLUDE_DIR} ${VK_VAL_LAYER_SRC_DIR}/include ${WRAPPER_DIR}  ${SRC_DIR}/AEInclude ${IMGUI_DIR} ${GLM_DIR} ${APP_GLUE_DIR})
include_directories(${VK_INCLUDE_DIR} ${WRAPPER_DIR}  ${SRC_DIR}/AEInclude ${IMGUI_DIR} ${GLM_DIR} ${APP_GLUE_DIR} ${STB_DIR} ${BOOST_DIR} ${TINY_GLTF_DIR})
add_library( # Sets the name of the library.
             AqooleEngine

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${WRAPPER_DIR}/vulkan_wrapper.cpp
             ${MY_VULKAN_SRC_DIR}/AEDevice.cpp
             ${MY_VULKAN_SRC_DIR}/AEDeviceQueue.cpp
             ${MY_VULKAN_SRC_DIR}/AEWindow.cpp
             ${MY_VULKAN_SRC_DIR}/AEImage.cpp
             ${MY_VULKAN_SRC_DIR}/descriptorSet.cpp
             ${MY_VULKAN_SRC_DIR}/AEPipeline.cpp
             ${MY_VULKAN_SRC_DIR}/AEBuffer.cpp
             ${MY_VULKAN_SRC_DIR}/AECommand.cpp
             ${MY_VULKAN_SRC_DIR}/AESyncObjects.cpp
             ${MY_VULKAN_SRC_DIR}/AEMatrix.cpp
             ${MY_VULKAN_SRC_DIR}/AEDrawObjects.cpp
             ${MY_VULKAN_SRC_DIR}/AECommand.cpp
             ${IMGUI_DIR}/imgui.cpp
             ${IMGUI_DIR}/imgui_demo.cpp
             ${IMGUI_DIR}/imgui_draw.cpp
             ${IMGUI_DIR}/imgui_tables.cpp
             ${IMGUI_DIR}/imgui_widgets.cpp
             ${IMGUI_DIR}/backends/imgui_impl_android.cpp
             ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
             AqooleEngine.cpp)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 \
                     -Wno-unused-variable -DUSE_DEBUG_EXTENTIONS \
                     -DVK_USE_PLATFORM_ANDROID_KHR -D__ANDROID__ \
                     -D__RAY_TRACING__")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       AqooleEngine

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} android jnigraphics)
target_compile_options(AqooleEngine PUBLIC -g -O0)
