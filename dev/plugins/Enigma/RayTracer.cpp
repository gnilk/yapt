#include "yapt/ySystem.h"
#include "yapt/logger.h"
#include <unordered_map>

#include "RayTracer.h"
#include "vec.h"
#include "vecutil.h"

#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

//
// NOTE - this won't be optimized, will trace basically everything twice
//

int Cell::Trace(RayTracer *tracer, float x, float y, float x_step, float y_step, int depth) {

	Ray *ray = NULL;
	ObjectPool<Ray> *raypool = tracer->GetRayPool();

	if (depth > 2) return 0;

	int before = raypool->Count();
	// setup all Ray's for the cell..
	Ray *topleft = NewRay(tracer, x, y);
	Ray *topright = NewRay(tracer, x+x_step, y);
	Ray *bottomright = NewRay(tracer, x+x_step, y+y_step);
	Ray *bottomleft = NewRay(tracer, x, y+y_step);


	int after = raypool->Count();
	int delta = after - before;

	if (delta == 0) return delta;

	x_step*=0.5;
	y_step*=0.5;
	// Need better control here..
	if ((topleft != NULL) && (topleft->IsHit())) Trace(tracer, x, y, x_step, y_step, depth+1);
	if ((topright != NULL) && (topright->IsHit())) Trace(tracer, x+x_step, y, x_step, y_step,depth+1);
	if ((bottomright != NULL) && (bottomright->IsHit())) Trace(tracer, x+x_step, y+y_step, x_step, y_step,depth+1);
	if ((bottomleft != NULL) && (bottomleft->IsHit())) Trace(tracer, x, y+y_step, x_step, y_step,depth+1);

	return delta;
}

Ray *Cell::NewRay(RayTracer *tracer, float x, float y) {
	ObjectPool<Ray> *raypool = tracer->GetRayPool();

	float pos[3], dir[3];
	vIni(pos, x*0.5, y*0.5, 0);	// Projection plane
	vIni(dir, x, y, 10);		// Calculated with projection plane sub camera pos

	Ray *ray = tracer->GetRay(pos);

	if (!ray->IsFromCache()) {
		//printf("Tc:%d\n",ray->GetTraceCount());
		//ray->PrepareForTrace(x*0.5, y*0.5, 0, x, y, 1);
		ray->PrepareForTrace(pos[0], pos[1], pos[2], dir[0], dir[1], dir[2]);
		tracer->CacheRay(ray);
		if (tracer->Trace(ray)) {
			return ray;
		} else {
			return NULL;
		}
	}

	// cached ray
	return ray;
}


void RayTracer::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	//for(int i=0;i<1024;i++) raypool.push_back(new Ray());
	//raypool = new RayPool(1024);
	raypool = new ObjectPool<Ray>(16384);
	cellpool = new ObjectPool<Cell>(4096);
	raycache.reserve(1024);
	for(int i=0;i<raycache.capacity();i++) {
		std::vector<Ray *> tmp;
		raycache.push_back(tmp);
	}

	width = pInstance->CreateProperty("width", kPropertyType_Float, "4", "");
	height = pInstance->CreateProperty("height", kPropertyType_Float, "2.25", "");
	xmin = pInstance->CreateProperty("xmin", kPropertyType_Float, "0.1", "");
	ymin = pInstance->CreateProperty("ymin", kPropertyType_Float, "0.1", "");

	numhits = pInstance->CreateProperty("numhits", kPropertyType_Integer, "0", "");
}

void RayTracer::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void RayTracer::Render(double t, IPluginObjectInstance *pInstance) {
	objects.clear();
}

void RayTracer::PostRender(double t, IPluginObjectInstance *pInstance) {
	float w = width->v->float_val;
	float h = height->v->float_val;

	x_step = (1.0 * w) / 32.0;
	y_step = (1.0 * h) / 18.0;

	printf("clear, cells: %d, rays: %d\n",cellpool->Count(), raypool->Count());
	raypool->Reset();
	cellpool->Reset();
	ClearRayCache();
	tracecount = 0;
	//printf("trace\n");

	//printf("%f,%f - %f,%f\n",w,h,x_step, y_step);
	int hits = CastPrimaryRays();
	int count = DrawGrid();
	//printf("T: %d\n",tracecount);

	//printf("%d,%d,%d,%d\n",count, hits, tracecount);
}

int RayTracer::CastPrimaryRays() {

	float w = width->v->float_val;
	float h = height->v->float_val;

	float v[3],zv[3];
	vIni(v, -w,0,4);	
	vNorm(v,v);
	vIni(zv,0,0,1);

	float dt = vDot(v,zv);
	//printf("dt: %f (%f)\n",dt,acos(dt));


	for (float x=-w;x<(w-x_step);x+=x_step) {
		for (float y=-h;y<(h-y_step);y+=y_step) {
			Cell *cell = cellpool->Get();
			if (cell->Trace(this, x, y, x_step, y_step, 0) > 0) {
				cellpool->Retain();
			}
		}
	}
	return raypool->Count();
}

