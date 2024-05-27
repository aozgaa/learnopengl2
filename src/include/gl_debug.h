#pragma once

#include <glad/glad.h>

#include <cstdio>

#define VARNAME(var) (#var)

void GLAPIENTRY glDebugMessageCb(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar *message,
                                       const void   *userParam) {
  const char *type_cstr =
      type == GL_DEBUG_TYPE_ERROR ? VARNAME(GL_DEBUG_TYPE_ERROR)
      : type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
          ? VARNAME(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
      : type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
          ? VARNAME(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
      : type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
          ? VARNAME(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
      : type == GL_DEBUG_TYPE_PORTABILITY ? VARNAME(GL_DEBUG_TYPE_PORTABILITY)
      : type == GL_DEBUG_TYPE_PERFORMANCE ? VARNAME(GL_DEBUG_TYPE_PERFORMANCE)
      : type == GL_DEBUG_TYPE_MARKER      ? VARNAME(GL_DEBUG_TYPE_MARKER)
      : type == GL_DEBUG_TYPE_MARKER      ? VARNAME(GL_DEBUG_TYPE_MARKER)
      : type == GL_DEBUG_TYPE_PUSH_GROUP  ? VARNAME(GL_DEBUG_TYPE_PUSH_GROUP)
      : type == GL_DEBUG_TYPE_POP_GROUP   ? VARNAME(GL_DEBUG_TYPE_POP_GROUP)
      : type == GL_DEBUG_TYPE_OTHER       ? VARNAME(GL_DEBUG_TYPE_OTHER)
                                          : "UNKNOWN TYPE";
  const char *severity_cstr =
      severity == GL_DEBUG_SEVERITY_HIGH     ? VARNAME(GL_DEBUG_SEVERITY_HIGH)
      : severity == GL_DEBUG_SEVERITY_MEDIUM ? VARNAME(GL_DEBUG_SEVERITY_MEDIUM)
      : severity == GL_DEBUG_SEVERITY_LOW    ? VARNAME(GL_DEBUG_SEVERITY_LOW)
      : severity == GL_DEBUG_SEVERITY_NOTIFICATION
          ? VARNAME(GL_DEBUG_SEVERITY_NOTIFICATION)
          : "UNKNOWN_SEVERITY";
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x (%s), "
          "severity = 0x%x (%s), "
          "message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
          type_cstr, severity, severity_cstr, message);
}

#undef VARNAME