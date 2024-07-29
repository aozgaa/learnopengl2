#pragma once

#include <glad/glad.h>

#include <csignal>
#include <cstdio>
#include <print>

/////////////////////////////////////////////
// GLCALL-style debugging
/////////////////////////////////////////////

#define ENUM_TEXT_PAIR(x)                                                                \
  { x, #x }

struct EnumTextPair {
  GLenum      m_enum;
  const char *m_text;
};

const char *GetGLErrorString(GLenum error) {
  static const EnumTextPair errors[] = { ENUM_TEXT_PAIR(GL_NO_ERROR),
                                         ENUM_TEXT_PAIR(GL_INVALID_ENUM),
                                         ENUM_TEXT_PAIR(GL_INVALID_VALUE),
                                         ENUM_TEXT_PAIR(GL_INVALID_OPERATION),
                                         ENUM_TEXT_PAIR(GL_INVALID_FRAMEBUFFER_OPERATION),
                                         ENUM_TEXT_PAIR(GL_OUT_OF_MEMORY) };

  for (int errorIndex = 0; errorIndex < std::size(errors); ++errorIndex) {
    if (errors[errorIndex].m_enum == error) {
      return errors[errorIndex].m_text;
    }
  }

  return "Unknown error!";
}

#if defined(_DEBUG)
#define GLCALL(call)                                                                     \
  do {                                                                                   \
    GLenum preError = glGetError();                                                      \
    call;                                                                                \
    GLenum postError = glGetError();                                                     \
    GLenum error     = preError != GL_NO_ERROR ? preError : postError;                   \
    if (error != GL_NO_ERROR) {                                                          \
      std::println("GLError [0x{:08x}]: {} encountered {} executing:\n\t" #call, error,  \
                   GetGLErrorString(error), error == preError ? "BEFORE" : "WHILE");     \
      std::raise(SIGINT);                                                                \
    }                                                                                    \
  } while (0)
#else
#define GLCALL(call)                                                                     \
  do {                                                                                   \
    call;                                                                                \
  } while (0)
#endif

/////////////////////////////////////////////
// glDebugMessageCb
/////////////////////////////////////////////

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
               "GL DEBUG MSG CALLBACK: {} type = 0x{:x} ({}), "
               "severity = 0x{:x} ({}), "
               "message = {}\n",
               (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, typeCstr,
               severity, severityCstr, message);
}

#undef VARNAME