#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_set>


#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"

using namespace yapt;


class Bitmap {
private:
	int width;
	int height;
	unsigned char *buffer;
public:
	Bitmap(int w, int h, unsigned char *buffer);
	virtual ~Bitmap();	

	int Width() { return width; }
	int Height() { return height; }
	unsigned char *Buffer() { return buffer; }
};

class FontChar {
private:
	int character;
	int bitmap_left;
	int bitmap_top;
	int advance_x;
	int advance_y;
	Bitmap *bitmap;		
public:
	FontChar(int character, int left, int top, int ax, int ay, Bitmap *bitmap);
	virtual ~FontChar();

	void GenerateTexture();
	void Bind();
	void Draw(float x, float y, float sx, float sy);
	int AdvanceX() { return advance_x; }
	int AdvanceY() { return advance_y; }

private:

	unsigned int idTexture;	
};

class Font {
private:
	std::string family;
	int size;
	FT_Face face; 
	int numChars;
	float sx, sy;
	//FontChar *chars;
	FontChar *characters[256];;
public:
	Font(std::string family, int size, FT_Face face);
	void Build();
	void BuildTexturesForString(std::string &string);


	int Count() { return numChars; }
	void AddChar(int character, FontChar *fc);
	FontChar *GetChar(int character);
	// render stuff
	void Bind(int character);
	void SetRenderScaling(float sx, float sy);
	void Draw(int character, float x, float y);
	void Draw(std::string str, float x, float y);
	void Estimate(std::string str, float *out_width, float *out_height);
	int Size() { return size; }
	std::string Family() { return family; }
};

class FontManager
{
private:
	static FontManager *instance;
	std::vector<Font *> fonts;
	FT_Library ft;
private:
	FontManager();
public:
	static FontManager *GetInstance();
	Font *GetFont(std::string family, int size);
};


class OpenGLDrawText: public PluginObjectImpl, public OpenGLShaderBase {
private:
	// input
	Property *font;
	Property *fontSize;
	Property *text;
	Property *vshader;
	Property *fshader;
	Property *alignment;
	Property *position;
	bool useShaders;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	Font *textureFont;
};
