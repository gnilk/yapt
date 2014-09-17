//
// This needs serious cleanup!
//
#include <stdio.h>
#include <stdlib.h>
#include <GL/GLEW.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <string>
#include <vector>

#include "BaseWindow.h"
#include "ConsoleWindow.h"

#include "yapt/logger.h"
#include "yapt/ySystem.h"

#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


static const char *text_f_glsl=""		\
"varying vec2 texpos;\n"					\
"uniform sampler2D tex;\n"				\
"uniform vec4 color;\n"					\
" \n" \
"void main(void) {\n"						\
"  gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).a) * color;\n"\
"}\n"										\
;

static const char *text_v_glsl="#version 120\n"\
"\n"\
"attribute vec4 coord;\n"					\
"varying vec2 texpos;\n\n"					\
"void main(void) {\n"						\
"  gl_Position = vec4(coord.xy, 0, 1);\n"	\
"  texpos = coord.zw;\n"					\
"}\n"										\
;


static int StrExplode(std::vector<std::string> *strList, char *mString, int chrSplit);

// for some reason these variables must be global...


//static GLuint CreateProgram(const char *vertexsource, const char *fragmentsource);
//static GLuint CreateShader(const char* source, GLenum type);
//static void render_text(const char *text, float x, float y, float sx, float sy);


//static GLint GetAttrib(GLuint program, const char *name);
//static GLint GetUniform(GLuint program, const char *name);
// static void PrintLog(GLuint object);

using namespace yapt;

ConsoleWindow::ConsoleWindow() {
	fontSize = 12;
	watchScreenHeight = 250;
}
ConsoleWindow::~ConsoleWindow() {
}

bool ConsoleWindow::InitializeFreeType()
{
	MakeCurrent();
	pLogger = Logger::GetLogger("ConsoleWindow");

	GLenum glew_status = glewInit();
	if (GLEW_OK != glew_status) {
		pLogger->Error("GLEW-Error: %s\n", glewGetErrorString(glew_status));
		return false;
	}

	if (!GLEW_VERSION_2_0) {
		pLogger->Error("No support for OpenGL 2.0 found\n");
		return false;
	}

	if (glewIsSupported("GL_VERSION_3_3")) {
		pLogger->Debug("Ok, Open GL version 3.3 supported");		
	}
	else {
		pLogger->Error("OpenGL 3.3 not supported");
		//exit(1);
	}


	if(FT_Init_FreeType(&ft)) {
  		printf("Could not init freetype library\n");
  		return false;
  	}

 
	if(FT_New_Face(ft, "Arial.ttf", 0, &face)) {
  		printf("Could not open font\n");
  		return false;
	}
	FT_Set_Pixel_Sizes(face, 0, fontSize);

    program = CreateProgram("text.v.glsl", "text.f.glsl");
    if(program == 0) {
    	pLogger->Error("Failed shader program");
        return false;
    }
    attribute_coord = GetAttrib(program, "coord");
    uniform_tex = GetUniform(program, "tex");
    uniform_color = GetUniform(program, "color");
    if(attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1)
        return false;

	// Create the vertex buffer object
    glGenBuffers(1, &vbo);

	pLogger->Debug("Creating CreateBackBufferTexture");    
 	CreateBackBufferTexture();

    ProcessStringCommand("watch camera.pos");

	pLogger->Debug("Ok, console window initialized ok");    

	Invalidate();

	return true;
}

bool ConsoleWindow::CreateBackBufferTexture() {
	GLenum status;

	glGenFramebuffers(1, &idFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, idFrameBuffer);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frameBufferWidth, frameBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		pLogger->Debug("OpenGL FAILED!");
		exit(1);
	}
	pLogger->Debug("Ok, render target configured");
}

