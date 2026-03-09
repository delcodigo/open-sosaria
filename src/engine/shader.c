#include <stdlib.h>
#include <stdio.h>
#include "dependencies/glad.h"

static const char *vertex_shader_source =
    "#version 120\n"
    "attribute vec3 aVertex;\n"
    "attribute vec2 aTexCoord;\n"
    "uniform mat4 uModel;\n"
    "uniform mat4 uViewProjection;\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_Position = uViewProjection * uModel * vec4(aVertex, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "}\n";

static const char *fragment_shader_source =
    "#version 120\n"
    "uniform sampler2D uTexture;\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(uTexture, vTexCoord);\n"
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
  glBindAttribLocation(program, 0, "aVertex");
  glBindAttribLocation(program, 1, "aTexCoord");
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