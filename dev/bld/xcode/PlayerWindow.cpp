#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "BaseWindow.h"
#include "PlayerWindow.h"
#include "yapt/ySystem.h"

using namespace yapt;


PlayerWindow::PlayerWindow() {
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
	system = GetYaptSystemInstance();
	// Need to have GL initalized before we can initalize the document
	// This is because a plugin that load textures should do this in the "Render" phase
	MakeCurrent();
	if (system->GetActiveDocument()) {
		system->GetActiveDocumentController()->RenderResources();
	}
}
void PlayerWindow::Prepare()
{

}
void PlayerWindow::Render()
{
	printf("Player Window: %d,%d\n",width,height);
	contextParams.width = width;
	contextParams.height = height;
	IBaseInstance *pBase = dynamic_cast<IBaseInstance *>(system->GetActiveDocument());
	IContext *pContext = pBase->GetContext();
	printf("PlayerWindowContext: %p\n", this);
	printf("DocContextInstance: %p\n", pContext);
	pContext->SetContextParamObject(&contextParams);
	system->GetActiveDocumentController()->Render(glfwGetTime());
	// Swap buffers done by base window
}

// implementation of IOpenGLContextParams
int OpenGLContextParams::GetFrameBufferWidth() {
	return width;
}
int OpenGLContextParams::GetFrameBufferHeight() {
	return height;
}

