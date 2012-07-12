//
// Curve animation handling for Yapt, facade for goat object
//
#include "yapt/ySystem.h"
#include "yapt/logger.h"
#include "Curve.h"
#include "expsolver.h"

#ifdef WIN32
#include <malloc.h>
#endif

#include <math.h>


using namespace yapt;
using namespace Goat;	// Curve is within the Goat namespace...


extern "C"
{
	int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class CurveFactory :
	public IPluginObjectFactory
{
public:	
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};

class BaseCurveFacade :
	public IPluginObject
{
public:
	BaseCurveFacade();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};


class GenericCurveKey :
	public BaseCurveFacade
{
protected:
	Key *pKey;
	Property *t;
	Property *tension;
	Property *bias;
	Property *continity;
	Property *values;
protected:
	virtual void ParseValueString();
	virtual void RegisterValueType(ISystem *ySys, IPluginObjectInstance *pInstance);
	// Own
public:
	Key *GetKey();
	// IPluginObject
public:
	GenericCurveKey();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);	
};

class VectorCurveKey :
	public GenericCurveKey
{
protected:
	virtual void ParseValueString();
	virtual void RegisterValueType(ISystem *ySys, IPluginObjectInstance *pInstance);

};

class YaptCurveFacade :
	public BaseCurveFacade
{
protected:
	Property *channels;
	Property *curveType;
	Property *result;	// output
	Property *testVector;
	Curve *pCurve;
public:
	YaptCurveFacade();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	
};

class YaptExpSolverFacade :
    public BaseCurveFacade
{
protected:
    Property *expression;
    Property *result;   // output

    double timeCurrent; // corrsponds to the 't' variable in the expression
    ExpSolver *pExpSolver; 
    // events
public:
    double OnConstantUserExpression(const char *data, int *bOk_out);
    double OnFunctionExpression(const char *data, double arg, int *bOk_out);
    // interface
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};


static CurveFactory factory;

IPluginObject *CurveFactory::CreateObject(ISystem *pSys, const char *identifier)
{
	ILogger *pLogger = pSys->GetLogger("StdCurve.Factory");//Logger::GetLogger("StdCurve.Factory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier,"Animation.GenericCurve"))
	{
		pObject = dynamic_cast<IPluginObject *> (new YaptCurveFacade());
	}
	if (!strcmp(identifier,"Animation.Key"))
	{
		pObject = dynamic_cast<IPluginObject *> (new GenericCurveKey());
	}
	if (!strcmp(identifier,"Animation.VectorKey"))
	{
		pObject = dynamic_cast<IPluginObject *> (new VectorCurveKey());
	}
	if (!strcmp(identifier,"Animation.ExpSolver"))
	{
		pObject = dynamic_cast<IPluginObject *> (new YaptExpSolverFacade());
	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}


// -- Base effect - simple pass through
BaseCurveFacade::BaseCurveFacade()
{
}

void BaseCurveFacade::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
}

void BaseCurveFacade::Render(double t, IPluginObjectInstance *pInstance)
{
}

void BaseCurveFacade::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
}

void BaseCurveFacade::PostRender(double t, IPluginObjectInstance *pInstance)
{
}

YaptCurveFacade::YaptCurveFacade()
{
}

