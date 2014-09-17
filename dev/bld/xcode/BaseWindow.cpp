#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include <string>

#include "BaseWindow.h"


BaseWindow::BaseWindow() {
	window = NULL;
}
BaseWindow::~BaseWindow() {

}

static void glfwOnChar(GLFWwindow *window, unsigned int code) {
	//printf("glfwOnChar, %c\n",code);
	BaseWindow *pThis = (BaseWindow *)glfwGetWindowUserPointer(window);
	if (pThis != NULL) {
		pThis->OnChar(code);
	}
}
static void glfwOnKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
	//printf("glfwOnKey: key:%d, scancode: %d, action: %d, mods: %d\n", key, scancode, action, mods);
	BaseWindow *pThis = (BaseWindow *)glfwGetWindowUserPointer(window);
	if (pThis != NULL) {
		pThis->OnKey(key, scancode, action, mods);
	}
}
void BaseWindow::Open(int width, int height, const char *name) {
	this->width = width;
	this->height = height;

	window = glfwCreateWindow(width, height, name, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window\n");
		return;
	}
	glfwSetWindowUserPointer(window, (void *)this);
	glfwSetCharCallback(window, glfwOnChar);
	glfwSetKeyCallback(window, glfwOnKey);
}

void BaseWindow::OnChar(unsigned int code) {
	input += code;
	CommitInputString(true);
}

void BaseWindow::OnKey(int key, int scancode, int action, int mods) {
	//printf("OnKey: key:%d, scancode: %d, action: %d, mods: %d\n", key, scancode, action, mods);
	switch(action) {
		case GLFW_PRESS   : OnKeyDown(key, scancode, mods); break;
		case GLFW_RELEASE : OnKeyUp(key, scancode, mods); break;
		case GLFW_REPEAT  : OnKeyDown(key, scancode, mods); break;
	}
}

void BaseWindow::OnKeyDown(int key, int scancode, int mods) {
	switch(key) {
		case GLFW_KEY_ENTER : 	// Send input string
		CommitInputString(false);
		break;
		case GLFW_KEY_BACKSPACE :	// Delete last char
		if (input.size() > 0) {
			input.erase(input.end()-1);
		}
		CommitInputString(true);
		break;
	}
}

void BaseWindow::OnKeyUp(int key, int scancode, int mods) {

}
void BaseWindow::OnKeyRepeat(int key, int scancode, int mods) {

}

void BaseWindow::CommitInputString(bool isChange) {
	if (isChange) {
		OnStringInputChanged(input);
	} else {
		OnStringInputCommit(input);
		input = "";
	}
}

void BaseWindow::OnStringInputChanged(std::string input) {
//	printf("CHANGE STRING: %s\n",input.c_str());
}

void BaseWindow::OnStringInputCommit(std::string input)
{
//	printf("COMMIT STRING: %s\n",input.c_str());
}



void BaseWindow::Close() {
	glfwDestroyWindow(window);
}

bool BaseWindow::ShouldClose() {
	return glfwWindowShouldClose(window);
}
void BaseWindow::SetPos(int xp, int yp)
{
	glfwSetWindowPos(window, xp, yp);
}
void BaseWindow::MakeCurrent()
{
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &width, &height);
}
void BaseWindow::Update() {

	MakeCurrent();
	glViewport(0, 0, width, height);	
	Render();
	glfwSwapBuffers(window);
}

void BaseWindow::Render()
{
	// TODO: Override and implement
}


