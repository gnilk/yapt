//
// Curve animation handling for Yapt, facade for goat object
//
#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"

#ifdef WIN32
#include <malloc.h>
#endif

#include <GL/glfw.h>

using namespace yapt;

extern "C" {
	int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class Factory: public IPluginObjectFactory {
public:
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};

class PluginObjectImpl: public IPluginObject {
public:
	PluginObjectImpl();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys,
			IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

class OpenGLRenderContext: public PluginObjectImpl {
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys,
			IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

class OpenGLTriangle: public PluginObjectImpl {
private:
	Property *numIndex;
	Property *indexData;
	Property *vertexData;
	Property *speed;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys,
			IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

class TestTriangleGenerator: public PluginObjectImpl {
private:
	Property *numIndex;
	Property *indexData;
	Property *vertexData;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys,
			IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

static Factory factory;

IPluginObject *Factory::CreateObject(ISystem *pSys, const char *identifier) {
	ILogger *pLogger = pSys->GetLogger("glFuncs.Factory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier, "gl.RenderContext")) {
		pObject = dynamic_cast<IPluginObject *>(new OpenGLRenderContext());
	}
	if (!strcmp(identifier, "gl.Plot")) {
//		pObject = dynamic_cast<IPluginObject *> (new YaptCurveFacade());
	}
	if (!strcmp(identifier, "gl.Rotate3f")) {
//		pObject = dynamic_cast<IPluginObject *> (new GenericCurveKey());
	}
	if (!strcmp(identifier, "gl.DrawTriangle")) {
		pObject = dynamic_cast<IPluginObject *>(new OpenGLTriangle());
	}
	if (!strcmp(identifier, "geom.Triangle")) {
		pObject = dynamic_cast<IPluginObject *>(new TestTriangleGenerator());
	}
	if (pObject != NULL) {
		pLogger->Debug("Ok");
	} else
		pLogger->Debug("Failed");
	return pObject;
}

static void perror() {
}

// This function must be exported from the lib/dll
int CALLCONV yaptInitializePlugin(ISystem *ySys) {

	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),
			"name=gl.RenderContext");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),
			"name=gl.Plot");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),
			"name=gl.Rotate3f");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),
			"name=gl.DrawTriangle");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),
			"name=geom.Triangle");
	return 0;
}
/////////////////////
PluginObjectImpl::PluginObjectImpl() {

}
void PluginObjectImpl::Initialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::Render(double t, IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::PostInitialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::PostRender(double t, IPluginObjectInstance *pInstance) {
}
//
// -- Render context
//
void OpenGLRenderContext::Initialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}
void OpenGLRenderContext::Render(double t, IPluginObjectInstance *pInstance) {
	int width = 640;
	int height = 480;
	glViewport(0, 0, 640, 480);

	// Clear color buffer to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Select and setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

	// Select and setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 1.0f, 0.0f,    // Eye-position
			0.0f, 20.0f, 0.0f,   // View-point
			0.0f, 0.0f, 1.0f);  // Up-vector

}

void OpenGLRenderContext::PostInitialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}

void OpenGLRenderContext::PostRender(double t,
		IPluginObjectInstance *pInstance) {

	// TODO: Copy to texture here!

}
//
// -- Triangle
//
void TestTriangleGenerator::Initialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {
	numIndex = pInstance->CreateOutputProperty("numIndex",
			kPropertyType_Integer, "0", "");
	indexData = pInstance->CreateOutputProperty("indexData",
			kPropertyType_UserPtr, NULL, "");
	vertexData = pInstance->CreateOutputProperty("vertexData",
			kPropertyType_UserPtr, NULL, "");
}
void TestTriangleGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}
void TestTriangleGenerator::PostInitialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {
	int numIdx = 3;
	int *pIndex = (int *) malloc(sizeof(int) * numIdx);
	float *pVertex = (float *) malloc(sizeof(float) * 3 * 3);

	vIni(&pVertex[0 * 3], -5.0f, 0.0f, -4.0f);
	vIni(&pVertex[1 * 3], 5.0f, 0.0f, -4.0f);
	vIni(&pVertex[2 * 3], 0.0f, 0.0f, 6.0f);

	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;

	numIndex->v->int_val = numIdx;
	indexData->v->userdata = pIndex;
	vertexData->v->userdata = pVertex;

}
void TestTriangleGenerator::PostRender(double t,
		IPluginObjectInstance *pInstance) {

}

void OpenGLTriangle::Initialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {
	numIndex = pInstance->CreateProperty("numindex", kPropertyType_Integer, "0",
			"");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr,
			NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr,
			NULL, "");
	speed = pInstance->CreateProperty("speed", kPropertyType_Float, "100", "");
}
void OpenGLTriangle::Render(double t, IPluginObjectInstance *pInstance) {
	glTranslatef(0.0f, 14.0f, 0.0f);
	float m_speed = speed->v->float_val;
	glRotatef((GLfloat) m_speed, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	int idx = 0;
	float *pVtx = (float *) vertexData->v->userdata;
	int *pIdx = (int *) indexData->v->userdata;

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3fv(&pVtx[pIdx[0] * 3]);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3fv(&pVtx[pIdx[1] * 3]);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3fv(&pVtx[pIdx[2] * 3]);

	glEnd();
}
void OpenGLTriangle::PostInitialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}
void OpenGLTriangle::PostRender(double t, IPluginObjectInstance *pInstance) {

}

