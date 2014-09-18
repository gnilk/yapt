#include "glShaderBase.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <math.h>

//#include <gl/glew.h>
#include <GL/GLEW.h>
//#include <OpenGl/glu.h>


using namespace yapt;

OpenGLShaderBase::OpenGLShaderBase() {
	reloadshaders = false;
	program = NULL;
}

OpenGLShaderBase::~OpenGLShaderBase() {

}

void OpenGLShaderBase::Initialize(ISystem *ySys, std::string vertexshader, std::string fragmentshader) {
	this->vertexshader = vertexshader;
	this->fragmentshader = fragmentshader;
	this->ySys = ySys;
}

bool OpenGLShaderBase::ReloadIfNeeded(bool forcereload /* = false */) {
	bool success = true;
	if (reloadshaders || forcereload) {
	    ShaderProgram *newProgram = LoadShaders();
	    if (newProgram != NULL) {
			ySys->GetLogger("GLShaderBase")->Debug("Ok, swapping shaders");
			// avoid delete on first call
			if (program != NULL) {
				delete program;				
			}
			program = newProgram;
		} else {
			ySys->GetLogger("GLShaderBase")->Debug("Failed to load new shader, keeping old");
			success = false;
	    }
		reloadshaders = false;
	}
	return success;
}

void OpenGLShaderBase::OnFileChanged(const char *filename) {
	reloadshaders = true;
}

ShaderProgram *OpenGLShaderBase::LoadShaders() {
  ShaderProgram *program = new ShaderProgram();
  long psz,vsz;

  const char *fname_vshader = vertexshader.c_str();
  const char *fname_pshader = fragmentshader.c_str();

  ySys->GetLogger("GLShaderQuad")->Debug("Loading shader files");
  void *psSrc = ySys->LoadData(fname_pshader,0,&psz);
  void *vsSrc = ySys->LoadData(fname_vshader,0,&vsz);

  if (!reloadshaders) {
    // Register files for notification when changed - only first time
    IFileWatcher *sink = dynamic_cast<IFileWatcher *>(this);
    ySys->WatchFile(fname_vshader, sink);
    ySys->WatchFile(fname_pshader, sink);
  } 

  // Needed?????
  //ySys->GetLogger("GLShaderQuad")->Debug("Loaded shaders to GL, ps=%p, vs=%p", psSrc, vsSrc);
  ((char *)psSrc)[psz-1]='\0';
  ((char *)vsSrc)[vsz-1]='\0';
  //ySys->GetLogger("GLShaderQuad")->Debug("VertexShader (bytes=%d)\n%s",vsz, vsSrc);
  //ySys->GetLogger("GLShaderQuad")->Debug("PixelShader (bytes=%d)\n%s",psz, psSrc);

  if (!program->Load((char *)vsSrc, (char *)psSrc)) {
    printf("program error\n");
    return NULL;
  }
  ySys->GetLogger("GLShaderQuad")->Debug("Shaders loaded");
  return program;
}

void OpenGLShaderBase::Attach() {
	if (program != NULL) {
		program->Attach();
	}
}
void OpenGLShaderBase::Detach() {
	if (program != NULL) {
		program->Detach();
	}
}
ShaderProgram *OpenGLShaderBase::GetProgram() {
	return program;
}


static void PrintLog(GLuint object)
{
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }

  char* log = (char*)malloc(log_length);

  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);

  fprintf(stderr, "%s", log);
  free(log);
}


Shader::Shader() {
  loaded = false;
}

Shader::~Shader() {
  Dispose();
}


void Shader::Dispose() {
  if (loaded) {
    glDeleteShader(idShader);    
    loaded = false;
  }
}

bool Shader::Load(char *src, GLenum type) {
  if (loaded) return true;

  printf("Create %s\n",(type==GL_VERTEX_SHADER)?"Vertex Shader":"Pixel Shader");
  idShader = glCreateShader(type);
  glShaderSource(idShader, 1, (const GLchar **)&src, NULL);
  glCompileShader(idShader);

  int compilationStatus;
  glGetShaderiv(idShader, GL_COMPILE_STATUS, &compilationStatus);

  if (compilationStatus == GL_FALSE) {
    if (type == GL_VERTEX_SHADER) {
      printf("VERTEX_SHADER\n");
    } else {
      printf("PIXEL_SHADER\n");
    }
    PrintLog(idShader);
    return false;
  }

  this->type = type;
  loaded = true;

  return 1; 
}

ShaderProgram::ShaderProgram() {
  loaded = false;
}

void ShaderProgram::Dispose() {
  if (loaded) {
    vertexShader.Dispose();
    fragmentShader.Dispose();
    glDeleteProgram(idProgram);      
  }
}

bool ShaderProgram::Load(char *vsSrc, char *psSrc) {

  Dispose();

  idProgram = glCreateProgram();

  if (!vertexShader.Load(vsSrc, GL_VERTEX_SHADER)) return false;
  if (!fragmentShader.Load(psSrc, GL_FRAGMENT_SHADER)) return false;
  glAttachShader(idProgram, vertexShader.ShaderId());
  glAttachShader(idProgram, fragmentShader.ShaderId());

  glLinkProgram(idProgram);
  GLint link_ok = GL_FALSE;
  glGetProgramiv(idProgram, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    PrintLog(idProgram);
    glDeleteProgram(idProgram);
    return false;
  }

  loaded = true;
  return true;
}

unsigned int ShaderProgram::GetAttrib(const char *name) {
  GLint attribute = glGetAttribLocation(idProgram, name);
  if(attribute == -1) {
//    fprintf(stderr, "Could not bind attribute %s\n", name);
  }
  return attribute;
}

unsigned int ShaderProgram::GetUniform(const char *name) {
  GLint uniform = glGetUniformLocation(idProgram, name);
  if(uniform == -1) {
   // fprintf(stderr, "Could not bind uniform %s\n", name);

  }
  return uniform;
}

void ShaderProgram::Attach() {
  glUseProgram(idProgram);
}

void ShaderProgram::Detach() {
  glUseProgram(0);
}

