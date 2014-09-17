#pragma once
#include <GLFW/glfw3.h>
#include "BaseWindow.h"
#include "yapt/ySystem.h"


using namespace yapt;

class OpenGLRenderContextParams : public IRenderContextParams
{
public:	// interface
	virtual int GetFrameBufferWidth();
    virtual int GetFrameBufferHeight();
public:
	int width, height;
};

class PlayerWindow : public BaseWindow
{
public:
	PlayerWindow();
	virtual ~PlayerWindow();

	virtual bool ShouldClose();
	void InitalizeYapt();
	virtual void OnKeyDown(int key, int scancode, int mods);
	virtual void OnKeyUp(int key, int scancode, int mods);

protected:
	virtual void Render();
private:
	void Prepare();
	float tLast;
	yapt::ISystem *system;
	OpenGLRenderContextParams contextParams;
	bool pausePlayer;


};