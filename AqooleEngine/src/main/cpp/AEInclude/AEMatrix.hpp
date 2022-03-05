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
#ifndef _AE_MATRIX
#define _AE_MATRIX
#define _USE_MATH_DEFINES

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#ifndef __ANDROID__
#include <glfw3.h>
#include <x86intrin.h>
#endif

namespace AEMatrix
{
    void Perspective(glm::mat4 &proj, float angle, float aspect, float near, float far);
    void View(glm::mat4 &view, glm::vec3 const& camera, glm::vec3 const& look, glm::vec3 const& up);
    void Rodrigues(glm::mat3 &rotate, float c, float s, glm::vec3 const& n);
#ifndef __ANDROID__
    glm::vec3 TestMult(glm::mat3 const& m, glm::vec3 const& v);
    void KeyEventRotate(GLFWwindow *window, glm::mat4 &rotate, float angle);
    void KeyEventTranslate(GLFWwindow *window, glm::mat4 &translate, float time);
    void KeyEventRotateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
        glm::vec3 &direction, glm::vec3 &up, float angle);
    void KeyEventTranslateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
        glm::vec3 &direction, glm::vec3 &up, float move);
    void KeyEventFormat(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
        glm::vec3 &direction, glm::vec3 &up);
    //mouse event
    void MouseCursolMove(GLFWwindow *window, void(*callback)(GLFWwindow*, double, double));
    void MouseCursolGetPos(GLFWwindow *window, double &x, double &y);
    void MouseEventRotateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
        glm::vec3 &direction, glm::vec3 &up, glm::vec3 &diff);
#endif
};

#endif