void YaptCurveFacade::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	pCurve = NULL;
	this->channels = pInstance->CreateProperty("channels", kPropertyType_Integer, "1", "");
	// make sure the curvetype matches the kCurveClass_ in Curve.h
	this->curveType = pInstance->CreateProperty("type",kPropertyType_Enum, "kbspline", "enum={unknown,hold,linear,linearsmooth,cubic,hermite,kbspline,kbquatspline}");
	this->curveType->v->int_val = (int)kCurveClass_Hermite; // this is the default
	this->testVector = pInstance->CreateProperty("vector",kPropertyType_Vector, "1", "");
	// TODO: Fix this, need to create this one during post-initialize since it depends on the number of channels.. 
	this->result = pInstance->CreateOutputProperty("result",kPropertyType_Vector, "", "");
	this->result->v->vector[0] = 1.0f;
	this->result->v->vector[1] = 2.0f;
	this->result->v->vector[2] = 4711.0f;

	//this->channels = ySys->CreateProperty(dynamic_cast<IPluginObject *>(this), "channels", kPropertyType_Integer, "1");
	//// make sure the curvetype matches the kCurveClass_ in Curve.h
	//this->curveType = ySys->CreateProperty(dynamic_cast<IPluginObject *>(this),"type",kPropertyType_Enum, "enum={unknown,hold,linear,linearsmooth,cubic,hermite,kbspline,kbquatspline}");
	//this->curveType->v->int_val = (int)kCurveClass_Hermite; // this is the default
	//this->testVector = ySys->CreateProperty(dynamic_cast<IPluginObject *>(this),"vector",kPropertyType_Vector, "1");
	//// TODO: Fix this, need to create this one during post-initialize since it depends on the number of channels.. 
	//this->result = ySys->CreateOutputProperty(dynamic_cast<IPluginObject *>(this), "result",kPropertyType_Vector, "");
}

void YaptCurveFacade::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = ySys->GetLogger("YaptCurveFacade");//Logger::GetLogger("YaptCurveFacade");
	pLogger->Debug("PostInitialize");
	if (pCurve != NULL)
	{
		pLogger->Debug("Dispose curve - not implemented, leaking memory");
	}
	pCurve = Curve::CreateCurve(kCurveClass(curveType->v->int_val), channels->v->int_val);

	IDocNode *pNode = pInstance->GetDocumentNode();
	int nChildren = pNode->GetNumChildren(kNodeType_ObjectInstance);
	pLogger->Debug("Assigning keys from children (num childs = '%d')",nChildren);
	int i;
	pLogger->Enter();
	for (i=0;i<nChildren;i++)
	{
		//		IPluginObjectInstance *pObject = pDoc->GetChildAt(pInstance, i, kNodeType_ObjectInstance);
		IDocNode *pChildNode = pNode->GetChildAt(i, kNodeType_ObjectInstance);
		IPluginObjectInstance *pObject = dynamic_cast<IPluginObjectInstance *>(pChildNode->GetNodeObject());
		GenericCurveKey *pCurveKey = dynamic_cast<GenericCurveKey *> (pObject->GetExtObject());
		if (pCurveKey != NULL)
		{
			Key *pKey = pCurveKey->GetKey();
			pCurve->AddKey(pKey);
		} else
		{
			pLogger->Error("Unsupported child type, Animation curves only supports 'GenericKey' derivates");
		}
	}
	pLogger->Leave();
}

void YaptCurveFacade::Render(double t, IPluginObjectInstance *pInstance)
{
	if (pCurve != NULL)
	{
		double *val = (double *)alloca(sizeof(double) * channels->v->int_val);
		
		FILE *fOut;
		fOut = fopen("samples.csv","w");
		for (int i=0;i<50;i++)
		{
			double t;
			t = (double)i/10.0f;
			pCurve->Sample(t, val);
			// pLogger->Debug("%f,%f",t,val[0]);
			fprintf(fOut,"%f,%f\n",t,val[1]);
		}
		fclose(fOut);		
	}
}

GenericCurveKey::GenericCurveKey()
{
	pKey = NULL;
}

Key *GenericCurveKey::GetKey()
{
	return pKey;
}

void GenericCurveKey::ParseValueString()
{
	char tmp[256];
	strncpy(tmp,values->v->string,256);
	char *pStart, *pNext;
	pStart = tmp;
	
	while ((pNext = strchr(pStart,',')) != NULL)
	{
		*pNext = '\0';
		pKey->values.push_back(atof(pStart));
		pStart = pNext+1;
	}
	pKey->values.push_back(atof(pStart));
}

