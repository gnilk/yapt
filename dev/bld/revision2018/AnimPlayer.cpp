/*-------------------------------------------------------------------------
File    : $Archive: AnimPlayer.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2018-01-23, 15:50
Descr   : 


Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 30.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/ySystem.h"
#include "yapt/logger.h"


#ifdef WIN32
#include <malloc.h>
#endif

#include <OpenGl/gl.h>
#include <OpenGl/glu.h>

#include "contour/contour.h"
#include "contour/bitmap.h"
#include "contour/lodepng.h"

#include <fstream>

using namespace yapt;

extern "C" 
{
int CALLCONV InitializeAnimPlayerPlugin(ISystem *ySys);
}

struct Point {
	uint8_t x;
	uint8_t y;
	float fx;	// Normalized range (-1 .. 1
	float fy;	// Normalized range (-1 .. 1)
};

class Strip {
private:
	uint8_t nPoints;
	std::vector<Point> points;
private:
	void Normalize();
public:
	void Load(FILE *f);
	int Points() { return points.size(); }
	void Render();
};

class Frame {
private:
	uint8_t nStrips;
	std::vector<Strip> strips;
private:
public:
	void Load(FILE *f);
	int Strips() { return strips.size(); }
	void Render();
};

class Animation {
private:
	std::vector<Frame *> frames;
public:
	void LoadFromFile(const char *filename);
	int Frames() { return frames.size(); }
	void Render(int frame);	
};


class InternalPluginFactory :
	public IPluginObjectFactory
{
public:	
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *guid_identifier);
};

static InternalPluginFactory animplayer_factory;

class AnimPlayer :
	public IPluginObject
{
protected:
	// Video properties
	Property *filename;
	Property *linewidth;
	Property *color;
	Property *depthtest;
	Animation anim;
	// System properties
//	Property *fps;
public:
	AnimPlayer();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};

class ContourTracer : public IPluginObject {
protected:
	Property *texture;
	Property *width;
	Property *height;

	gnilk::contour::Trace tracer;
public:
	ContourTracer();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
private:
	void RenderStrip(gnilk::contour::Strip *strip);
};


class CopyGLDepthBufferToTexture : public IPluginObject {
protected:
	Property *framebuffer;
	Property *width;
	Property *height;
	Property *depthtexture;
public:
	CopyGLDepthBufferToTexture();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};

int CALLCONV InitializeAnimPlayerPlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&animplayer_factory),"name=gl.AnimPlayer");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&animplayer_factory),"name=gl.CopyDepthBuffer");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&animplayer_factory),"name=gl.ContourTracer");
	return 0;
}


IPluginObject *InternalPluginFactory::CreateObject(ISystem *pSys, const char *identifier)
{
	//pSysPtr = pSys; // Curve does not have an ISystem ptr  - use a globally shared variable instead

	ILogger *pLogger = pSys->GetLogger("InternalPluginFactory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier,"gl.AnimPlayer"))
	{
		pObject = dynamic_cast<IPluginObject *> (new AnimPlayer());
	} else if (!strcmp(identifier, "gl.CopyDepthBuffer")) {
		pObject = dynamic_cast<IPluginObject *> (new CopyGLDepthBufferToTexture());
	} else if (!strcmp(identifier, "gl.ContourTracer")) {
		pObject = dynamic_cast<IPluginObject *> (new ContourTracer());
	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}


/*
glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT,
                          0, 0, screenWidth, screenHeight,
                          GL_COLOR_BUFFER_BIT,
                          GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
*/


