#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include <string>

#include "BaseWindow.h"


BaseWindow::BaseWindow() {
	window = NULL;
	preferred_monitor = 0;
}
BaseWindow::~BaseWindow() {

}

void BaseWindow::SetPreferredMonitor(int monitor) {
	preferred_monitor = monitor;
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
void BaseWindow::Open(int width, int height, const char *name, bool fullScreen /* = false */) {
	this->width = width;
	this->height = height;

	int count;
	GLFWmonitor **monitors = glfwGetMonitors(&count);
	for(int i=0;i<count;i++) {
		int wmm, hmm;
		printf("Monitor: %d\n", i);
		const GLFWvidmode *currentmode = glfwGetVideoMode(monitors[i]);
		glfwGetMonitorPhysicalSize(monitors[i], &wmm, &hmm);

		const double dpi = currentmode->width / (wmm / 25.4);
		printf("  Width(mm) : %d\n", wmm);
		printf("  Height(mm): %d\n", hmm);
		printf("  DPI.......: %f\n", dpi);
		printf("  Resolution: %d x %d at %dhz\n", currentmode->width, currentmode->height, currentmode->refreshRate);
	}
 
	if (!fullScreen) {
		printf("Creating window, resolution: %d x %d\n", width, height);
		window = glfwCreateWindow(width, height, name, NULL, NULL);
	} else {
		GLFWmonitor *usemon = glfwGetPrimaryMonitor();

		if (preferred_monitor <= count) {
			printf("Selecting monitor: %d\n", preferred_monitor);
			usemon = monitors[preferred_monitor];
		}
		// if (count > 1) {
		//  	usemon = monitors[1];
		// }

		const GLFWvidmode *currentmode = glfwGetVideoMode(usemon);
		// in fullscreen w=0, h=0 means use default monitor resolution
		if ((width==0) || (height==0)) {
			width = currentmode->width;
			height = currentmode->height;
		}


		printf("Creating window, resolution: %d x %d\n", width, height);
		glfwWindowHint(GLFW_REFRESH_RATE, currentmode->refreshRate);
		window = glfwCreateWindow(width, height, name, usemon, NULL);
		//glfwSetWindowSize(window, width, height);

		const GLFWvidmode *selectedMode = glfwGetVideoMode(usemon);
		printf("Mode is: %d x %d at %dhz\n", selectedMode->width, selectedMode->height, selectedMode->refreshRate);

	}
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window\n");
		return;
	}
	glfwSetWindowUserPointer(window, (void *)this);
	glfwSetCharCallback(window, glfwOnChar);
	glfwSetKeyCallback(window, glfwOnKey);
}
void BaseWindow::SetTitle(std::string title) {
	glfwSetWindowTitle(window, title.c_str());
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
	glfwGetFramebufferSize(window, &px_width, &px_height);
}
void BaseWindow::Update() {

	MakeCurrent();
	glViewport(0, 0, px_width, px_height);	
	Render();
	glfwSwapBuffers(window);
}

void BaseWindow::Render()
{
	// TODO: Override and implement
}


