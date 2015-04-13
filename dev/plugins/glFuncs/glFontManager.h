//
// Generate bitmap fonts for use with OpenGL texturing
//
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_set>


#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"
#include "Bitmap.h"

using namespace yapt;


class Font;

class OpenGLFontLoader: public PluginObjectImpl {
private:
	// input
	Property *fontName;
	Property *fontSize;
	Property *outputFontObject;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};



// Font manager has a 'cache' of already generated fonts
class FontManager
{
private:
	static FontManager *instance;
	std::vector<Font *> fonts;
	FT_Library ft;
private:
	FontManager();
public:
	static FontManager *GetInstance(ISystem *ysys);
	Font *GetFont(std::string family, int size);
	Font *LoadBitmapFont(std::string filename);
	Font *LoadTrueTypeFont(std::string filename, int size);
private:
	Font *GetFromCache(std::string name, int size);
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

class BitmapCharDef {
private:
	int ch;
	int used;
	int x,y,width,height;
public:
	BitmapCharDef(int ch) {
		this->used = 0;
        this->ch = ch;
	}
    void SetPos(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->width = w;
        this->height = h;
        this->used = 1;
    }
    int X() {
        return x;
    }
    int Y() {
        return y;
    }
    int Width() {
        return width;
    }
    int Height() {
        return height;
    }
	bool Used() {
		return this->used==1?true:false;
	}
};


class Font {
private:
	std::string family;
	int size;
	FT_Face face; 
	int numChars;
	//FontChar *chars;
	FontChar *characters[256];;
protected:
	float sx, sy;
	bool isBuilt;
public:
	Font(std::string family);
	Font(std::string family, int size, FT_Face face);
	virtual void Build();

	void BuildTexturesForString(std::string &string);

	int Count() { return numChars; }
	void AddChar(int character, FontChar *fc);
	FontChar *GetChar(int character);
	// render stuff
	void Bind(int character);
	void SetRenderScaling(float sx, float sy);
	virtual FontChar *Draw(int character, float x, float y);
	virtual void Draw(std::string str, float x, float y);
	virtual void Estimate(std::string str, float *out_width, float *out_height);
	virtual int Size() { return size; }
	virtual std::string Family() { return family; }
};


class BitmapFont : public Font {
	std::string filename;
	Bitmap *bitmap;	// raw bitmap
	BitmapCharDef *chardefs[256];	// induvidual bitmaps per char

public:
	BitmapFont(std::string filename);
	virtual void Build();
	virtual FontChar *Draw(int character, float x, float y);
	virtual void Draw(std::string str, float x, float y);
	virtual void Estimate(std::string str, float *out_width, float *out_height);
private:
	bool LoadFontMap();

};