void RayTracer::ProcessPrimaryRays() {
	// for (auto& rme: raymap) {
	// 	Ray *ray = rme.second;
 // 	}

}

bool RayTracer::Trace(Ray *ray) {
	bool hit = false;
	tracecount++;
	for(int i=0;i<objects.size();i++) {
		float t = objects[i]->Hit(ray);
		if ((t>0) && (t<ray->t)) {
			// 1) Compute hit position
			// 2) Check if closer than previous
			// 3) Set object to ray
			// 4) return true
			ray->hitObject = objects[i];
			ray->t = t;
			hit = true;
		}
	}
	ray->IncTraceCount();
	return hit;
}

int RayTracer::DrawGrid() {

	glBegin(GL_POINTS);
	int num = raypool->Count();
	for(int i=0;i<num;i++) {
		Ray *ray = raypool->At(i);
		if (ray->IsHit()) glColor3f(1,0,0);
		else glColor3f(1,1,1);

		glVertex3f(ray->pos[0], ray->pos[1], ray->pos[2]);					
	}
	glEnd();
	return num;
}

Ray *RayTracer::GetRay(float *v) {
	unsigned long key = vHash(v);
	key = key & 1023;

	if (raycache[key].size() > 0) {
		for(int i=0;i<raycache[key].size();i++) {
			Ray *ray = raycache[key].at(i);
			if (ray == NULL) {
				printf("NULL Ray found for key=%d at pos: %d (size=%d)\n",key,i, raycache[key].size());
				exit(1);
			}
			if (ray->IsSamePos(v)) {
				ray->SetCached(true);
				return raycache[key].at(i);
			}
		}
	}
	Ray *ray = raypool->Get();
	ray->SetCached(false);
	ray->SetHashKey(key);
	return ray;
}

void RayTracer::CacheRay(Ray *ray) {
	// Need to know if this ray was brought up from the cache or not..
	if (ray->IsFromCache()) return;	// no need to cache this one
	unsigned long key = ray->GetHashKey();
	raycache[key].push_back(ray);
	raypool->Retain();
}

void RayTracer::ClearRayCache() {
//	printf("Cache root size: %d\n",raycache.size());
	for(int i=0;i<raycache.size();i++) {
//		printf("%d:%d\n",i,raycache.at(i).size());
		raycache.at(i).clear();
	}
}

void RayTracer::PushObject(ITraceObject *pObject) {
	this->objects.push_back(pObject);
}

/////////////

float ITraceObject::Hit(Ray *ray) {
	return -1;
}
void ITraceObject::HitPoint(Ray *ray) {
	vMul(ray->hitpoint,ray->dir,ray->t);
    vAdd(ray->hitpoint,ray->hitpoint,ray->pos);   
}
RayTracer *ITraceObject::GetTracer(ISystem *pSys) {
	IContext *ctx = pSys->GetCurrentContext();
	IPluginObjectInstance *plugObject = dynamic_cast<IPluginObjectInstance *>(ctx->TopRenderObject());
	if (plugObject == NULL) {
		printf("Error: TopRenderObject not IPluginObjectInstance\n");
		exit(1);
	}
	RayTracer *tracer = dynamic_cast<RayTracer *>(plugObject->GetExtObject());
	if (tracer == NULL) {
		printf("Error: GetExtObject not RayTraer\n");
		exit(1);
	}
	return tracer;
}

// Sphere object for tracer
void Sphere::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	this->pSys = ySys;
	radius = pInstance->CreateProperty("radius", kPropertyType_Float, "1.5", "");
	position = pInstance->CreateProperty("position", kPropertyType_Vector, "0,0,2", "");
}

void Sphere::Render(double t, IPluginObjectInstance *pInstance) {
	// 1) retrieve ray tracer from context
	// 2) add 'this' to object array
	// - precompute this every frame (if animated)

	RayTracer *tracer = GetTracer(this->pSys);

	// position->v->vector[0] = 2.0 * sin(t);
	// position->v->vector[1] = 0;
	// position->v->vector[2] = 2;

	radius_square = radius->v->float_val;
	radius_square *= radius_square;

	tracer->PushObject(this);
}

void Sphere::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void Sphere::PostRender(double t, IPluginObjectInstance *pInstance) {
	
}

float Sphere::Hit(Ray *ray) {
  //    float raysphereintersect(float *raypos, float *raydir, float *origo, float rad);
    float t;
    float a[3],b,c,tmp1[3],l;
    
    
    vSub(a,position->v->vector,ray->pos);
    b=vDot(a,ray->dir);

    if (b < 0)
        return -1;
    
    c=b*b-vDot(a,a)+radius_square;
    
    if (c<=0) return -1;
    
    
	float t1=(b-sqrt(c));
	float t2=(b+sqrt(c));

	if (t1<0 && t2<0) return -1;
	if (t1<0) return t2;
	if (t2<0) return t1;

	return min(t1,t2);
}

void Sphere::HitPoint(Ray *ray) {
	ITraceObject::HitPoint(ray);
    
    vSub (ray->normal,ray->hitpoint,position->v->vector);
    vNorm (ray->normal,ray->normal);    
}