CopyGLDepthBufferToTexture::CopyGLDepthBufferToTexture() {

}
void CopyGLDepthBufferToTexture::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	framebuffer = pInstance->CreateProperty("framebuffer", kPropertyType_Integer, "0", "");	
	width = pInstance->CreateProperty("width", kPropertyType_Integer, "1280", "");
	height = pInstance->CreateProperty("height", kPropertyType_Integer, "720", "");
	depthtexture = pInstance->CreateOutputProperty("depthtexture", kPropertyType_Integer, "0", "");
}
static bool saved = false;
void CopyGLDepthBufferToTexture::Render(double t, IPluginObjectInstance *pInstance) {
	int szBuffer = width->v->int_val * height->v->int_val;
	float buffer[szBuffer];
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->v->int_val);
	glReadPixels(0,0,width->v->int_val, height->v->int_val, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	if (t > 2) {
		if (!saved) {
			float fMax, fMin;
			fMax = -100000000.0f;
			fMin = 1000000000.0f;
			int histogram[10];
			for (int i=0;i<10;i++) { histogram[i] = 0; }
			for(int i=0;i<szBuffer;i++) {
				if (buffer[i] > fMax) {
					fMax = buffer[i];
				}
				if (buffer[i] < fMin) {
					fMin= buffer[i];
				}
				int idx = 9 * buffer[i];
				if ((idx >= 0) && (idx < 10)) {
					histogram[idx] += 1;
				} else {
					printf("OUT OF RANGE: %f:%d\n", buffer[i], idx);
				}
			}
			printf("Saving depth buffer\n");
			printf("  Min: %f\n", fMin);
			printf("  Max: %f\n", fMax);
			printf("  Historgram\n");
			for(int j=0;j<10;j++) {
				printf("     %d: %d\n", j, histogram[j]);
			}
			FILE *f = fopen("buffer.bin", "w");
			fwrite(buffer, 1, szBuffer * sizeof(float), f);
			fclose(f);
			saved = true;			
		}
	}



	// glGetFrameBufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_WIDTH, &width);
	// glGetFrameBufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_HEIGHT, &height);

	//glBindFramebuffer(GL_FRAMEBUFFER_EXT, framebuffer->v->int_val);
	// glBindTexture(GL_TEXTURE_2D, depthtexture->v->int_val);
	// glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0,0, width->v->int_val, height->v->int_val,0);

	// glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->v->int_val);
 //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
 //    glBlitFramebuffer(0, 0, width, height,
 //                      0, 0, 1280, 720,
 //                      GL_DEPTH_BUFFER_BIT ,
 //                      GL_LINEAR);
 //    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
void CopyGLDepthBufferToTexture::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	GLuint idTexture;

  	glGenTextures(1, &idTexture);	
	glBindTexture(GL_TEXTURE_2D, idTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width->v->int_val, height->v->int_val, 0, GL_DEPTH_COMPONENT,	GL_FLOAT, 0);	
	depthtexture->v->int_val = idTexture;
}
void CopyGLDepthBufferToTexture::PostRender(double t, IPluginObjectInstance *pInstance) {

}
void CopyGLDepthBufferToTexture::Signal(int channelId, const char *channelName, int sigval, double sigtime) {

}


//
// Animplayer
//

AnimPlayer::AnimPlayer() {

}

void AnimPlayer::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	filename = pInstance->CreateProperty("filename", kPropertyType_String, "strips_full_movie_0.95_opt.db", "Strip Frame File");
	linewidth = pInstance->CreateProperty("linewidth", kPropertyType_Float, "2.0", "");
	color = pInstance->CreateProperty("color", kPropertyType_Color,"1,1,1,1","");
	depthtest = pInstance->CreateProperty("depthtest",kPropertyType_Bool,"true","");
}
void AnimPlayer::Render(double t, IPluginObjectInstance *pInstance) {
	int frameCounter = (int)(t * 30.0);
	if (frameCounter > anim.Frames()) {
		frameCounter = 0;
	}
	if (!depthtest->v->boolean) {
		glDisable(GL_DEPTH_TEST);
	} else {
		glEnable(GL_DEPTH_TEST);
	}

	glColor3f(color->v->rgba[0], color->v->rgba[1], color->v->rgba[2]);	
	glLineWidth(linewidth->v->float_val);
	anim.Render(frameCounter);	
	if (!depthtest->v->boolean) {
		glEnable(GL_DEPTH_TEST);
	} 

}
void AnimPlayer::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	ySys->GetLogger("AnimPlayer")->Error("Load animation file from: %s", filename->v->string);
	anim.LoadFromFile(filename->v->string);

}
void AnimPlayer::PostRender(double t, IPluginObjectInstance *pInstance) {

}
void AnimPlayer::Signal(int channelId, const char *channelName, int sigval, double sigtime) {

}


//////////////////////////////////