void ConsoleWindow::Prepare()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    glOrtho(0, width, 0, height, -5, 1);       
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void ConsoleWindow::PrepareBackBufferView() {
	glDisable(GL_DEPTH_TEST);
	glViewport(0,0,frameBufferWidth,frameBufferHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(0, width, 0, height, -5, 1);       
   	gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();	
}


bool ConsoleWindow::IsDirty() {
	return dirty;
}
void ConsoleWindow::SetDirty(bool bIsDirty) {
	dirty = bIsDirty;
}

void ConsoleWindow::Render()
{
	// Restore frambuffer
	bool renderToPrimary = false;

	if (NeedRedraw()) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		RenderToTexture(renderToPrimary);
		glPopAttrib();
		this->needRedraw = false;		
	}
	if (!renderToPrimary) {
		RenderLastTextureToScreen();
	}
}
static void renderBoundingSphere(float *mid, double radius)
{
	glColor3f(1,0.5,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1], mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,1,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double y = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0], mid[1]+y, mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,0.5,1);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double y = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1]+y, mid[2]);
		}
	glEnd();
}	


void ConsoleWindow::RenderToTexture(bool renderToPrimary) {
	float sx = 2.0 / width;
	float sy = 2.0 / height;

	if (!renderToPrimary) {
		glBindFramebuffer(GL_FRAMEBUFFER, idFrameBuffer);
		PrepareBackBufferView();
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Prepare();
	}

	/* White background */
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glUseProgram(program);
	/* Enable blending, necessary for our alpha texture */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat black[4] = { 0, 0, 0, 1 };
	GLfloat red[4] = { 1, 0, 0, 1 };
	GLfloat transparent_green[4] = { 0, 1, 0, 0.5 };

	/* Set font size , color to black */
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	glUniform4fv(uniform_color, 1, black);

	RenderWatchers(sx, sy);

	// This should render to a texture!
	// If 'not dirty' we can just swap the old texture..
	// The text seems to be pretty heavy to render..

	RenderHistoryBuffer(sx, sy);
	// Render the input area
	glUniform4fv(uniform_color, 1, black);
	RenderText(consoleInputString.c_str(), -1 + 8 * sx, 1 - (height-10) * sy, sx, sy);

	// Unload program
	glUseProgram(0);

	RenderDividers();

	glDisable(GL_BLEND);
	// restore frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ConsoleWindow::RenderLastTextureToScreen() {
	// - render full screen quad
	Prepare();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0,0,width,height); // Ensure,if someone did change it
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glScalef(1, 1, 1);
	//glTranslatef(0, -height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glDisable(GL_CULL_FACE);

    glBindTexture(GL_TEXTURE_2D,fbTexture);
    glEnable(GL_TEXTURE_2D);
    //glColor3f(0,1,0);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f,0.0f);
      glVertex2f(0.0f, 0.0f);
      glTexCoord2f(1.0f,0.0f);
      glVertex2f(width, 0.0f);
      glTexCoord2f(1.0f,1.0f);
      glVertex2f(width, height);
      glTexCoord2f(0.0f,1.0f);
      glVertex2f(0.0f, height);
     glEnd();  //glBegin(GL_QUADS);
    glDisable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);

}

void ConsoleWindow::RenderWatchers(float sx, float sy) {

	int yPos = 1;
	for(int i=0;i<watchList.size();i++) {
		IConsoleWatch *pWatch = watchList[i];
		std::string name = pWatch->GetName();
		std::string value = pWatch->GetValue();
		
		RenderText(name.c_str(), -1 + 8 * sx, 1 - yPos * fontSize * sy, sx, sy);
		RenderText(value.c_str(), -1 + width/2 * sx, 1 - yPos * fontSize * sy, sx, sy);

		std::vector<std::string> metadata;
		pWatch->GetMetaData(metadata);
		for(int j=0;j<metadata.size();j++) {
			yPos++;
			RenderText(metadata[j].c_str(), -1 + 32 * sx, 1 - yPos * fontSize * sy, sx, sy);
		}
		yPos++;
	}
}

