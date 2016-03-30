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
using namespace gnilk;	// Expression solver

ISystem *pSysPtr; // Used by curve to query the logger..

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
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
};

class VectorMux :
	public IPluginObject
{
protected:
	Property *xv, *yv, *zv;
	Property *vectorResult;
public:
	VectorMux();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
};

class IntMinMax :
	public IPluginObject
{
protected:
	Property *iMin, *iMax, *iVal;
	Property *result;
public:
	IntMinMax();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
};

class FloatToInt :
	public IPluginObject
{
protected:
	Property *floatVal;
	Property *intVal;
public:
	FloatToInt();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
};

class VectorElement :
	public IPluginObject
{
protected:
	Property *vectorval;
	Property *index;
	Property *result;
public:
	VectorElement();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
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
	Property *tScale;
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
    Property *var_a;
    Property *var_b;
    Property *expression;
    Property *result;   // output

    double timeCurrent; // corrsponds to the 't' variable in the expression
    ExpSolver *pExpSolver; 
    // events
public:
    virtual double OnConstantUserExpression(const char *data, int *bOk_out);
    virtual double OnFunctionExpression(const char *data, int args, double *arg, int *bOk_out);
    // interface
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};

class YaptVecExpSolverFacade :
	public YaptExpSolverFacade
{
protected:
	Property *vec_a, *vec_b;

    Property *expression_x;
    Property *expression_y;
    Property *expression_z;
    Property *result;   // output

    ExpSolver *pExpSolver_x; 
    ExpSolver *pExpSolver_y; 
    ExpSolver *pExpSolver_z; 
    // interface
public:
	virtual double OnConstantUserExpression(const char *data, int *bOk_out);

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
	if (!strcmp(identifier,"Animation.Expression"))
	{
		pObject = dynamic_cast<IPluginObject *> (new YaptExpSolverFacade());
	}
	if (!strcmp(identifier,"Animation.VectorExpression"))
	{
		pObject = dynamic_cast<IPluginObject *> (new YaptVecExpSolverFacade());
	}
	if (!strcmp(identifier,"Numeric.VectorMux"))
	{
		pObject = dynamic_cast<IPluginObject *> (new VectorMux());
	}
	if (!strcmp(identifier,"Numeric.IntMinMax"))
	{
		pObject = dynamic_cast<IPluginObject *> (new IntMinMax());
	}
	if (!strcmp(identifier,"Numeric.FloatToInt"))
	{
		pObject = dynamic_cast<IPluginObject *> (new FloatToInt());
	}
	if (!strcmp(identifier,"Numeric.VectorElement"))
	{
		pObject = dynamic_cast<IPluginObject *> (new VectorElement());
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
	this->curveType = pInstance->CreateProperty("type",kPropertyType_Enum, "kbspline", "enum={hold,linear,linearsmooth,cubic,hermite,kbspline,kbquatspline}");
	//this->curveType->v->int_val = (int)kCurveClass_Hermite; // this is the default
	this->testVector = pInstance->CreateProperty("vector",kPropertyType_Vector, "1", "");

	this->tScale = pInstance->CreateProperty("timescale",kPropertyType_Float, "1", "");
	// TODO: Fix this, need to create this one during post-initialize since it depends on the number of channels.. 
	this->result = pInstance->CreateOutputProperty("result",kPropertyType_Vector, "", "");
	this->result->v->vector[0] = 1.0f;
	this->result->v->vector[1] = 2.0f;
	this->result->v->vector[2] = 4711.0f;
}

void YaptCurveFacade::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = ySys->GetLogger("YaptCurveFacade");//Logger::GetLogger("YaptCurveFacade");
	pLogger->Debug("PostInitialize");
	if (pCurve != NULL)
	{
		pLogger->Debug("Dispose curve - not implemented, leaking memory");
	}

	// 'unknown' not supported (it's pointless) so just add '1' to the incoming type
	pCurve = Curve::CreateCurve(kCurveClass(curveType->v->int_val+1), channels->v->int_val);

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
			if (pKey != NULL) {
				pKey->t *= tScale->v->float_val;
				pCurve->AddKey(pKey);
				pLogger->Debug("Added key '%d' at t=%f",i,pKey->t);				
			} else {
				pLogger->Debug("Key is NULL!");
				exit(1);
			}
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

		pCurve->Sample(t, val);
		//pLogger->Debug("%f,%f",t,val[0]);
		//printf("%f,%f\n",t,val[0]);
		for (int i=0;i<channels->v->int_val;i++) {
			result->v->vector[i] = (float)val[i];
		}

		// double *val = (double *)alloca(sizeof(double) * channels->v->int_val);
		
		// FILE *fOut;
		// fOut = fopen("samples.csv","w");
		// for (int i=0;i<50;i++)
		// {
		// 	double t;
		// 	t = (double)i/10.0f;
		// 	pCurve->Sample(t, val);
		// 	pLogger->Debug("%f,%f",t,val[0]);
		// 	fprintf(fOut,"%f,%f\n",t,val[0]);
		// }
		// fclose(fOut);		
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

	if (pKey == NULL)
	{
		pKey = new Key();
	}

}

