#ifndef _GLUTILS_H_
#define _GLUTILS_H_

#include "common.h"

#ifdef _WIN32
#pragma warning (disable: 4224)
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#endif

#ifdef __APPLE__
//#include <GLUT/GLUT.h>
#include <OpenGL/gl.h>
#include "GLFW/glfw3.h"
#endif

// check if an OpenGL error
inline void error_if_glerror() {
    auto error = glGetError();
    error_if_not(error == GL_NO_ERROR, "gl error");
    
}

// check if an OpenGL shader is compiled
inline void error_if_shader_not_valid(int shader_id) {
    int iscompiled;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &iscompiled);
    if(not iscompiled) {
        char buf[10000];
        glGetShaderInfoLog(shader_id, 10000, 0, buf);
        error("shader not compiled\n\n%s\n\n",buf);
    }
}

// check if an OpenGL program is valid
inline void error_if_program_not_valid(int prog_id) {
    int islinked;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &islinked);
    if(not islinked) {
        char buf[10000];
        glGetProgramInfoLog(prog_id, 10000, 0, buf);
        error("program not linked\n\n%s\n\n",buf);
    }
    
    int isvalid;
    glValidateProgram(prog_id);
    glGetProgramiv(prog_id, GL_VALIDATE_STATUS, &isvalid);
    if(not isvalid) {
        char buf[10000];
        glGetProgramInfoLog(prog_id, 10000, 0, buf);
        error("program not validated\n\n%s\n\n",buf);
    }
}

#endif
