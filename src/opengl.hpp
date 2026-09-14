#pragma once

#include <Windows.h>
#include <gl/gl.h>
#include "vendor/opengl/glext.h"

#define FOR_OPENGL_FUNCTIONS(DO)                                                                                       \
    DO(PFNGLCREATESHADERPROC, glCreateShader)                                                                          \
    DO(PFNGLSHADERSOURCEPROC, glShaderSource)                                                                          \
    DO(PFNGLCOMPILESHADERPROC, glCompileShader)                                                                        \
    DO(PFNGLGETSHADERIVPROC, glGetShaderiv)                                                                            \
    DO(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                                                                  \
    DO(PFNGLATTACHSHADERPROC, glAttachShader)                                                                          \
    DO(PFNGLDETACHSHADERPROC, glDetachShader)                                                                          \
    DO(PFNGLDELETESHADERPROC, glDeleteShader)                                                                          \
    DO(PFNGLCREATEPROGRAMPROC, glCreateProgram)                                                                        \
    DO(PFNGLLINKPROGRAMPROC, glLinkProgram)                                                                            \
    DO(PFNGLVALIDATEPROGRAMPROC, glValidateProgram)                                                                    \
    DO(PFNGLUSEPROGRAMPROC, glUseProgram)                                                                              \
    DO(PFNGLGETPROGRAMIVPROC, glGetProgramiv)                                                                          \
    DO(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)                                                                \
    DO(PFNGLDELETEPROGRAMPROC, glDeleteProgram)                                                                        \
    DO(PFNGLGENBUFFERSPROC, glGenBuffers)                                                                              \
    DO(PFNGLBINDBUFFERPROC, glBindBuffer)                                                                              \
    DO(PFNGLBUFFERDATAPROC, glBufferData)                                                                              \
    DO(PFNGLBUFFERSUBDATAPROC, glBufferSubData)                                                                        \
    DO(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                                                                        \
    DO(PFNGLCREATEBUFFERSPROC, glCreateBuffers)                                                                        \
    DO(PFNGLNAMEDBUFFERSTORAGEPROC, glNamedBufferStorage)                                                              \
    DO(PFNGLNAMEDBUFFERDATAPROC, glNamedBufferData)                                                                    \
    DO(PFNGLNAMEDBUFFERSUBDATAPROC, glNamedBufferSubData)                                                              \
    DO(PFNGLMAPNAMEDBUFFERPROC, glMapNamedBuffer)                                                                      \
    DO(PFNGLUNMAPNAMEDBUFFERPROC, glUnmapNamedBuffer)                                                                  \
    DO(PFNGLGETNAMEDBUFFERPARAMETERIVPROC, glGetNamedBufferParameteriv)                                                \
    DO(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)                                                                    \
    DO(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                                                                    \
    DO(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)                                                              \
    DO(PFNGLCREATEVERTEXARRAYSPROC, glCreateVertexArrays)                                                              \
    DO(PFNGLENABLEVERTEXARRAYATTRIBPROC, glEnableVertexArrayAttrib)                                                    \
    DO(PFNGLDISABLEVERTEXARRAYATTRIBPROC, glDisableVertexArrayAttrib)                                                  \
    DO(PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding)                                                  \
    DO(PFNGLVERTEXARRAYATTRIBFORMATPROC, glVertexArrayAttribFormat)                                                    \
    DO(PFNGLVERTEXARRAYATTRIBIFORMATPROC, glVertexArrayAttribIFormat)                                                  \
    DO(PFNGLVERTEXARRAYVERTEXBUFFERPROC, glVertexArrayVertexBuffer)                                                    \
    DO(PFNGLVERTEXARRAYELEMENTBUFFERPROC, glVertexArrayElementBuffer)                                                  \
    DO(PFNGLVERTEXARRAYBINDINGDIVISORPROC, glVertexArrayBindingDivisor)                                                \
    DO(PFNGLCREATETEXTURESPROC, glCreateTextures)                                                                      \
    DO(PFNGLTEXTURESTORAGE2DPROC, glTextureStorage2D)                                                                  \
    DO(PFNGLTEXTURESTORAGE3DPROC, glTextureStorage3D)                                                                  \
    DO(PFNGLTEXTURESUBIMAGE2DPROC, glTextureSubImage2D)                                                                \
    DO(PFNGLTEXTURESUBIMAGE3DPROC, glTextureSubImage3D)                                                                \
    DO(PFNGLTEXTUREPARAMETERIPROC, glTextureParameteri)                                                                \
    DO(PFNGLTEXTUREPARAMETERFPROC, glTextureParameterf)                                                                \
    DO(PFNGLGENERATETEXTUREMIPMAPPROC, glGenerateTextureMipmap)                                                        \
    DO(PFNGLBINDTEXTUREUNITPROC, glBindTextureUnit)                                                                    \
    DO(PFNGLCREATEFRAMEBUFFERSPROC, glCreateFramebuffers)                                                              \
    DO(PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, glNamedFramebufferTexture)                                                    \
    DO(PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC, glNamedFramebufferRenderbuffer)                                          \
    DO(PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC, glCheckNamedFramebufferStatus)                                            \
    DO(PFNGLCLEARNAMEDFRAMEBUFFERFVPROC, glClearNamedFramebufferfv)                                                    \
    DO(PFNGLCLEARNAMEDFRAMEBUFFERFIPROC, glClearNamedFramebufferfi)                                                    \
    DO(PFNGLCREATERENDERBUFFERSPROC, glCreateRenderbuffers)                                                            \
    DO(PFNGLNAMEDRENDERBUFFERSTORAGEPROC, glNamedRenderbufferStorage)                                                  \
    DO(PFNGLGENSAMPLERSPROC, glGenSamplers)                                                                            \
    DO(PFNGLCREATESAMPLERSPROC, glCreateSamplers)                                                                      \
    DO(PFNGLDELETESAMPLERSPROC, glDeleteSamplers)                                                                      \
    DO(PFNGLBINDSAMPLERPROC, glBindSampler)                                                                            \
    DO(PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri)                                                                \
    DO(PFNGLSAMPLERPARAMETERFPROC, glSamplerParameterf)                                                                \
    DO(PFNGLSAMPLERPARAMETERIVPROC, glSamplerParameteriv)                                                              \
    DO(PFNGLSAMPLERPARAMETERFVPROC, glSamplerParameterfv)                                                              \
    DO(PFNGLPROGRAMUNIFORM1IPROC, glProgramUniform1i)                                                                  \
    DO(PFNGLPROGRAMUNIFORM1FPROC, glProgramUniform1f)                                                                  \
    DO(PFNGLPROGRAMUNIFORM2FPROC, glProgramUniform2f)                                                                  \
    DO(PFNGLPROGRAMUNIFORM3FPROC, glProgramUniform3f)                                                                  \
    DO(PFNGLPROGRAMUNIFORM4FPROC, glProgramUniform4f)                                                                  \
    DO(PFNGLPROGRAMUNIFORMMATRIX4FVPROC, glProgramUniformMatrix4fv)                                                    \
    DO(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)                                                    \
    DO(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray)                                                  \
    DO(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)                                                            \
    DO(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)                                                              \
    DO(PFNGLUNIFORM1FPROC, glUniform1f)                                                                                \
    DO(PFNGLUNIFORM2FPROC, glUniform2f)                                                                                \
    DO(PFNGLUNIFORM3FPROC, glUniform3f)                                                                                \
    DO(PFNGLUNIFORM4FPROC, glUniform4f)                                                                                \
    DO(PFNGLUNIFORM1IPROC, glUniform1i)                                                                                \
    DO(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv)                                                                  \
    DO(PFNGLACTIVETEXTUREPROC, glActiveTexture)                                                                        \
    DO(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap)                                                                      \
    DO(PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback)                                                          \
    DO(PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl)                                                            \
    DO(PFNGLBINDBUFFERBASEPROC, glBindBufferBase)                                                                      \
    DO(PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform)                                                                  \

#define DO_DEFINE(TYPE, NAME) inline TYPE NAME;
FOR_OPENGL_FUNCTIONS(DO_DEFINE)