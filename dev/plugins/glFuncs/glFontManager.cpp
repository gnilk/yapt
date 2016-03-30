#include "glShaderBase.h"
#include "glFontManager.h"

#include "StringUtil.h"

#include "Bitmap.h"


#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <vector>
#include <fstream>
#include <math.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <OpenGl/glu.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

// Supplied to GetInstance set on first access only
static ISystem *psys = NULL;

// static
FontManager *FontManager::instance = NULL;

void OpenGLFontLoader::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	fontName = pInstance->CreateProperty("fontname", kPropertyType_String, "Arial.ttf", "");
	fontSize = pInstance->CreateProperty("fontsize", kPropertyType_Integer, "24", "");
	outputFontObject = pInstance->CreateOutputProperty("font",kPropertyType_UserPtr,NULL,"");
}

void OpenGLFontLoader::Render(double t, IPluginObjectInstance *pInstance) {

}

void OpenGLFontLoader::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	Font *font;
	font = FontManager::GetInstance(ySys)->GetFont(std::string(fontName->v->string), fontSize->v->int_val);
	//textureFont = FontManager::GetInstance(ySys)->LoadBitmapFont(std::string("fontmap.fnt"));
	//new Font(std::string(font->v->string), fontSize->v->int_val, face);
	font->Build();
	outputFontObject->v->userdata = font;


}

void OpenGLFontLoader::PostRender(double t, IPluginObjectInstance *pInstance) {

}



FontManager::FontManager() {
	if(FT_Init_FreeType(&ft)) {
  		psys->GetLogger("FontManager")->Error("OpenGLDrawText, Could not init freetype library");
  		exit(1);
  	}
}

FontManager *FontManager::GetInstance(ISystem *ysys) {
	if (instance == NULL) {
		::psys = ysys;
		instance = new FontManager();
	}
	return instance;
}

