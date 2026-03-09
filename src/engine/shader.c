#include <stdlib.h>
#include <stdio.h>
#include "dependencies/glad.h"

static const char *vertex_shader_source =
    "#version 120\n"
    "attribute vec3 aColor;\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    vColor = aColor;\n"
    "}\n";

static const char *fragment_shader_source =
    "#version 120\n"
    "varying vec3 vColor;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(vColor, 1.0);\n"
    "}\n";

static unsigned int shader_compile(unsigned int type, const char *source) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success = 0;
  char info_log[512] = {0};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    fprintf(stderr, "Shader compilation failed:\n%s\n", info_log);
    exit(EXIT_FAILURE);
  }

  return shader;
}

unsigned int shader_create_program(void) {
  unsigned int vertex_shader = shader_compile(GL_VERTEX_SHADER, vertex_shader_source);
  unsigned int fragment_shader = shader_compile(GL_FRAGMENT_SHADER, fragment_shader_source);

  unsigned int program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glBindAttribLocation(program, 0, "gl_Vertex");
  glBindAttribLocation(program, 1, "aColor");
  glLinkProgram(program);

  int success = 0;
  char info_log[512] = {0};
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, info_log);
    fprintf(stderr, "Program linking failed:\n%s\n", info_log);
    exit(EXIT_FAILURE);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}