void ConsoleWindow::RenderHistoryBuffer(float sx, float sy) {
	// Traverse history buffer backwards

	int maxRenderHeight = (height - watchScreenHeight) - 2*fontSize;
	// was: ((height/2)-fontSize)

	int linecount=0;
	for(int i=inputHistoryBuffer.size()-1;i>=0;i--) {
		int yp = (linecount+1) * fontSize;
		RenderText(inputHistoryBuffer[i].c_str(), -1 + 8 * sx, 1 - ((height-fontSize - yp) * sy), sx, sy);		
		linecount++;
		if ((yp+fontSize) > maxRenderHeight) break;
	}
}

void ConsoleWindow::RenderDividers()  {
	// Render comsmetic details
	// 1 - input area divider - bottom
	glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0,fontSize+10,0);
    glVertex3f(width, fontSize+10,0);
    glEnd();
    // 2 - divider between top and history buffer
	glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0,height - watchScreenHeight,0);	// was height/2
    glVertex3f(width, height - watchScreenHeight,0);
    glEnd();
    // 3 - divider between name and values for the watchers
	glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(width/2, height,0);	// was height/2
    glVertex3f(width/2, height - watchScreenHeight,0);
    glEnd();
}


struct point {
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
};


void ConsoleWindow::ProcessStringCommand(std::string input)
{
	std::vector<std::string> args;
	int num = StrExplode(&args, (char *)input.c_str(), ' ');
	if (num > 0) {
		IConsoleCommandHandler *command = ConsoleCommandFactory::GetInstance(args[0]);
		if (command != NULL) {
			command->Execute(this, input, args);
			// TODO: This should be delegated, we don't know if it was allocated or singleton
			delete command;
		} else {
			WriteLine("not found: "+input);
		}
	} else {
		WriteLine("Can process empty command string");
	}
}


void ConsoleWindow::OnStringInputChanged(std::string input)
{
	consoleInputString = input;
	Invalidate();
}
void ConsoleWindow::OnStringInputCommit(std::string input)
{
	consoleInputString = "";
	WriteLine(">" + input);

	// Process input string here
	ProcessStringCommand(input);	
}
void ConsoleWindow::AddWatch(IConsoleWatch *pWatch) {
	// TODO: Check watch list for duplicates
	watchList.push_back(pWatch);
	WriteLine("Ok, added watch: "+pWatch->GetName());
	Invalidate();
}
void ConsoleWindow::WriteLine(const char *string) {
	WriteLine(std::string(string));
}

void ConsoleWindow::WriteLine(std::string string) {
	inputHistoryBuffer.push_back(string);
	Invalidate();
}

// TODO: Refactor this to own TextRendering class
// Need more helpers
// - Estimation of graphical size of string
// - Per character attribute control
// - Cursor functionality
// - etc..
void ConsoleWindow::RenderText(const char *text, float x, float y, float sx, float sy) {
	const char *p;
	FT_GlyphSlot g = face->glyph;

	/* Create a texture that will be used to hold one "glyph" */
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uniform_tex, 0);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Set up the VBO for our vertex data */
	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Loop through all characters */
	int charcount = 0;
	for (p = text; *p; p++) {
		/* Try to load and render the character */
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;

		/* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		/* Calculate the vertex and texture coordinates */
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		point box[4] = {
			{x2, -y2, 0, 0},
			{x2 + w, -y2, 1, 0},
			{x2, -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		/* Draw the character on the screen */
		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;

		charcount++;
	}

	glDisableVertexAttribArray(attribute_coord);
	glDeleteTextures(1, &tex);
}

/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL
 */
static char* file_read(const char* filename)
{
  FILE* in = fopen(filename, "rb");
  if (in == NULL) return NULL;

  int res_size = BUFSIZ;
  char* res = (char*)malloc(res_size);
  int nb_read_total = 0;

  while (!feof(in) && !ferror(in)) {
    if (nb_read_total + BUFSIZ > res_size) {
      if (res_size > 10*1024*1024) break;
      res_size = res_size * 2;
      res = (char*)realloc(res, res_size);
    }
    char* p_res = res + nb_read_total;
    nb_read_total += fread(p_res, 1, BUFSIZ, in);
  }
  
  fclose(in);
  res = (char*)realloc(res, nb_read_total + 1);
  res[nb_read_total] = '\0';
  return res;
}


void ConsoleWindow::PrintLog(GLuint object)
{
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }

  char* log = (char*)malloc(log_length);

  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);

  fprintf(stderr, "%s", log);
  free(log);
}

 
GLuint ConsoleWindow::CreateShader(const char* filename, GLenum type)
{
  const GLchar* source = file_read(filename);
  if (source == NULL) {
    fprintf(stderr, "Error opening %s: ", filename); perror("");
    return 0;
  }
  GLuint res = glCreateShader(type);
  const GLchar* sources[] = {
    // Define GLSL version
#ifdef GL_ES_VERSION_2_0
    "#version 100\n"
#else
    "#version 120\n"
#endif
    ,
    // GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
    // Define default float precision for fragment shaders:
    (type == GL_FRAGMENT_SHADER) ?
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "precision highp float;           \n"
    "#else                            \n"
    "precision mediump float;         \n"
    "#endif                           \n"
    : ""
    // Note: OpenGL ES automatically defines this:
    // #define GL_ES
#else
    // Ignore GLES 2 precision specifiers:
    "#define lowp   \n"
    "#define mediump\n"
    "#define highp  \n"
#endif
    ,
    source };
  glShaderSource(res, 3, sources, NULL);
  free((void*)source);

  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "%s:", filename);
    PrintLog(res);
    glDeleteShader(res);
    return 0;
  }

  return res;
}

