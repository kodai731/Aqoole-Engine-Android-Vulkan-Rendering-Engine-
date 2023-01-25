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
#include "AEMatrix.hpp"
//=====================================================================
//AE matrix
//=====================================================================
void AEMatrix::Perspective(glm::mat4 &proj, float angle, float aspect, float near, float far)
{
	angle = (angle / 2.0f) * (M_PI / 180.0f);
	float baseAngle = tanf(angle);
	float scaleX = aspect;
	float baseZ = 1.0f / (far - near);
	float scaleY;
	// __m512 base512 = _mm512_load_ps(&baseAngle);
	// __m512 scaleY512 = _mm512_load_ps(&scaleY);
	// scaleY512 = _mm512_rcp14_ps(base512);
	// __m512 scaleX512 = _mm512_load_ps(&scaleX);
	// scaleX512 = _mm512_div_ps(scaleY512, scaleX512);
	// __m512 scaleZ512 = _mm512_load_ps(&scaleZ);
	// scaleZ512 = _mm512_rcp14_ps(scaleZ512);
	scaleY = 1.0f / baseAngle;
	scaleX = scaleY / scaleX;
	float scaleZ = -1.0f * (far + near) * baseZ;
	float transZ = -2.0f * near * far * baseZ;
	proj = glm::mat4(scaleX, 0.0f, 0.0f, 0.0f, 0.0f, scaleY, 0.0f, 0.0f,
					 0.0f, 0.0f, scaleZ, -1.0f, 0.0f, 0.0f, transZ, 0.0f);
    //proj = glm::perspective(angle, aspect, near, far);
}

void AEMatrix::View(glm::mat4 &view, glm::vec3 const& camera, glm::vec3 const& direction, glm::vec3 const& up)
{
	//https://yttm-work.jp/gmpg/gmpg_0003.html
	//orientation matrix
	/*
	direction toward the reverse of camera
	https://learnopengl.com/Getting-started/Camera
	so we need to negate the direction vector
	*/
	glm::vec3 nZ = glm::normalize(direction);
	glm::vec3 nX = glm::normalize(glm::cross(up, nZ));
	glm::vec3 nY = glm::cross(nX, nZ);
	glm::mat4 orientation (glm::vec4(nX.x, nY.x, nZ.x, 0.0f), glm::vec4(nX.y, nY.y, nZ.y, 0.0f),
						   glm::vec4(nX.z, nY.z, nZ.z, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	//translate matrix
	glm::mat4 translate(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
						glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(-camera, 1.0f));
	view = orientation * translate;
}

/*
theory of Rodrigues
 rotate theta based from n with right-hand
 https://ja.wikipedia.org/wiki/%E3%83%AD%E3%83%89%E3%83%AA%E3%82%B2%E3%82%B9%E3%81%AE%E5%9B%9E%E8%BB%A2%E5%85%AC%E5%BC%8F
*/
void AEMatrix::Rodrigues(glm::mat3 &rotate, float c, float s, glm::vec3 const& n)
{
	float ac = 1.0f - c;
	float xyac = n.x * n.y * ac;
	float yzac = n.y * n.z * ac;
	float zxac = n.x * n.z * ac;
	float xs = n.x * s;
	float ys = n.y * s;
	float zs = n.z * s;
	// rotate = glm::mat3(c + n.x * n.x * ac, n.x * n.y * ac + n.z * s, n.z * n.x * ac - n.y * s,
	//     n.x * n.y * ac - n.z * s, c + n.y * n.y * ac, n.y * n.z * ac + n.x * s,
	//     n.z * n.x * ac + n.y * s, n.y * n.z * ac - n.x * s, c + n.z * n.z * ac);
	rotate = glm::mat3(c + n.x * n.x * ac, xyac + zs, zxac - ys,
					   xyac - zs, c + n.y * n.y * ac, yzac + xs,
					   zxac + ys, yzac - xs, c + n.z * n.z * ac);
	return;
}

/*
 * ortho vec3 to plane
 * https://python.atelierkobato.com/projection/
 */
glm::vec3 AEMatrix::Ortho(glm::vec3 base1, glm::vec3 base2, glm::vec3 obj)
{
    glm::mat2x3 a(base1, base2);
    glm::mat3x2 ta = glm::transpose(a);
    return (a * glm::inverse(ta * a) * ta) * obj;
}

#ifndef __ANDROID__
/*
test multiple vector of matrix
*/
glm::vec3 AEMatrix::TestMult(glm::mat3 const& m, glm::vec3 const& v)
{
	float f[4];
	__m128 x0 = _mm_set_ps(0.0f, m[0][2], m[0][1], m[0][0]);
	__m128 x1 = _mm_broadcast_ss(&v.x);
	__m128 x2 = _mm_set_ps(0.0f, m[1][2], m[1][1], m[1][0]);
	__m128 x3 = _mm_broadcast_ss(&v.y);
	__m128 x4 = _mm_set_ps(0.0f, m[2][2], m[2][1], m[2][0]);
	__m128 x5 = _mm_broadcast_ss(&v.z);
	x0 = _mm_mul_ps(x0, x1);
	x2 = _mm_mul_ps(x2, x3);
	x4 = _mm_mul_ps(x4, x5);
	x0 = _mm_add_ps(x0, x2);
	x0 = _mm_add_ps(x0, x4);
	_mm_store_ps(f, x0);
	return glm::vec3(f[0], f[1], f[2]);
}


/*
key event
*/
void AEMatrix::KeyEventRotate(GLFWwindow *window, glm::mat4 &rotate, float angle)
{
	//wiki
	//wiki/kaitenngyouretu "theory of Rodrigues"
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{	
		float radian = -1.0f * (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		rotate = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f,
			0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) * rotate;
		return;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		float radian = (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		rotate = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f,
			0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) * rotate;
		return;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		float radian = (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		rotate = glm::mat4(c, 0.0f, -s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) * rotate;
		return;
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		float radian = (angle / 180.0f) * (float)M_PI * -1.0f;
		float c = cos(radian);
		float s = sin(radian);
		rotate = glm::mat4(c, 0.0f, -s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) * rotate;
		return;
	}
	return;
}

/*
translate model view matrix
*/
void AEMatrix::KeyEventTranslate(GLFWwindow *window, glm::mat4 &translate, float time)
{
	float move = time * 0.1f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		translate *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, move, 0.0f));
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, -1.0f * move, 0.0f, 1.0f)) * translate;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(-1.0f * move, 0.0f, 0.0f, 1.0f)) * translate;
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(move, 0.0f, 0.0f, 1.0f)) * translate;
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, -move, 0.0f, 1.0f)) * translate;
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		translate = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, move, 0.0f, 1.0f)) * translate;
	return;
}

