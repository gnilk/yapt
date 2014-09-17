#pragma once

#include <unordered_map>
#include <math.h>

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"

#include "vec.h"



using namespace yapt;

class Ray;
class RayTracer;

class ITraceObject {
public:
	virtual float Hit(Ray *ray);
	virtual void HitPoint(Ray *ray);
	virtual RayTracer *GetTracer(ISystem *pSys);
};

class Ray {
public:
	float dir[3];
	float pos[3];
	float hitpoint[3];
	float normal[3];
	ITraceObject *hitObject;
	float t;
	bool cached;
	unsigned long hashkey;
	int tracecount;

	// void Reset() {
	// 	t = 1000;
	// }
	// void SetDir(float x, float y, float z) {
	// 	vIni(dir,x,y,z);
	// 	vNorm(dir,dir);
	// }
	// void SetPos(float x, float y, float z) {
	// 	vIni(pos,x,y,z);
	// }

	void PrepareForTrace(float px, float py, float pz, float dx, float dy, float dz) {
		t = 1000;
		hitObject = NULL;
		vIni(pos,px,py,pz);

		vIni(dir,dx,dy,dz);
		vNorm(dir,dir);
	}

	void HitPoint() {
		hitObject->HitPoint(this);
	}
	ITraceObject *HitObject() {
		return hitObject;
	}
	bool IsHit() {
		return (hitObject != NULL);
	}
	int Hash() {
		// primes from some paper on the internet - had this in old code
		unsigned long xi = (unsigned long)(pos[0] * 73856093);
    	unsigned long yi = (unsigned long)(pos[1] * 19349663);
    	unsigned long zi = (unsigned long)(pos[2] * 83492791);
    	unsigned long h = (xi)^(yi)^(zi) ; 
    	return h;
	}

	bool IsSamePos(float *v) {
		if (fabs(v[0]-pos[0]) > 0.001) return false;
		if (fabs(v[1]-pos[1]) > 0.001) return false;
		if (fabs(v[2]-pos[2]) > 0.001) return false;

		return true;
	}
	void SetCached(bool isFromCache) {
		cached = isFromCache;
	}
	bool IsFromCache() {
		return cached;
	}
	void SetHashKey(unsigned long key) {
		tracecount = 0;
		hashkey = key;
	}
	unsigned long GetHashKey() {
		return hashkey;
	}
	void IncTraceCount() {
		tracecount++;
	}
	int GetTraceCount() {
		return tracecount;
	}
};


template <class T> class ObjectPool {
private:
	int count;
	std::vector<T *> objects;
public:
	ObjectPool(int max) {
		for(int i=0;i<max;i++) {
			objects.push_back(new T());
		}
		count = 0;
	}

	virtual T* At(int index) {
		return objects.at(index);
	}

	virtual void Reset() {
		count = 0;
	}	

	virtual int Count() {
		return count;
	}

	virtual T* Get() {
		return objects[count];
	}

	virtual void Retain() {
		count++;
		if (count > (objects.size() - 1)) {
			printf("Pool full (%d)\n",count);
			exit(1);
		}
	}
};


class Cell
{
private:
	Ray *topleft, *topright, *bottomright, *bottomleft;
public:
	int Trace(RayTracer *tracer, float x, float y, float x_step, float y_step, int depth);
private:
	Ray *NewRay(RayTracer *tracer, float x, float y);
};


class RayTracer: public PluginObjectImpl {
private:
	Property *width;
	Property *height;
	Property *xmin;
	Property *ymin;
	Property *numhits;
	ISystem *pSys;

	// int raycount;
	// std::vector<Ray *>raypool;
	ObjectPool<Cell> *cellpool;
	ObjectPool<Ray> *raypool;
	std::vector<std::vector<Ray *> > raycache;
	std::vector<ITraceObject *> objects;


	int tracecount;

	float x_step;
	float y_step;

public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
public:
	void PushObject(ITraceObject *pObject);
	Ray *GetRay(float *v);
	void CacheRay(Ray *ray);
	ObjectPool<Cell> *GetCellPool() { return cellpool; }
	ObjectPool<Ray> *GetRayPool()  { return raypool; }
	bool Trace(Ray *ray);
private:
	void ClearRayCache();
	int DrawGrid();
	int CastPrimaryRays();
	void ProcessPrimaryRays();

};

class Sphere: 
	public PluginObjectImpl,
	public ITraceObject {
private:
	Property *radius;
	Property *position;
	float radius_square;
	ISystem *pSys;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
public:
	virtual float Hit(Ray *ray);
	virtual void HitPoint(Ray *ray);
};
