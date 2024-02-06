#ifndef IMG_UTIL_HPP
#define IMG_UTIL_HPP
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>


bool loadPngImage(const char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
void saveImage(const char* filepath, GLFWwindow* w);

#endif