/*
rotate camera
*/
void AEMatrix::KeyEventRotateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
    glm::vec3 &direction, glm::vec3 &up, float angle)
{
	//wiki
	//wiki/kaitenngyouretu "theory of Rodrigues"
    //attention that camera in on axis
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{	
		float radian = -1.0f * (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		glm::mat3 localRotate;
		Rodrigues(localRotate, c, s, glm::normalize(glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(direction.x, 0.0, direction.z))));
        up = localRotate * up;
		direction = localRotate * direction;
        View(view, cameraPos, direction, up);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		float radian = (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		glm::mat3 localRotate;
		Rodrigues(localRotate, c, s, glm::normalize(glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(direction.x, 0.0, direction.z))));
        up = localRotate * up;
		direction = localRotate * direction;
		View(view, cameraPos, direction, up);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		float radian = (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		glm::mat3 localRotate;
		Rodrigues(localRotate, c, s, glm::vec3(0.0f, -1.0f, 0.0f));
		direction = localRotate * direction;
		up = localRotate * up;
        View(view, cameraPos, direction, up);
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		float radian = -1.0f * (angle / 180.0f) * (float)M_PI;
		float c = cos(radian);
		float s = sin(radian);
		glm::mat3 localRotate;
		Rodrigues(localRotate, c, s, glm::vec3(0.0f, -1.0f, 0.0f));
		direction = localRotate * direction;
		up = localRotate * up;
        View(view, cameraPos, direction, up);
	}
	return;
}

/*
translate camera
*/
void AEMatrix::KeyEventTranslateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
    glm::vec3 &direction, glm::vec3 &up, float move)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
		cameraPos += -move * glm::vec3(direction.x, 0.0f, direction.z);
        View(view, cameraPos, direction, up);
    }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos += move * glm::vec3(direction.x, 0.0f, direction.z);
        View(view, cameraPos, direction, up);
    }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
		glm::vec3 localDir = glm::cross(up, direction);
        cameraPos += -move * glm::vec3(localDir.x, 0.0f, localDir.z);
        View(view, cameraPos, direction, up);
    }
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
		glm::vec3 localDir = glm::cross(up, direction);
        cameraPos += move * glm::vec3(localDir.x, 0.0f, localDir.z);
        View(view, cameraPos, direction, up);
    }
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        cameraPos += move * glm::vec3(0.0f, -1.0f, 0.0f);
        View(view, cameraPos, direction, up);
    }
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        cameraPos += move * glm::vec3(0.0f, 1.0f, 0.0f);
        View(view, cameraPos, direction, up);
    }
	return;
}



/*
format camera direction and up
*/
void AEMatrix::KeyEventFormat(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
    glm::vec3 &direction, glm::vec3 &up)
{
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
		direction = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
		up = glm::vec3(0.0f, -1.0f, 0.0f);
        View(view, cameraPos, direction, up);
    }
}


/*
mouse event
*/
void AEMatrix::MouseCursolMove(GLFWwindow *window, void(*callback)(GLFWwindow*, double, double))
{
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)callback);
}


/*
mouse pos get
*/
void AEMatrix::MouseCursolGetPos(GLFWwindow *window, double &x, double &y)
{
	glfwGetCursorPos(window, &x, &y);
}

/*
mouse event rotate camera
*/
void AEMatrix::MouseEventRotateCamera(GLFWwindow *window, glm::mat4 &view, glm::vec3 &cameraPos,
    glm::vec3 &direction, glm::vec3 &up, glm::vec3 &diff)
{
	//axis
	glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), diff));
	//degree
	glm::vec3 b = glm::vec3(0.0f, 0.0f, 1.0f) + diff;
	float lengthB = glm::length(b);
	float lengthDiff = glm::length(diff);
	float c = 1.0f / lengthB;
	float s = lengthDiff / lengthB;
	//rotate matrix
	glm::mat3 rotateM;
	Rodrigues(rotateM, c, s, axis);
    up = rotateM * up;
	direction = rotateM * direction;
	View(view, cameraPos, direction, up);
}

#endif