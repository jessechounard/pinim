#pragma once

#include "Platform.h"

/* clang-format off */

#if defined(PLATFORM_WINDOWS)
	#include <glad/gl.h>
    #define GL_DESKTOP

#elif defined(PLATFORM_MACOSX)
	// todo: should this be glad/gl.h like Windows/Linux?
	#include <OpenGL/gl.h>
	#define GL_DESKTOP

#elif defined(PLATFORM_LINUX)
	#include <glad/gl.h>
	#define GL_DESKTOP

#elif defined(PLATFORM_IOS)
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	#define GL_ES2

#elif defined(PLATFORM_ANDROID)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #define GL_ES2

/* clang-format on */

#endif