void GenericCurveKey::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ySys->GetLogger("GenericCurveKey")->Debug("PostInitialize");
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

    static  double CALLCONV cbExpVariable(void *pUser, const char *pData, int *bOk_out) {
        YaptExpSolverFacade *pFacade = (YaptExpSolverFacade *)pUser;
        return pFacade->OnConstantUserExpression(pData, bOk_out);
    }    
    static  double CALLCONV cbExpFunc(void *pUser, const char *pData, int args, double *arg, int *bOk_out) {
        YaptExpSolverFacade *pFacade = (YaptExpSolverFacade *)pUser;
        return pFacade->OnFunctionExpression(pData, args, arg, bOk_out);
    }    
}

double YaptExpSolverFacade::OnConstantUserExpression(const char *data, int *bOk_out) {
    double result = 0.0;
    *bOk_out = 0;
    if (!strcmp(data,"t")) {
        *bOk_out = 1;
        result = timeCurrent;
    } else if (!strcmp(data,"a")) {
    	*bOk_out = 1;
    	result = var_a->v->float_val;
    } else if (!strcmp(data,"b")) {
    	*bOk_out = 1;
    	result = var_b->v->float_val;
    } else  if (!strcmp(data,"apa")) {
        *bOk_out = 1;
        result = sin(timeCurrent);
    }
    
    return result;
}

double YaptExpSolverFacade::OnFunctionExpression(const char *data, int args, double *arg, int *bOk_out) {
    double result = 0.0;
    *bOk_out = 0;
    if (!strcmp(data,"sin")) {
        *bOk_out = 1;
        result = sin(arg[0]);
    } else  if (!strcmp(data,"cos")) {
        *bOk_out = 1;
        result = cos(arg[0]);
    } else  if (!strcmp(data,"abs")) {
        *bOk_out = 1;
        result = fabs(arg[0]);
    } else  if ((!strcmp(data,"pow")) && (args == 2)) {
        *bOk_out = 1;
        result = pow(arg[0],arg[1]);
    } else if (!strcmp(data, "smooth")) {
    	*bOk_out = 1;
    	result = SMOOTHSTEP(arg[0]);
    }
    
    return result;
}

void YaptExpSolverFacade::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	var_a = pInstance->CreateProperty("var_a", kPropertyType_Float,"0.0","");
	var_b = pInstance->CreateProperty("var_b", kPropertyType_Float,"0.0","");
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

// vector expression
double YaptVecExpSolverFacade::OnConstantUserExpression(const char *data, int *bOk_out) {
    double result = 0.0;
    *bOk_out = 0;
    if (!strcmp(data,"va.x")) {
        *bOk_out = 1;
        result = vec_a->v->vector[0];
    } else if (!strcmp(data,"va.y")) {
    	*bOk_out = 1;
    	result = vec_a->v->vector[1];
    } else if (!strcmp(data,"va.z")) {
    	*bOk_out = 1;
    	result = vec_a->v->vector[2];
    } else if (!strcmp(data,"vb.x")) {
        *bOk_out = 1;
        result = vec_b->v->vector[0];
    } else if (!strcmp(data,"vb.y")) {
    	*bOk_out = 1;
    	result = vec_b->v->vector[1];
    } else if (!strcmp(data,"vb.z")) {
    	*bOk_out = 1;
    	result = vec_b->v->vector[2];
    } else {
    	result = YaptExpSolverFacade::OnConstantUserExpression(data, bOk_out);
    }
    return result;
}

void YaptVecExpSolverFacade::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	vec_a = pInstance->CreateProperty("vec_a", kPropertyType_Float,"0.0","");
	vec_b = pInstance->CreateProperty("vec_b", kPropertyType_Float,"0.0","");
    expression_x = pInstance->CreateProperty("x",kPropertyType_String,"0.0","");
    expression_y = pInstance->CreateProperty("y",kPropertyType_String,"0.0","");
    expression_z = pInstance->CreateProperty("z",kPropertyType_String,"0.0","");
    result = pInstance->CreateOutputProperty("result",kPropertyType_Vector, "", "");
}

