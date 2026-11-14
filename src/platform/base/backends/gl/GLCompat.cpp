#include <Platform/GLCompat.h>

#if defined(_WIN32)

PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
PFNGLCLIPCONTROLPROC glClipControl = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;

bool Platform_LoadGLFunctions(void* (*getProcAddress)(const char*))
{
	if (!getProcAddress) return false;

	glActiveTexture = (PFNGLACTIVETEXTUREPROC)getProcAddress("glActiveTexture");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)getProcAddress("glClientActiveTexture");
	glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)getProcAddress("glCompressedTexImage2D");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getProcAddress("glBindFramebuffer");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getProcAddress("glGenFramebuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)getProcAddress("glDeleteFramebuffers");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)getProcAddress("glCheckFramebufferStatus");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getProcAddress("glFramebufferTexture2D");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)getProcAddress("glFramebufferRenderbuffer");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)getProcAddress("glBindRenderbuffer");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)getProcAddress("glGenRenderbuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)getProcAddress("glDeleteRenderbuffers");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)getProcAddress("glRenderbufferStorage");
	glClipControl = (PFNGLCLIPCONTROLPROC)getProcAddress("glClipControl");
	glCreateShader = (PFNGLCREATESHADERPROC)getProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)getProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProcAddress("glGetShaderInfoLog");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)getProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)getProcAddress("glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getProcAddress("glGetProgramInfoLog");
	glUseProgram = (PFNGLUSEPROGRAMPROC)getProcAddress("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getProcAddress("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)getProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)getProcAddress("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FPROC)getProcAddress("glUniform3f");
	glUniform1i = (PFNGLUNIFORM1IPROC)getProcAddress("glUniform1i");
	glDeleteShader = (PFNGLDELETESHADERPROC)getProcAddress("glDeleteShader");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)getProcAddress("glDeleteProgram");

	return glActiveTexture && glClientActiveTexture && glCompressedTexImage2D &&
		   glBindFramebuffer && glGenFramebuffers && glDeleteFramebuffers &&
		   glCheckFramebufferStatus && glFramebufferTexture2D && glFramebufferRenderbuffer &&
		   glBindRenderbuffer && glGenRenderbuffers && glDeleteRenderbuffers &&
		   glRenderbufferStorage && glClipControl &&
		   glCreateShader && glShaderSource && glCompileShader && glGetShaderiv &&
		   glGetShaderInfoLog && glCreateProgram && glAttachShader && glLinkProgram &&
		   glGetProgramiv && glGetProgramInfoLog && glUseProgram && glGetUniformLocation &&
		   glUniform1f && glUniform2f && glUniform3f && glUniform1i && glDeleteShader &&
		   glDeleteProgram;
}

#endif // _WIN32
