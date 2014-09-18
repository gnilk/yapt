#include <stdio.h>
#include <stdlib.h>
#include <GL/GLEW.h>
#include <GLFW/glfw3.h>

#include "BaseWindow.h"
#include "PlayerWindow.h"
#include "yapt/ySystem.h"

using namespace yapt;

static void perror() {
	kErrorClass eClass;
	kError eCode;
	char estring[256];

	GetYaptLastError(&eClass, &eCode);
	GetYaptErrorTranslation(estring, 256);
	Logger::GetLogger("main")->Error("(%d:%d); %s\n", eClass, eCode, estring);
	exit(1);
}


PlayerWindow::PlayerWindow() {
	pausePlayer = false;
}
PlayerWindow::~PlayerWindow() {
}

bool PlayerWindow::ShouldClose()
{
	bool res = BaseWindow::ShouldClose();
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) res = true;	
	return res;
}
void PlayerWindow::InitalizeYapt()
{
	//Update();
	ILogger *pLogger = Logger::GetLogger("PlayerWindow");

	system = GetYaptSystemInstance();
	// Need to have GL initalized before we can initalize the document
	// This is because a plugin that load textures should do this in the "Render" phase
	MakeCurrent();
	
	glEnable(GL_DEPTH_TEST);	// should always be enabled

	pLogger->Debug("Initialize glew");
	GLenum glew_status = glewInit();
	if (GLEW_OK != glew_status) {
		pLogger->Error("GLEW-Error: %s\n", glewGetErrorString(glew_status));
		return;
	}

	// if (glewIsSupported("GL_VERSION_3_3"))
	// 	pLogger->Debug("Ok, Open GL version 3.3 supported");
	// else {
	// 	printf("OpenGL 3.3 not supported\n");
	// 	exit(1);
	// }


	if (!GLEW_VERSION_2_0) {
		pLogger->Error("No support for OpenGL 2.0 found\n");
		return;
	}
}
void PlayerWindow::Prepare()
{

}
void PlayerWindow::Render()
{
	contextParams.width = width;
	contextParams.height = height;
	IBaseInstance *pBase = dynamic_cast<IBaseInstance *>(system->GetActiveDocument());
	IContext *pContext = pBase->GetContext();
	pContext->PushContextParamObject(&contextParams,"RenderContext");
	float tRender = glfwGetTime();
	if (pausePlayer) {
		tRender = tPause;		
	}
	system->GetActiveDocumentController()->Render(tRender);
	pContext->PopContextParamObject();	// Need to pop
	// Swap buffers done by base window
}

void PlayerWindow::OnKeyDown(int key, int scancode, int mods) {
	BaseWindow::OnKeyDown(key, scancode, mods);
	float dt = 1;	// default is jump one second
	if ((mods & GLFW_MOD_ALT)) {
		dt = 10;		// jump 10 seconds if ALT is pressed
	}
	switch(key) {
		case GLFW_KEY_SPACE :	// pause replay
			if (pausePlayer) {
				glfwSetTime(tPause);
				pausePlayer = false;
			}
			else {
				tPause = glfwGetTime();
				pausePlayer = true;
			}
			break;
		case GLFW_KEY_LEFT :	// fast forward in time
			{				
				if (!pausePlayer) {
					float nt = glfwGetTime() - dt;
					if (nt < 0) nt = 0;
					glfwSetTime(nt); 					
				} else {
					tPause -= dt;
					if (tPause < 0) tPause = 0;
				}

			}
			break;
		case GLFW_KEY_RIGHT :	// reverse time
			{
				if (!pausePlayer) {
					float nt = glfwGetTime() + dt;
					glfwSetTime(nt); 									
				} else {
					tPause += dt;
				}
			}
			break;
	} // switch
}

void PlayerWindow::OnKeyUp(int key, int scancode, int mods) {
	BaseWindow::OnKeyUp(key, scancode, mods);
}



// implementation of IOpenGLContextParams
int OpenGLRenderContextParams::GetFrameBufferWidth() {
	return width;
}
int OpenGLRenderContextParams::GetFrameBufferHeight() {
	return height;
}