GLuint ConsoleWindow::CreateProgram(const char *vertexfile, const char *fragmentfile) {
	GLuint program = glCreateProgram();
	GLuint shader;

	if(vertexfile) {
		shader = CreateShader(vertexfile, GL_VERTEX_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	if(fragmentfile) {
		shader = CreateShader(fragmentfile, GL_FRAGMENT_SHADER);
		if(!shader)
			return 0;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		PrintLog(program);
		glDeleteProgram(program);
		return 0;
	}

	return program;
}

GLint ConsoleWindow::GetAttrib(GLuint program, const char *name) {
	GLint attribute = glGetAttribLocation(program, name);
	if(attribute == -1)
		fprintf(stderr, "Could not bind attribute %s\n", name);
	return attribute;
}

GLint ConsoleWindow::GetUniform(GLuint program, const char *name) {
	GLint uniform = glGetUniformLocation(program, name);
	if(uniform == -1)
		fprintf(stderr, "Could not bind uniform %s\n", name);
	return uniform;
}
//
// --- Console Watch handling
//
ConsolePropertyWatcher::ConsolePropertyWatcher(IBaseInstance *pObject) {

	this->pObject = pObject;
	this->pProp = dynamic_cast<IPropertyInstance *>(pObject);
}
ConsolePropertyWatcher::~ConsolePropertyWatcher() {

}

std::string ConsolePropertyWatcher::GetName() {
	return pObject->GetFullyQualifiedName();
}
std::string ConsolePropertyWatcher::GetValue() {
	char tmp[256];
	return (std::string(pProp->GetValue(tmp, 256)));
}
void ConsolePropertyWatcher::GetMetaData(std::vector<std::string> &metadata) {
   	if (pProp->IsSourced()) {
   		IPropertyInstance *pSource = pProp->GetSource();
   		IBaseInstance *pSourceBase = dynamic_cast<IBaseInstance *>(pSource);
   		std::string fullSrcName = pSourceBase->GetFullyQualifiedName();
        std::string srcMeta = " [S: " + std::string(pProp->GetSourceString()) + " (" + fullSrcName +")]";

		metadata.push_back(srcMeta);        
    }

}


//
// --- Console command handling
//
class ConsoleCommandDumpDoc : IConsoleCommandHandler, IDocumentTraversalSink
{
public:
	virtual void OnNode(IDocNode *node, int depth);
	virtual bool Execute(IConsole *pConsole, std::string raw, std::vector<std::string> arguments);
	static IConsoleCommandHandler *GetInstance();
private:
	IConsole *pConsole;
	std::string nodeNameFilter;
};

class ConsoleCommandWatch : public IConsoleCommandHandler, IDocumentTraversalSink
{
public:
	virtual void OnNode(IDocNode *node, int depth);
	virtual bool Execute(IConsole *pConsole, std::string raw, std::vector<std::string> arguments);
	virtual void HandleSearchResult();
	virtual std::string GetCommandName();
	static IConsoleCommandHandler *GetInstance();
protected:
	IConsole *pConsole;
	std::string setValue;
	std::string nodeNameFilter;
	std::vector<IBaseInstance *>filterSearchResult;
};

class ConsoleCommandSet : ConsoleCommandWatch
{
public:
	virtual std::string GetCommandName();
	virtual void HandleSearchResult();
	static IConsoleCommandHandler *GetInstance();
};


static CONSOLE_COMMAND consoleCommandFactoryList[] =
{
	"dump", ConsoleCommandDumpDoc::GetInstance,
	"watch", ConsoleCommandWatch::GetInstance,
	"set", ConsoleCommandSet::GetInstance,
	"", NULL,
};

IConsoleCommandHandler *ConsoleCommandFactory::GetInstance(std::string command) {
	for(int i=0;consoleCommandFactoryList[i].factory!=NULL;i++) {
		if (consoleCommandFactoryList[i].name == command) {
			return consoleCommandFactoryList[i].factory();
		}
	}
	return NULL;
}

//
// dump doc
//
bool ConsoleCommandDumpDoc::Execute(IConsole *pConsole, std::string raw, std::vector<std::string> arguments) {
	nodeNameFilter = "";

	for(int i=1;i<arguments.size();i++) {
		nodeNameFilter = arguments[i];
	}
	this->pConsole = pConsole;
	yapt::ISystem *system = GetYaptSystemInstance();
	system->GetActiveDocumentController()->TraverseDocument(this);

	return false;
}

void ConsoleCommandDumpDoc::OnNode(IDocNode *node, int depth) {

	bool printNodeInfo = true;

	IBaseInstance *pObject = node->GetNodeObject();
	std::string nodeinfo = "";

	// Do node filtering here
	std::string name = std::string(pObject->GetFullyQualifiedName());
	if (!nodeNameFilter.empty()) {
		if (name.find(nodeNameFilter) == std::string::npos) {
			printNodeInfo = false;
		}
	}

	// break out if this node is filtered
	if (printNodeInfo == false) return;

	nodeinfo += name;

	//nodeinfo += std::string(pObject->GetAttributeValue("name"));

	// How to extract a property value from a specifically named node
	if (pObject->GetInstanceType() == kInstanceType_Property) {
	    IPropertyInstance *pProp = dynamic_cast<IPropertyInstance *>(pObject);
	    if(pProp != NULL) {
	        char tmp[256];
	        pProp->GetValue(tmp,256);
	        nodeinfo += " (" + std::string(tmp) + ")";
	       	if (pProp->IsSourced()) {

	       		IPropertyInstance *pSource = pProp->GetSource();
	       		IBaseInstance *pSourceBase = dynamic_cast<IBaseInstance *>(pSource);
	       		std::string fullSrcName = pSourceBase->GetFullyQualifiedName();

	            nodeinfo += " [S: " + std::string(pProp->GetSourceString()) + " (" + fullSrcName +")]"; //std::string(pProp->GetSourceString());	       
	        }
	    }
	}
	// print node info
	pConsole->WriteLine(nodeinfo);

}


IConsoleCommandHandler *ConsoleCommandDumpDoc::GetInstance() {
	return new ConsoleCommandDumpDoc();
}


//
// watch
//
std::string ConsoleCommandWatch::GetCommandName() {
	return std::string("watch");
}

bool ConsoleCommandWatch::Execute(IConsole *pConsole, std::string raw, std::vector<std::string> arguments) {
	this->pConsole = pConsole;
	bool addAll = false;
	nodeNameFilter = "";
	setValue = "";

	for(int i=1;i<arguments.size();i++) {
		if (arguments[i][0]=='-' || arguments[i][0]=='/') {
			switch(arguments[i][1]) {
				case 'a' : addAll = true; break;
				case 'v' : setValue = std::string(arguments[++i]); break;
				default :
					pConsole->WriteLine("Unknown option: "+arguments[i]);
					pConsole->WriteLine("Usage: "+GetCommandName()+" [-a] [-v <value>] <filter> ");
					return false;
			}

		} else {
			if (nodeNameFilter == "") {
				nodeNameFilter = arguments[i];				
			} else {
				setValue = arguments[i];
			}

		}
	}

	if (!nodeNameFilter.empty()) {
		yapt::ISystem *system = GetYaptSystemInstance();
		system->GetActiveDocumentController()->TraverseDocument(this);	
		HandleSearchResult();

	} else {
		pConsole->WriteLine("use: "+GetCommandName()+" <propertyname>");
	}

	return false;
}
void ConsoleCommandWatch::HandleSearchResult() {
	if (filterSearchResult.size() == 1) {
		ConsolePropertyWatcher *pWatch  = new ConsolePropertyWatcher(filterSearchResult[0]);
		pConsole->AddWatch(pWatch);		
	} else if (filterSearchResult.size() > 1) {
		pConsole->WriteLine("Multiple matches, narrow or use -a to add all");
		for(int i=0;i<filterSearchResult.size();i++) {
			pConsole->WriteLine(filterSearchResult[i]->GetFullyQualifiedName());
		}
	} else {
		pConsole->WriteLine("No match");
	}
}

void ConsoleCommandWatch::OnNode(IDocNode *node, int depth) {

	IBaseInstance *pObject = node->GetNodeObject();
	if (pObject->GetInstanceType() == kInstanceType_Property) {
		std::string nodeinfo = "";

		// Do node filtering here
		std::string name = std::string(pObject->GetFullyQualifiedName());
		if (!nodeNameFilter.empty()) {
			if (name.find(nodeNameFilter) != std::string::npos) {
				//IPropertyInstance *prop = dynamic_cast<IPropertyInstance *>(pObject);
				filterSearchResult.push_back(pObject);
			}
		}
	}
}


IConsoleCommandHandler *ConsoleCommandWatch::GetInstance() {
	return new ConsoleCommandWatch();
}


std::string ConsoleCommandSet::GetCommandName() {
	return std::string("set");
}

void ConsoleCommandSet::HandleSearchResult() {

	if (filterSearchResult.size() == 1) {
		IBaseInstance *pObject = filterSearchResult[0];
		IPropertyInstance *pProperty = dynamic_cast<IPropertyInstance *>(pObject);
		if (setValue != std::string("")) {
			pProperty->SetValue(setValue.c_str());
			pConsole->WriteLine(std::string(pObject->GetFullyQualifiedName()) + "="+setValue);
		}
	} else if (filterSearchResult.size() > 1) {
		pConsole->WriteLine("Multiple matches, narrow or use -a to add all");
		for(int i=0;i<filterSearchResult.size();i++) {
			pConsole->WriteLine(filterSearchResult[i]->GetFullyQualifiedName());
		}
	} else {
		pConsole->WriteLine("No Match");
	}
}

IConsoleCommandHandler *ConsoleCommandSet::GetInstance() {
	return new ConsoleCommandSet();
}


// Well, this is a copy from logger.cpp
static int StrExplode(std::vector<std::string> *strList, char *mString, int chrSplit)
{
	std::string strTmp,strPart;
	size_t ofs,pos;
	int count;

	strTmp = std::string(mString);
	pos = count = 0;
	do 	
	{
		ofs = strTmp.find_first_of(chrSplit,pos);

		if (ofs == -1)
		{
			if (pos != -1)
			{
				strPart = strTmp.substr(pos,strTmp.length()-pos);
				strList->push_back(strPart);
				count++;
			}
			else
			{
				// We had trailing spaces...
			}

			break;
		}
		strPart = strTmp.substr(pos,ofs - pos);
		strList->push_back(strPart);
		pos = ofs+1;
		pos = strTmp.find_first_not_of(chrSplit,pos);
		count++;
	} while(1);

	return count;
} // StrExplode