void Strip::Normalize() {
	for (int i=0;i<points.size();i++) {
		points[i].fx = (float(points[i].x) - 128.0) / 128.0;
		points[i].fy = (float(points[i].y) - 128.0) / 128.0;

		// printf("      %f, %f\n", points[i].fx, points[i].fy);
	}
}
void Strip::Load(FILE *f) {	
	fread(&nPoints, sizeof(uint8_t), 1, f);
	//printf("Points: %d",nPoints);
	for (int p=0;p<nPoints;p++) {
		uint8_t x,y;
		Point pt;
		fread(&pt.x, sizeof(uint8_t), 1, f);
		fread(&pt.y, sizeof(uint8_t), 1, f);
		points.push_back(pt);
	}
	Normalize();
}
void Strip::Render() {
	glBegin(GL_LINE_STRIP);
	for (int i=0;i<points.size();i++) {
		glVertex3f(points[i].fx, points[i].fy, 0);
	}
	glEnd();
}

void Frame::Load(FILE *f) {
	fread(&nStrips, sizeof(uint8_t), 1, f);
	//printf("  Strips In Frame: %d\n", nStrips);
	for(int i=0;i<nStrips;i++) {
		Strip s;
		//printf("    Strip: %d, ", i);
		s.Load(f);
		//printf("\n");
		strips.push_back(s);
	}
}

void Frame::Render() {
	for (int i=0;i<strips.size();i++) {
		strips[i].Render();
	}
}


void Animation::LoadFromFile(const char *filename) {
	FILE *f = fopen(filename,"r");
	if (f == NULL) {
		perror("Unable to open strips file");
	}

	int frameCounter = 0;
	while(!feof(f)) {
		//printf("Frame: %d, array: %d\n", frameCounter, frames.size());
		Frame *frame = new Frame();
		frame->Load(f);
		this->frames.push_back(frame);
		// This is a frame!
		frameCounter++;
		// if (frameCounter > 143) {
		// 	break;
		// }
	}	
	printf("Frames loaded: %d\n", this->Frames());

	fclose(f);
}

void Animation::Render(int frame) {
	frames[frame]->Render();
}


///////////////
ContourTracer::ContourTracer() {

}
void ContourTracer::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	texture = pInstance->CreateProperty("texture", kPropertyType_Integer, "0", "");
	width = pInstance->CreateProperty("width", kPropertyType_Integer, "256", "");
	height = pInstance->CreateProperty("height", kPropertyType_Integer, "256", "");

}

void ContourTracer::Render(double t, IPluginObjectInstance *pInstance) {

	int szBuffer = width->v->int_val * height->v->int_val;
	unsigned char buffer[szBuffer * 4];

	glBindTexture(GL_TEXTURE_2D, texture->v->int_val);	
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);   	

	std::vector<gnilk::contour::Strip *> strips;
	tracer.ProcessImage(strips, buffer, width->v->int_val, height->v->int_val);

	glBindTexture(GL_TEXTURE_2D, 0);

	// draw strips
	for(int i=0;i<strips.size();i++) {
		RenderStrip(strips[i]);
	}

	if ((saved == false) && (t > 2)) {
		// printf("Saved texture to file!\n");
		// 	screenshot("input.tga",1280,720);
		// 	exit(1);
		//glFlush();
		// glPixelStorei(GL_PACK_ALIGNMENT, 1);
		//glReadPixels(0,0,width->v->int_val,height->v->int_val, GL_RGBA,GL_UNSIGNED_BYTE,&buffer[0]);

		gnilk::Bitmap *bitmap = new gnilk::Bitmap(width->v->int_val, height->v->int_val, buffer);
		bitmap->SaveToFile("input.png");
		printf("Saved texture to file!\n");
		saved = true;
	}
}
void ContourTracer::RenderStrip(gnilk::contour::Strip *strip) {
	// Normalize
	float w = 0.5 * (float)width->v->int_val;
	float h = 0.5 * (float)height->v->int_val;

	glColor3f(1,0.75,0.6);
	glLineWidth(4);
	glBegin(GL_LINE_STRIP);
	for (int i=0;i<strip->size();i++) {
		float x = (float(strip->at(i).x) - w) / w;
		float y = (float(strip->at(i).y) - h) / w;
		// printf("      %f, %f\n", points[i].fx, points[i].fy);
		glVertex3f(x,y, 0);
	}
	glEnd();
}

void ContourTracer::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void ContourTracer::PostRender(double t, IPluginObjectInstance *pInstance) {

}
void ContourTracer::Signal(int channelId, const char *channelName, int sigval, double sigtime) {

}