static bool StrEnds (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

Font *FontManager::GetFont(std::string family, int size) {

	Font *font = NULL;
	if (StrEnds(family, std::string(".fnt"))) {
		font = LoadBitmapFont(family);
	} else {
		font = LoadTrueTypeFont(family, size);	
	}

	return font;
}

Font *FontManager::LoadBitmapFont(std::string filename) {
	Font *font = GetFromCache(filename, 0);	// bitmap font's have no size
	if (font != NULL) return font;

	psys->GetLogger("FontManager")->Debug("Loading bitmap font '%s'", filename.c_str());

	font = new BitmapFont(filename);
	font->Build();
	fonts.push_back(font);

	return font;
}
Font *FontManager::LoadTrueTypeFont(std::string family, int size) {
	Font *font = GetFromCache(family, size);

	if (font != NULL) return font;

	psys->GetLogger("FontManager")->Debug("Building font '%s' with size %d (not cached), building",family.c_str(), size);
	// not cached, create new and cache
  	FT_Face face;
	if(FT_New_Face(ft, family.c_str(), 0, &face)) {
		psys->GetLogger("FontManager")->Error("OpenGLDrawText, Could not open font");
  		exit(1);
	}
	FT_Set_Pixel_Sizes(face, 0, size);

	font = new Font(family, size, face);
	font->Build();
	fonts.push_back(font);

	return font;
}

Font *FontManager::GetFromCache(std::string name, int size) {
	// Look through cache
	for(int i=0;i<fonts.size();i++) {
		if ((name == fonts[i]->Family()) && (size == fonts[i]->Size())) {
			psys->GetLogger("FontManager")->Debug("Found font '%s' with size %d in cached",name.c_str(), size);
			return fonts[i];
		}
	}
	return NULL;	
}


//
// -- Bitmap font
//

BitmapFont::BitmapFont(std::string filename) :
	Font(filename)
{
	this->filename = filename;
	for(int i=0;i<256;i++) {
		chardefs[i] = new BitmapCharDef(i);
	}
	bitmap = NULL;

}

void BitmapFont::Build() {

	if (isBuilt) return;

	if (!LoadFontMap()) {
		psys->GetLogger("BitmapFont")->Error("Failed to load font map");
		return;
	}

	for(int i=0;i<256;i++) {
		if (chardefs[i]->Used()) {
			BitmapCharDef *character = chardefs[i];

			psys->GetLogger("BitmapFont")->Debug("Char '%c' (X:%d, Y:%d), (W:%d, H:%d)",i,character->X(), character->Y(), character->Width(), character->Height());
			Bitmap *bmpChar = bitmap->CopyToNew(character->X(), character->Y(), character->Width(), character->Height());

			FontChar *fc = new FontChar(i, 0, 0, bmpChar->Width(), bmpChar->Height(), bmpChar);
			//FontChar *fc = new FontChar(i, 0, 0, bitmap->Width(), bitmap->Height(), bitmap);
			fc->GenerateTexture();
			AddChar(i, fc);
		}
	}

	isBuilt = true;
}

FontChar *BitmapFont::Draw(int character, float x, float y) {
	FontChar *c = GetChar(character);

	if (c != NULL) {
		c->Draw(x,y,sx,sy);
	} else {
		//printf("not found\n");
	}
	return c;

}
void BitmapFont::Draw(std::string str, float x, float y) {

	for(int i=0;i<str.length();i++) {
		FontChar *c = GetChar(str.at(i));
		if (!c)	{
			continue;
		}
		//printf("%c at %f,%f\n",str.at(i),x,y);
		c->Draw(x,y,sx,sy);
		x+=c->AdvanceX() * sx;
		//y+=c->AdvanceY();
	}
}

void BitmapFont::Estimate(std::string str, float *out_width, float *out_height) {
	float x,y;
	x = y = 0;
	for(int i=0;i<str.length();i++) {
		FontChar *c = GetChar(str.at(i));
		if (!c)	continue;
		x+=c->AdvanceX() * sx;
		y =c->AdvanceY() * sy;
	}
	*out_width = x;
	*out_height = y;
}

bool BitmapFont::LoadFontMap() {
    const char *fn = filename.c_str();
    FILE *f = fopen(fn, "rb");			// TODO: Replace with noice.io
    if (f == NULL) {
    	psys->GetLogger("BitmapFont")->Error("Unable to open file '%s'",fn);
    	return false;
    }

	bool result = true;
    char tmp[256];
    while (!feof(f)) {
        fgets(tmp, 256, f);
        std::vector<std::string> tokens;
        StringUtil::Split(tokens, tmp, ',');
        if (tokens[0] == std::string("image")) {
            // image
            bitmap = Bitmap::LoadPNGImage(tokens[1]);
            // should probably be optional..
            bitmap->CreateAlphaMask(0,255);
            if (bitmap == NULL) {
                //qDebug() << "Failed to load image file: " << tokens[1].c_str();
                psys->GetLogger("BitmapFont")->Error("Unable to open image file '%s'",tokens[1].c_str());
                result = false;
                break;
            }
            //qDebug() << "Ok, Loaded Font Map Image";
        } else if (tokens[0] == std::string("char")) {
            // char mapping
            int ch = atoi(tokens[1].c_str());
            int x = atoi(tokens[2].c_str());
            int y = atoi(tokens[3].c_str());
            int w = atoi(tokens[4].c_str());
            int h = atoi(tokens[5].c_str());

            chardefs[ch]->SetPos(x,y,w,h);
            //qDebug() << "Ok, Mapped char " << ch;
        }
    }
    fclose(f);
    return result;
}





//
// -- Font
//
Font::Font(std::string family) {
	this->family = family;
	this->size = 0;

	for(int i=0;i<256;i++) {
		characters[i] = NULL;
	}
	isBuilt = false;
	this->sx = this->sy = 1;

}
Font::Font(std::string family, int size, FT_Face face) {
	this->family = family;
	this->size = size;
	this->face = face;

	for(int i=0;i<256;i++) {
		characters[i] = NULL;
	}
	isBuilt = false;
}

void Font::Build() {

	if (isBuilt) return;

	static std::string lowerCase="abcdefghijklmnopqrstuvwxyzåäö";
	static std::string upperCase="ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ ";
	static std::string otherChars="01234567890!@#$\'\"";

	BuildTexturesForString(lowerCase);
	BuildTexturesForString(upperCase);
	BuildTexturesForString(otherChars);

	isBuilt = true;
}

void Font::BuildTexturesForString(std::string &string) {
	for (int i=0;i<string.length();i++) {
		if (FT_Load_Char(face, string.at(i), FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot g = face->glyph;

		int character = string.at(i);
		Bitmap *bmp = new Bitmap(g->bitmap.width, g->bitmap.rows, g->bitmap.buffer);
		FontChar *fc = new FontChar(character, g->bitmap_left, g->bitmap_top, g->advance.x, g->advance.y, bmp);
		fc->GenerateTexture();
		AddChar(character, fc);
	}
}

void Font::AddChar(int character, FontChar *fc) {
	if ((character >= 0) && (character < 256)) {
		characters[character] = fc;
	}
	numChars++;
}

FontChar *Font::GetChar(int character) {
	if ((character >= 0) && (character < 256)) {
		return characters[character];
	}
	return NULL;
}

void Font::Bind(int character) {
	FontChar *c = GetChar(character);
	if (c != NULL) {
		c->Bind();
	}
}

void Font::SetRenderScaling(float sx, float sy) {
	this->sx = sx;
	this->sy = sy;
}

FontChar *Font::Draw(int character, float x, float y) {
	FontChar *c = GetChar(character);
	if (c != NULL) {
		c->Draw(x,y,sx,sy);
	} else {
		//printf("not found\n");
	}
	return c;
}
void Font::Draw(std::string str, float x, float y) {
	for(int i=0;i<str.length();i++) {
		FontChar *c = GetChar(str.at(i));
		if (!c)	continue;

		c->Draw(x,y,sx,sy);
		x+=(c->AdvanceX() >> 6) * sx;
		y+=(c->AdvanceY() >> 6) * sy;
	}
}
void Font::Estimate(std::string str, float *out_width, float *out_height) {
	float x,y;
	x = y = 0;
	for(int i=0;i<str.length();i++) {
		FontChar *c = GetChar(str.at(i));
		if (!c)	continue;
		x+=(c->AdvanceX() >> 6) * sx;
		y+=(c->AdvanceY() >> 6) * sy;
	}
	*out_width = x;
	*out_height = y;
}


//
// -- Font char
//
FontChar::FontChar(int character, int left, int top, int ax, int ay, Bitmap *bitmap) {
	this->character = character;
	this->bitmap_left = left;
	this->bitmap_top = top;
	this->advance_x = ax;
	this->advance_y = ay;
	this->bitmap = bitmap;

	this->idTexture = 0;
}

FontChar::~FontChar() {
	if (bitmap != NULL) {
		delete bitmap;
	}
}

void FontChar::GenerateTexture() {

	//glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexture);
	glBindTexture(GL_TEXTURE_2D, idTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->Width(), bitmap->Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->Buffer());
}

void FontChar::Bind() {
	glBindTexture(GL_TEXTURE_2D, idTexture);	
}

struct point {
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
};

void FontChar::Draw(float x, float y, float sx, float sy) {
	Bind();

	float x2 = x + bitmap_left * sx;
	float y2 = -y - bitmap_top * sy;
	float w = bitmap->Width() * sx;
	float h = bitmap->Height() * sy;

	point box[4] = {
		{x2, -y2, 0, 0},
		{x2 + w, -y2, 1, 0},
		{x2, -y2 - h, 0, 1},
		{x2 + w, -y2 - h, 1, 1},
	};

    glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);    
  	glBindTexture(GL_TEXTURE_2D,idTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(x2,-y2,0);

      	glTexCoord2f(1.0f,0.0f);		
		glVertex3f(x2+w,-y2,0);

      	glTexCoord2f(1.0f,1.0f);
		glVertex3f(x2+w,-y2-h,0);

      	glTexCoord2f(0.0f,1.0f);		
		glVertex3f(x2,-y2-h,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

