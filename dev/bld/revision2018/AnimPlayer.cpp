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

#include <OpenGl/glu.h>

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


int CALLCONV InitializeAnimPlayerPlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&animplayer_factory),"name=gl.AnimPlayer");
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
	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}



AnimPlayer::AnimPlayer() {

}

void AnimPlayer::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	filename = pInstance->CreateProperty("filename", kPropertyType_String, "strips_full_movie_0.95_opt.db", "Strip Frame File");
	linewidth = pInstance->CreateProperty("linewidth", kPropertyType_Float, "2.0", "");
	color = pInstance->CreateProperty("color", kPropertyType_Color,"1,1,1,1","");
	depthtest = pInstance->CreateProperty("depthtest",kPropertyType_Bool,"false","");
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









