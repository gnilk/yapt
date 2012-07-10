/*-------------------------------------------------------------------------
File    : $Archive: Curve.h $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-23, 15:50
Descr   : Defines some fundamental interpolation schemes

* Hold
* Linear
* LinearSmooth
* Cubic
* Hermite
* KBSpline
* KBQuatSpline

Each curve interpolates between at least two keys. The Key structure is generic
and supports all curve types.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 23.09.09, FKling, Implementation

---------------------------------------------------------------------------*/
#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

using namespace yapt;

namespace Goat
{
	typedef enum
	{
		kCurveType_Regular,
		kCurveType_Quat,
		kCurveType_Quat_3DS,
	} kCurveType;

	typedef enum
	{
		kCurveClass_Unknown,
		kCurveClass_Hold,
		kCurveClass_Linear,
		kCurveClass_LinearSmooth,
		kCurveClass_Cubic,
		kCurveClass_Hermite,
		kCurveClass_KBSpline,
		kCurveClass_KBQuatSpline,
	} kCurveClass;

	class Key
	{
	public:
		Key();
		Key(int nChannels);

		double t;
		double tension, bias, continuity;	// TCB splinescontinuity
		double easein, easeout;			// TODO: Support this
		std::vector<double> values;
	};

	// Base class for multichannel curves
	class Curve
	{
	protected:
		yapt::ILogger *pLogger;
		int nChannels;
		kCurveType curveType;
		kCurveClass curveClass;
		std::vector<Key *> keys;		
	protected:
		void SortKeys();
		int FindStartKey(double t);
		int FindEndKey(double t);
		// Each curve type deriving from this class should implement this function
		// where, t = the time between the keys indicated by idxStart and idxEnd
		// dest is where the computed values should be stored
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	public:
		Curve(int nChannels, kCurveClass cClass);
		virtual Key *AddKey(double t);
		virtual Key *AddKey(Key *key);
		void SetValue(Key *pKey, int channel, double v);
		void SetValue(Key *pKey, double *v);
		kCurveType ChangeType(kCurveType newType);
		Key *GetKey(int idx);

		void DumpKeys(void);

		void Save(const char *filename);

		virtual void Sample(double t, double *dest);

		static Curve *Load(const char *filename);
		static char *GetClassName(kCurveClass curveClass, char *sDest, int nMax);
		static Curve *CreateCurve(kCurveClass curveClass, int nChannels);
	};

#define LERP(t,a,b) ((a) + (t)*((b)-(a)))
#define SMOOTHSTEP(t) ((t)*(t)*(3.0-2.0*(t)))

	// some fundamental interpolation methods..
	class Hold:
		public Curve
	{
	public:
		Hold(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	};
	class Linear :
		public Curve
	{
	public:
		Linear(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	};
	class LinearSmooth :
		public Curve
	{
	public:
		LinearSmooth(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	};
	class Cubic :
		public Curve
	{
	public:
		Cubic(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	};
	class Hermite : 
		public Curve
	{
	public:
		Hermite(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);	
	};
	class KBSpline :
		public Curve
	{
	protected:
		std::vector<double> ta;
		std::vector<double> tb;
	public:
		KBSpline(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);	

	};
	// requires input in quaternion format
	// q={x,y,z,w}, thus 4 channels!!
	class KBQuatSpline:
		public Curve
	{
	protected:
		std::vector<double> ta;
		std::vector<double> tb;
	public:
		KBQuatSpline(int nChannels);
	protected:
		virtual void Interpolate(double t, int idxStart, int idxEnd, double *dest);
	};


}
