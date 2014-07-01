#pragma once
#include <GLFW/glfw3.h>
#include "BaseWindow.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "yapt/logger.h"
#include "yapt/ySystem.h"

#include <string>
#include <vector>


using namespace yapt;

class IConsoleWatch;

class IConsole {
public:
	virtual void WriteLine(const char *string) = 0;
	virtual void WriteLine(std::string string) = 0;
	virtual void AddWatch(IConsoleWatch *pWatch) = 0;
};

class IConsoleCommandHandler 
{
public:
	virtual bool Execute(IConsole *pConsole, std::string raw, std::vector<std::string> arguments) = 0;
};
typedef IConsoleCommandHandler *(*CONSOLE_PFNCOMMANDACTORY)();
typedef struct
{
	std::string name;
	CONSOLE_PFNCOMMANDACTORY factory;
	
} CONSOLE_COMMAND;


class ConsoleCommandFactory {
public:
	static IConsoleCommandHandler *GetInstance(std::string command);
private:
};

class IConsoleWatch
{
public:
	virtual std::string GetName() = 0;
	virtual std::string GetValue() = 0;
	virtual void GetMetaData(std::vector<std::string> &metadata) = 0;
};

class ConsolePropertyWatcher : public IConsoleWatch
{
public:
	ConsolePropertyWatcher(IBaseInstance *pObject);
	virtual ~ConsolePropertyWatcher();
	virtual std::string GetName();
	virtual std::string GetValue();
	virtual void GetMetaData(std::vector<std::string> &metadata);


private:
	IBaseInstance *pObject;
	IPropertyInstance *pProp;
};


class ConsoleWindow : public BaseWindow, public IConsole
{
public:
	ConsoleWindow();
	virtual ~ConsoleWindow();
	bool InitializeFreeType();

	virtual void WriteLine(const char *string);
	virtual void WriteLine(std::string string);
	virtual void AddWatch(IConsoleWatch *pWatch);
protected:
	virtual void Render();
	virtual void OnStringInputChanged(std::string input);
	virtual void OnStringInputCommit(std::string input);

	void ProcessStringCommand(std::string input);

	void RenderWatchers(float sx, float sy);
	void RenderHistoryBuffer(float sx, float sy);
	void RenderDividers();

private:
	bool IsDirty();
	void SetDirty(bool bIsDirty);
	void Prepare();
	void RenderText(const char *text, float x, float y, float sx, float sy);
	// TODO: Move to shader help
	GLuint CreateProgram(const char *vertexsource, const char *fragmentsource);
	GLuint CreateShader(const char* source, GLenum type);
	GLint GetAttrib(GLuint program, const char *name);
	GLint GetUniform(GLuint program, const char *name);
	void PrintLog(GLuint object);

	std::vector<std::string> inputHistoryBuffer;
	std::vector<IConsoleWatch *>watchList;

	int fontSize;
	int watchScreenHeight;

	FT_Library ft;
	FT_Face face; 
	GLuint program;
	GLint uniform_color;
	GLint uniform_tex;
	GLint attribute_coord;

	GLuint vbo;

	std::string consoleInputString;

	bool dirty;
	yapt::ILogger *pLogger;

};