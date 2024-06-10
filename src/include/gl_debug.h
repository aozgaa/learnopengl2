#pragma once

#include <glad/glad.h>

#include <cstdio>
#include <print>

#define VARNAME(var) (#var)

void GLAPIENTRY glDebugMessageCb(GLenum source, GLenum type, GLuint id, GLenum severity,
                                 GLsizei length, const GLchar *message,
                                 const void *userParam) {

  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    return; // ignore
  }
  const char *typeCstr =
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
  const char *severityCstr =
      severity == GL_DEBUG_SEVERITY_HIGH     ? VARNAME(GL_DEBUG_SEVERITY_HIGH)
      : severity == GL_DEBUG_SEVERITY_MEDIUM ? VARNAME(GL_DEBUG_SEVERITY_MEDIUM)
      : severity == GL_DEBUG_SEVERITY_LOW    ? VARNAME(GL_DEBUG_SEVERITY_LOW)
      : severity == GL_DEBUG_SEVERITY_NOTIFICATION
          ? VARNAME(GL_DEBUG_SEVERITY_NOTIFICATION)
          : "UNKNOWN_SEVERITY";
  std::println(stderr,
               "GL CALLBACK: {} type = 0x{:x} ({}), "
               "severity = 0x{:x} ({}), "
               "message = {}\n",
               (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, typeCstr,
               severity, severityCstr, message);
}

#undef VARNAME