void YaptVecExpSolverFacade::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
    pExpSolver_x = new ExpSolver(expression_x->v->string);
    pExpSolver_x->RegisterUserVariableCallback(cbExpVariable,this);
    pExpSolver_x->RegisterUserFunctionCallback(cbExpFunc,this);
    pExpSolver_x->Prepare();    

    pExpSolver_y = new ExpSolver(expression_y->v->string);
    pExpSolver_y->RegisterUserVariableCallback(cbExpVariable,this);
    pExpSolver_y->RegisterUserFunctionCallback(cbExpFunc,this);
    pExpSolver_y->Prepare();    

    pExpSolver_z = new ExpSolver(expression_z->v->string);
    pExpSolver_z->RegisterUserVariableCallback(cbExpVariable,this);
    pExpSolver_z->RegisterUserFunctionCallback(cbExpFunc,this);
    pExpSolver_z->Prepare();    

}

void YaptVecExpSolverFacade::Render(double t, IPluginObjectInstance *pInstance) {
    timeCurrent = t;
    result->v->vector[0] = pExpSolver_x->Evaluate();
    result->v->vector[1] = pExpSolver_y->Evaluate();
    result->v->vector[2] = pExpSolver_z->Evaluate();
}


VectorMux::VectorMux() {

}

void VectorMux::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	xv = pInstance->CreateProperty("x", kPropertyType_Float, "0.0", "");
	yv = pInstance->CreateProperty("y", kPropertyType_Float, "0.0", "");
	zv = pInstance->CreateProperty("z", kPropertyType_Float, "0.0", "");
	vectorResult = pInstance->CreateOutputProperty("vector", kPropertyType_Vector, "0.0,0.0,0.0", "");
}
void VectorMux::Render(double t, IPluginObjectInstance *pInstance) {
	vectorResult->v->vector[0] = xv->v->float_val;
	vectorResult->v->vector[1] = yv->v->float_val;
	vectorResult->v->vector[2] = zv->v->float_val;
}
void VectorMux::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void VectorMux::PostRender(double t, IPluginObjectInstance *pInstance) {

}

IntMinMax::IntMinMax() {

}
void IntMinMax::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	iMin = pInstance->CreateProperty("min", kPropertyType_Integer, "0", "");
	iMax = pInstance->CreateProperty("max", kPropertyType_Integer, "0", "");
	iVal = pInstance->CreateProperty("value", kPropertyType_Integer, "0", "");
	result = pInstance->CreateOutputProperty("result", kPropertyType_Integer, "", "");
}
void IntMinMax::Render(double t, IPluginObjectInstance *pInstance) {

	int v = iVal->v->int_val;

	v = v<iMin->v->int_val?iMin->v->int_val:v;
	v = v>iMax->v->int_val?iMax->v->int_val:v;

	result->v->int_val = v;

}
void IntMinMax::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void IntMinMax::PostRender(double t, IPluginObjectInstance *pInstance) {

}

FloatToInt::FloatToInt() {

}

void FloatToInt::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	floatVal = pInstance->CreateProperty("float",kPropertyType_Float, "0.0", "");
	intVal = pInstance->CreateOutputProperty("int", kPropertyType_Integer, "0", "");
}
void FloatToInt::Render(double t, IPluginObjectInstance *pInstance) {
	intVal->v->int_val = floatVal->v->float_val;
}
void FloatToInt::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void FloatToInt::PostRender(double t, IPluginObjectInstance *pInstance) {

}

VectorElement::VectorElement() {

}

void VectorElement::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	vectorval = pInstance->CreateProperty("vector",kPropertyType_Vector, "0,0,0", "");
	index = pInstance->CreateProperty("index", kPropertyType_Integer, "0", "");
	result = pInstance->CreateOutputProperty("float", kPropertyType_Float, "0", "");
}
void VectorElement::Render(double t, IPluginObjectInstance *pInstance) {
	int idx = index->v->int_val;
	idx = idx>2?2:idx;
	idx = idx<0?0:idx;
	result->v->float_val = vectorval->v->vector[idx];
}
void VectorElement::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void VectorElement::PostRender(double t, IPluginObjectInstance *pInstance) {

}

static void perror()
{
}

// This function must be exported from the lib/dll
int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
  	pSysPtr = ySys;
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.GenericCurve");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.Key");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.VectorKey");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.ExpSolver");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.Expression");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Animation.VectorExpression");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Numeric.VectorMux");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Numeric.IntMinMax");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Numeric.FloatToInt");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Numeric.VectorElement");

	return 0;
}