void GenericCurveKey::RegisterValueType(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	//values = ySys->CreateProperty(dynamic_cast<IPluginObject *>(this), "values", kPropertyType_String, "0.0");
	values = pInstance->CreateProperty("values", kPropertyType_String, "0.0", "");
}

void GenericCurveKey::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	t = pInstance->CreateProperty("t",kPropertyType_Float, "0.0", "");
	tension = pInstance->CreateProperty("tension", kPropertyType_Float, "0.0", "");
	bias = pInstance->CreateProperty("bias", kPropertyType_Float, "0.0", "");
	continity = pInstance->CreateProperty("continity", kPropertyType_Float, "0.0", "");
	RegisterValueType(ySys, pInstance);
}

void GenericCurveKey::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	if (pKey == NULL)
	{
		pKey = new Key();
	}
	pKey->t = t->v->float_val;
	pKey->tension = tension->v->float_val;
	pKey->bias = bias->v->float_val;
	
	ParseValueString();
}

void VectorCurveKey::ParseValueString()
{
	pKey->values.push_back(values->v->vector[0]);
	pKey->values.push_back(values->v->vector[1]);
	pKey->values.push_back(values->v->vector[2]);
}

void VectorCurveKey::RegisterValueType(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	values = pInstance->CreateProperty("values", kPropertyType_Vector, "0.0,0.0,0.0", "");
}

void GenericCurveKey::Render(double t, IPluginObjectInstance *pInstance)
{
}

extern "C" {
    static  double cbExpVariable(void *pUser, const char *pData, int *bOk_out) {
        YaptExpSolverFacade *pFacade = (YaptExpSolverFacade *)pUser;
        return pFacade->OnConstantUserExpression(pData, bOk_out);
    }    
    static  double cbExpFunc(void *pUser, const char *pData, double arg, int *bOk_out) {
        YaptExpSolverFacade *pFacade = (YaptExpSolverFacade *)pUser;
        return pFacade->OnFunctionExpression(pData, arg, bOk_out);
    }    
}

double YaptExpSolverFacade::OnConstantUserExpression(const char *data, int *bOk_out) {
    double result = 0.0;
    *bOk_out = 0;
    if (!strcmp(data,"t")) {
        *bOk_out = 1;
        result = timeCurrent;
    } else  if (!strcmp(data,"apa")) {
        *bOk_out = 1;
        result = sin(timeCurrent);
    }
    
    return result;
}
double YaptExpSolverFacade::OnFunctionExpression(const char *data, double arg, int *bOk_out) {
    double result = 0.0;
    *bOk_out = 0;
    if (!strcmp(data,"sin")) {
        *bOk_out = 1;
        result = sin(arg);
    } else  if (!strcmp(data,"cos")) {
        *bOk_out = 1;
        result = cos(arg);
    } else  if (!strcmp(data,"abs")) {
        *bOk_out = 1;
        result = fabs(arg);
    }    
    
    return result;
}

void YaptExpSolverFacade::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
    expression = pInstance->CreateProperty("expression",kPropertyType_String,"0.0","");
    result = pInstance->CreateOutputProperty("result",kPropertyType_Float, "", "");
}

void YaptExpSolverFacade::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
    pExpSolver = new ExpSolver(expression->v->string);
    pExpSolver->RegisterUserVariableCallback(cbExpVariable,this);
    pExpSolver->RegisterUserFunctionCallback(cbExpFunc,this);
    pExpSolver->Prepare();    
}

void YaptExpSolverFacade::Render(double t, IPluginObjectInstance *pInstance) {
    timeCurrent = t;
    result->v->float_val = pExpSolver->Evaluate();
}



static void perror()
{
}

// This function must be exported from the lib/dll
int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.GenericCurve");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.Key");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.VectorKey");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.ExpSolver");
	return 0;
}
