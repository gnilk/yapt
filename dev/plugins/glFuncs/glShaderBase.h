#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <GL/GLEW.h>

using namespace yapt;

class Shader;
class ShaderProgram;

class OpenGLShaderBase : public IFileWatcher {

public:
	OpenGLShaderBase();
	virtual ~OpenGLShaderBase();
	void Initialize(ISystem *ySys, std::string vertexshader, std::string fragmentshader);
	bool ReloadIfNeeded(bool forcereload = false);
	virtual void OnFileChanged(const char *filename);	
	ShaderProgram *LoadShaders();

	void Attach();
	void Detach();
	ShaderProgram *GetProgram();
private:
	ISystem *ySys;
	bool reloadshaders;
	std::string vertexshader;
	std::string fragmentshader;
protected:
	ShaderProgram *program;
};

class Shader {
private:
	unsigned int idShader;
	GLenum type;
	bool loaded;
public:
	Shader();
	virtual ~Shader();
	void Dispose();
	bool Load(char *src, GLenum type);
	bool IsLoaded() { return loaded; }
	unsigned int ShaderId() { return idShader; }
};

class ShaderProgram {
private:
	bool loaded;
	unsigned int idProgram;
	Shader vertexShader, fragmentShader;
public:
	ShaderProgram();
	void Dispose();
	bool IsLoaded() { return loaded; };
	bool Load(char *vsSrc, char *psSrc);
	void Attach();
	void Detach();
	unsigned int ProgramId() { return idProgram; }
	unsigned int GetUniform(const char *name);
	unsigned int GetAttrib(const char *name);

};
