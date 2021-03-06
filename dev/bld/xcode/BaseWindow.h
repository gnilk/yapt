#pragma once
#include <GLFW/glfw3.h>

#include <string>

class BaseWindow
{
public:
	BaseWindow();
	virtual ~BaseWindow();

	virtual void Open(int width, int height, const char *name, bool fullScreen = false);
	virtual void SetPos(int xp, int yp);
	virtual void Close();
	virtual void MakeCurrent();
	virtual void Update();
	virtual bool ShouldClose();
	virtual void OnChar(unsigned int code);
	virtual void OnKey(int key, int scancode, int action, int mods);
	virtual void OnKeyDown(int key, int scancode, int mods);
	virtual void OnKeyUp(int key, int scancode, int mods);
	virtual void OnKeyRepeat(int key, int scancode, int mods);
	virtual void SetTitle(std::string title);

	void SetPreferredMonitor(int monitor);
private:
	void CommitInputString(bool isChange);
protected:
	virtual void OnStringInputCommit(std::string input);
	virtual void OnStringInputChanged(std::string input);
	virtual void Render();
	int width, height;
	int px_width, px_height;
	GLFWwindow *window;
	std::string input;
	int preferred_monitor;

};