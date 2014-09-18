#include "glShaderBase.h"
#include "glDrawText.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#include <OpenGl/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

static ISystem *psys = NULL;	// font management

void OpenGLDrawText::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	font = pInstance->CreateProperty("font", kPropertyType_String, "Arial.ttf", "");
	fontSize = pInstance->CreateProperty("fontsize", kPropertyType_Integer, "24", "");
	text = pInstance->CreateProperty("text", kPropertyType_String, "hello world!", "");
	alignment = pInstance->CreateProperty("alignment", kPropertyType_Enum, "center","enum={none,left,right,center}");
	position = pInstance->CreateProperty("position", kPropertyType_Vector,"0,0,0","");
	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
	psys = ySys;
}

void OpenGLDrawText::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	textureFont = FontManager::GetInstance()->GetFont(std::string(font->v->string), fontSize->v->int_val);
	//new Font(std::string(font->v->string), fontSize->v->int_val, face);
	textureFont->Build();

	std::string vtxshader(vshader->v->string);
	std::string fragshader(fshader->v->string);

	if (!vtxshader.empty() && !fragshader.empty()) {
	  OpenGLShaderBase::Initialize(ySys, 
	    std::string(vshader->v->string),
	    std::string(fshader->v->string));

	  OpenGLShaderBase::ReloadIfNeeded(true);
	  if (program == NULL) {
	    ySys->GetLogger("GLShaderQuad")->Error("Shader program load error");
	    exit(1);
	  }		
	  useShaders = true;
	}
}

void OpenGLDrawText::Render(double t, IPluginObjectInstance *pInstance) {
	// Render bitmaps
	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	float width = contextParams->GetFrameBufferWidth();
	float height = contextParams->GetFrameBufferHeight();

	unsigned int uniform_tex = 0;
	unsigned int uniform_color = 0;


	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}


	float sx = 2.0 / width;
	float sy = 2.0 / height;

	sx *= width/height;

//	glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
	glScalef(-1,1,1);
	textureFont->SetRenderScaling(sx,sy);

	float xp,yp;
	float str_width, str_height;
	std::string str = std::string(text->v->string);
	textureFont->Estimate(str, &str_width, &str_height);
	xp = yp = 0;
	xp = position->v->vector[0];
	yp = position->v->vector[1];
	switch(alignment->v->int_val) {
		case 0 :	// none, use position
			break;
		case 1 :
			xp = 0;
			break;
		case 2 :	// right
			xp = 1 - str_width;
			break;
		case 3 : 	// center
			xp = 0 - str_width/2.0f;
			break;
	}

//	printf("alignment: %d\n",alignment->v->int_val);


	textureFont->Draw(str,xp,yp);
	glPopMatrix();

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

}

void OpenGLDrawText::PostRender(double t, IPluginObjectInstance *pInstance) {

}
//
// Helpers, todo - refactor these to own file
//

//
// -- font management
//
// static
FontManager *FontManager::instance = NULL;

FontManager::FontManager() {
	if(FT_Init_FreeType(&ft)) {
  		psys->GetLogger("FontManager")->Error("OpenGLDrawText, Could not init freetype library");
  		exit(1);
  	}
}

FontManager *FontManager::GetInstance() {
	if (instance == NULL) {
		instance = new FontManager();
	}
	return instance;
}

Font *FontManager::GetFont(std::string family, int size) {
	Font *font = NULL;
	// Look through cache
	for(int i=0;i<fonts.size();i++) {
		if ((family == fonts[i]->Family()) && (size == fonts[i]->Size())) {
			psys->GetLogger("FontManager")->Debug("Found font '%s' with size %d in cached",family.c_str(), size);
			return fonts[i];
		}
	}

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



// -- Font
Font::Font(std::string family, int size, FT_Face face) {
	this->family = family;
	this->size = size;
	this->face = face;

	for(int i=0;i<256;i++) {
		characters[i] = NULL;
	}
}

void Font::Build() {
	static std::string lowerCase="abcdefghijklmnopqrstuvwxyzåäö";
	static std::string upperCase="ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ ";
	static std::string otherChars="01234567890!@#$\'\"";

	BuildTexturesForString(lowerCase);
	BuildTexturesForString(upperCase);
	BuildTexturesForString(otherChars);
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

void Font::Draw(int character, float x, float y) {
	FontChar *c = GetChar(character);
	if (c != NULL) {
		c->Draw(x,y,sx,sy);
	} else {
		printf("not found\n");
	}
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


// Font char

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



//
// bitmap data for font
//

Bitmap::Bitmap(int w, int h, unsigned char *buffer) {
	this->width = w;
	this->height = h;
	this->buffer = (unsigned char *)malloc(sizeof(char) * w * h *4);
	for(int i=0;i<w*h;i++) {
		this->buffer[i*4+0] = buffer[i];
		this->buffer[i*4+1] = buffer[i];
		this->buffer[i*4+2] = buffer[i];
		this->buffer[i*4+3] = buffer[i];
	}
}
Bitmap::~Bitmap() {
	free(this->buffer);
}


