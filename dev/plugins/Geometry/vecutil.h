#ifndef __VECUTIL_H__
#define __VECUTIL_H__

#include "vec.h"

#ifdef __cplusplus
extern "C" {
#endif

void linelinedistance(float *v1, float *a1, float *v2, float *a2, float *t1, float *t2);
float rayplaneintersect(float *raypos, float *raydir, float *planep, float *planen);
float rotinteract(float *pivot, float *axis, float *pickpoint, float *raypos, float *raydir);
float raysphereintersect(float *raypos, float *raydir, float *origo, float rad);
float raysphereintersectback(float *raypos, float *raydir, float *origo, float rad);
float linepointdistance(float *raypos, float *raydir, float *point);
float *projectpointonplane(float *point, float *planep, float *planen, float *pointret);
float n_raysphereintersect(float *raypos, float *raydir, float *origo, float rad);

#ifdef __cplusplus
}
#endif

#endif