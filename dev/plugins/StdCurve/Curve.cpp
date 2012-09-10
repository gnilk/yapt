/*-------------------------------------------------------------------------
 File    : $Archive: Curve.cpp $
 Author  : $Author: FKling $
 Version : $Revision: 1 $
 Orginal : 2009-09-23, 15:50
 Descr   : Defines some simple Interplation curves
 
 * Hold
 * Linear
 * LinearSmooth
 * Cubic
 * Hermite
 
 Each curve interpolates between at least two keys. The Key structure is generic
 and supports all curve types.
 
 Modified: $Date: $ by $Author: Fkling $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
 ! Move to Goat library
 - Make it adhere to the templateification of Goat
 + Replace the use of "std::vector" in keys and curve to use "double *" (or T*) 
   causes problem for quat functions. or check portabillity fo vector.front()
 ! Extend with TCB (Kornack-Bartel - or what the name is) splines
 - Add Ease In/Out functions to base class
 ! Resort keys on add key
 [ Create two proper (extend in tangent direction) virtual key's at start/end of curve ]
 ! Add Quats form lerping rotational curves
 - Verify KB implementation
 - Add load save functionality, bring over implementation from GOA (will require class)
 ! Add factory from CurveClass to instance in order to support loading of curves
 ! Solve tokenizer issue in Load, want this to be independent, try not to depend on Goat
 </pre>
 
 
 \History
 - 23.09.09, FKling, Implementation
 - 26.09.09, FKling, Move into the Goat namespace
 - 29.09.09, FKling, Changed "CalcAt" to "Sample" suggestion by Krikkit
 - 30.09.09, FKling, Added KB Splines from GOA for rotation and position
 - 30.09.09, FKling, Factory for curve now implemented..
 - 01.10.09, FKling, Implemented simple tokenizer to in order to solve loading
 ---------------------------------------------------------------------------*/
#include <math.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "yapt/ySystem.h"
#include "yapt/logger.h"
#include "Curve.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

static const int CURVE_PARSE_MAGIC=666;	// this comes from GOA_KEYFRAME_SAVE - not needed

using namespace yapt;
using namespace Goat;
// Quats are included for now, until this is changed. I assume my type of implementation
// will cause some discussion
static double *qScale (double *qr, double *qa, double scale);
static double qDot (double *q1, double *q2);
static double* qAdd(double *q, double *q1, double *q2);
static double* qSlerp(double *q, double *q1, double *q2, double t);
static double *qDup(double *dst, double *src);

extern ISystem *pSysPtr;

Key::Key()
{
	tension = 0.0;
	bias = 0.0;
	continuity = 0.0;	
	easein = 0.0;
	easeout = 0.0;
}

Key::Key(int nChannels) 
{
	Key();
	values.reserve(nChannels);
}

Curve::Curve(int nChannels, kCurveClass cClass)
{
	this->nChannels = nChannels;
	curveType = kCurveType_Regular;	// change in order to hint
	curveClass = cClass;

	pLogger = pSysPtr->GetLogger("Curve");//Logger::GetLogger("Curve");
}

kCurveType Curve::ChangeType(kCurveType newType)
{
	kCurveType oldType = curveType;
	curveType = newType;
	return oldType;
}

void Curve::DumpKeys(void)
{
	int i;
	pLogger->Debug("Dumping keys");
	SortKeys();
	for (i=0;i<keys.size();i++)
	{
		pLogger->Debug("%d,%f",i,keys[i]->t);
	}
}

// bubble sort the keys in the array, normally the array has few keys (<100)
// thus the bubble sort is no problem, also, the this is a preprocess step..
void Curve::SortKeys()
{
	Key *tmp;
	int i,j;
	for(i=0;i<keys.size();i++)
	{
		for (j=0;j<keys.size();j++)
		{
			if ((keys[i]->t < keys[j]->t) && (i!=j))
			{
				tmp = keys[i];
				keys[i] = keys[j];
				keys[j] = tmp;
			}
		}
	}
}

//
// Adds a key and sorts the data
//
Key *Curve::AddKey(Key *pKey)
{
	if (pKey != NULL)
	{
		keys.push_back(pKey);
		SortKeys();
		pLogger->Debug("AddKey direct, t=%f",pKey->t);
	}
	return pKey;
}

//
// Creats a key at 't' and adds it to the curve
//
Key *Curve::AddKey(double t)
{	
	Key *pKey = new Key(nChannels);
	pKey->t = t;
	keys.push_back(pKey);
	SortKeys();
	pLogger->Debug("AddKey indirect, t=%f",t);
	return pKey;
}

Key *Curve::GetKey(int idx)
{
	if ((idx < 0) || (idx >= keys.size())) return NULL;		
	return keys[idx];
}

void Curve::SetValue(Key *pKey, int channel, double v)
{
	pLogger->Debug("SetVal for t=%f, v=%f",pKey->t,v);
	pKey->values[channel] = v;
}

void Curve::SetValue(Key *pKey, double *v)
{
	int i;
	for (i=0;i<nChannels;i++)
	{
		pKey->values[i] = v[i];
	}
}
int Curve::FindStartKey(double t)
{
	// must atleast have to keys to find start key
	if (keys.size() > 1)
	{
		for (int i=0;i<keys.size()-1;i++)
		{
			if ((t >= keys[i]->t) && (t <= keys[i+1]->t)) return i;
		}
	}
	return -1;
}
int Curve::FindEndKey(double t)
{
	if (keys.size() > 1)
	{
		for (int i=0;i<keys.size()-1;i++)
		{
			if ((t >= keys[i]->t) && (t <= keys[i+1]->t)) return i+1;
		}
	}
	return -1;
}

//
// Sample the curve at an exact position in time
//
void Curve::Sample(double t, double *dest)
{
	// 1. find the two values
	//	value_a->t < t < value_b->t
	int idxStart = FindStartKey(t);
	int idxEnd = FindEndKey(t);
	
	// Out of range
	if ((idxStart == -1) || (idxEnd == -1)) 
	{
		// TODO: have to solve this
		return;
	}

	// do 0..1 mapping of 't'
	double t_start = keys[idxStart]->t;
	double t_end = keys[idxEnd]->t;
	double t_adjust = (t - t_start) / (t_end - t_start);

	pLogger->Debug("Sample t=%f (%f), idxStart=%d (t_start=%f), idxEnd=%d (t_end=%f)",
				   t, t_adjust, idxStart, t_start, idxEnd, t_end);

	Interpolate(t_adjust, idxStart, idxEnd, dest);

}

//
// Interpolate, should  be implemented by the actual curve type
//
void Curve::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	// TODO: User should implement this
}

//
// Save the curve using ASCII 
//
void Curve::Save(const char *filename)
{
	FILE *f = fopen(filename,"w");
	if (f!=NULL)
	{
		char tmp[128];
		Curve::GetClassName(curveClass,tmp,128);
		fprintf(f,"// Generated by Goat::Curve::Save(..) \n\n");
		fprintf(f,"// flags, num_keys, num_channels\n");
		fprintf(f,"%d %d %d\n",(int)curveType, (int)keys.size(), nChannels);
		fprintf(f,"// class enum, class name\n");
		fprintf(f,"%d %s\n\n",curveClass, tmp);

		int i,j;
		for (i=0; i<keys.size(); i++) 
		{
			Key *k = keys[i];
			fprintf(f, "// key #%d\n", i);
			fprintf(f, "// t, tension, bias, continuity, easein, easeout\n");
			fprintf(f, "%f %f %f %f %f %f\n", k->t, k->tension, k->bias, k->continuity, k->easein, k->easeout);
			fprintf(f, "// channel data:\n");

			for (j=0; j<nChannels; j++) 
			{
				fprintf(f,"%f ", k->values[j]);
			}

			fprintf(f, "\n\n");
		}
		fclose(f);
	}
}

// digs out a token, separated by whitespace, from the input
// returns the starting offset for next call or zero if we reached the end of the string
static int StrToken(const char *sInput, int iOffset, char *dest, int nMax)
{
	int idx = iOffset;
	int iTarget=0;
	// skip leading space
	while(isspace(sInput[idx]))
	{
		if (sInput[idx]=='\0') return 0;
		idx++;
	}
	// copy data until white space starts
	while(!isspace(sInput[idx])) 
	{
		if (sInput[idx]=='\0') return 0;
		if (iTarget > (nMax-1)) break;	// protected destination string against buffer overflow
		dest[iTarget++]=sInput[idx++];
	}
	dest[iTarget]='\0';
	return idx;
}

//
// Loads a curve from disc
//
Curve *Curve::Load(const char *filename)
{
	ILogger *pLogger = pSysPtr->GetLogger("Curve");//Logger::GetLogger("Curve");

	Curve *pCurve = NULL;
	FILE *f = fopen(filename,"r");
	if (f != NULL)
	{
		int flags, nKeys, nChannels, classid;
		char className[128];
		char tok[128];
		double values[64];	// Max channels

		Key *pKey = NULL;
		flags = nKeys = nChannels = -1;

		int nLines = 0;
		int state = 0;	// parse state
		bool bDone = false;
		char tmp[256];

		pLogger->Debug("File '%s' opened ok, parsing data",filename);
		pLogger->Enter();
		while (!bDone)
		{
			nLines++;
			if (!fgets(tmp,256,f))
			{
				// EOF or error
				pLogger->Error("fgets returned 0, stopping",filename);
				bDone = true;
				continue;
			}
			if (((tmp[0]=='/') && (tmp[1]=='/')) || (tmp[0]=='#')) continue;
			if (strlen(tmp) < 2) continue;
			pLogger->Debug("state: %d, line=%s",state,tmp);
			switch(state)
			{
			case 0 :	// parse first info line
				sscanf(tmp,"%d %d %d",&flags, &nKeys, &nChannels);
				pLogger->Debug("flags=%d, keys=%d, channels=%d",flags, nKeys, nChannels);
				state++;
				break;
			case 1 :	// parse second info line & create class
				sscanf(tmp,"%d %s",&classid, className);
				pCurve = Curve::CreateCurve(kCurveClass(classid),nChannels);
				if (pCurve == NULL)
				{
					pLogger->Error("Unknown class with id=%d",classid);
					bDone = true;

				} else
				{
					pLogger->Debug("classid=%d (%s), CurveObject: %p",classid, Curve::GetClassName(kCurveClass(classid), tmp, 256),pCurve);
				}
				state++;
				break;
			case 2 :	// parse key header info
				{
					int idx = 0;
					int offset = 0;
					for(int i=0;i<6;i++) values[i]=0.0;
					while ((offset = StrToken(tmp, offset, tok, 128)))
					{
						values[idx++] = atof(tok);
					}
					if (idx < 5)
					{
						// error, not enough number of values
						pLogger->Warning("not enough values in key header, expected 6, got=%d",idx);
					}
					pKey = pCurve->AddKey(values[0]);
					pKey->tension = values[1];
					pKey->bias = values[2];
					pKey->continuity = values[3];
					pKey->easein = values[4];
					pKey->easeout = values[5];
					pLogger->Debug("t=%f, tension=%f, bias=%f, continuity=%f, easein=%f, easeout=%f",
						values[0], values[1], values[2], values[3], values[4], values[5]);
					state++;
				}
				break;
			case 3 :	// parse key channel data & add key
				{
					int i=0,offset=0;
					while((offset = StrToken(tmp, offset, tok, 128)))
					{
						values[i++]=atof(tok);
					}
					pLogger->Debug("%d values",i);
				}
				pCurve->SetValue(pKey, values);
				nKeys--;
				if (nKeys ==0)
				{
					pLogger->Debug("All keys parsed");
					bDone = true;
				}
				state = 2;
				break;
			}

		}
		pLogger->Leave();
		pLogger->Debug("Parsing stopped after %d lines",nLines);
		fclose(f);
	}
	return pCurve;
}

static char *classNameArray[]=
{
"Unknown",
"Hold",
"Linear",
"LinearSmooth",
"Cubic",
"Hermite",
"KBSpline",
"KBQuatSpline",
};
char *Curve::GetClassName(kCurveClass curveClass, char *sDest, int nMax)
{
	int idx = (int)curveClass;
	if ((idx >= 0) && (idx < 8))
	{
		snprintf(sDest,nMax,"%s",classNameArray[idx]);
	} else
	{
		snprintf(sDest,nMax,"Unknown");
	}

	return sDest;
}
Curve *Curve::CreateCurve(kCurveClass curveClass, int nChannels)
{
	Curve *pCurve = NULL;
	switch(curveClass)
	{
	case kCurveClass_Hold :
		pCurve = dynamic_cast<Curve *>(new Hold(nChannels));
		break;
	case kCurveClass_Linear:
		pCurve = dynamic_cast<Curve *>(new Linear(nChannels));
		break;
	case kCurveClass_LinearSmooth:
		pCurve = dynamic_cast<Curve *>(new LinearSmooth(nChannels));
		break;
	case kCurveClass_Cubic:
		pCurve = dynamic_cast<Curve *>(new Cubic(nChannels));
		break;
	case kCurveClass_Hermite:
		pCurve = dynamic_cast<Curve *>(new Hermite(nChannels));
		break;
	case kCurveClass_KBSpline:
		pCurve = dynamic_cast<Curve *>(new KBSpline(nChannels));
		break;
	case kCurveClass_KBQuatSpline:
		pCurve = dynamic_cast<Curve *>(new KBQuatSpline(nChannels));
		break;
	default:
		// issue warning
		break;
	}
	return pCurve;
}

// Holds just holds the incoming A value until we swap over and then holds the next
// Could be called "level"
Hold::Hold(int nChannels) : Curve(nChannels, kCurveClass_Hold)
{
	// do nothing..
}
void Hold::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pA;
	pA = GetKey(idxStart);
	for (i=0;i<nChannels;i++)
	{
		dest[i] = pA->values[i];
	}
}

//
// Interpolates linearly between A&B
//
Linear::Linear(int nChannels) : Curve(nChannels, kCurveClass_Linear)
{
	// do nothing..
}
void Linear::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pA, *pB;
	pA = GetKey(idxStart);
	pB = GetKey(idxEnd);
	for (i=0;i<nChannels;i++)
	{
		dest[i] = LERP(t, pA->values[i], pB->values[i]);
	}
}

// Interpolate linear SMOOTH between A&B (TODO: FIX)
LinearSmooth::LinearSmooth(int nChannels) : Curve(nChannels, kCurveClass_LinearSmooth)
{
	// do nothing..
}
void LinearSmooth::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pA, *pB;
	pA = GetKey(idxStart);
	pB = GetKey(idxEnd);
	t = SMOOTHSTEP(t);
	for (i=0;i<nChannels;i++)
	{
		dest[i] = LERP(t, pA->values[i], pB->values[i]);
	}
}

// Cubic interpolation between A&B
Cubic::Cubic(int nChannels) : Curve(nChannels, kCurveClass_Cubic)
{

}

static double CubicInterpolate(double y0,double y1,double y2,double y3,double mu)
{
	double a0,a1,a2,a3,mu2;
	
	mu2 = mu*mu;
	a0 = y3 - y2 - y0 + y1;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;
	
	return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}


void Cubic::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pPrev, *pA, *pB, *pPost;
	pA = GetKey(idxStart);
	pB = GetKey(idxEnd);
	pPrev = GetKey(idxStart -1);
	pPost = GetKey(idxEnd + 1);
	
	if (pPrev == NULL) pPrev = pA;
	if (pPost == NULL) pPost = pB;
	

	for (i=0;i<nChannels;i++)
	{
		dest[i] = CubicInterpolate(pPrev->values[i], pA->values[i], pB->values[i], pPost->values[i], t);
	}
}

/*
 Tension: 1 is high, 0 normal, -1 is low
 Bias: 0 is even,
 positive is towards first segment,
 negative towards the other
 */
static double HermiteInterpolate(double y0,double y1, double y2,double y3, double mu, double tension, double bias)
{
	double m0,m1,mu2,mu3;
	double a0,a1,a2,a3;
	
	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0  = (y1-y0)*(1+bias)*(1-tension)/2;
	m0 += (y2-y1)*(1-bias)*(1-tension)/2;
	m1  = (y2-y1)*(1+bias)*(1-tension)/2;
	m1 += (y3-y2)*(1-bias)*(1-tension)/2;
	a0 =  2*mu3 - 3*mu2 + 1;
	a1 =    mu3 - 2*mu2 + mu;
	a2 =    mu3 -   mu2;
	a3 = -2*mu3 + 3*mu2;
	
	return(a0*y1+a1*m0+a2*m1+a3*y2);
}

Hermite::Hermite(int nChannels) : Curve(nChannels,kCurveClass_Hermite)
{
}
void Hermite::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pPrev, *pA, *pB, *pPost;
	pA = GetKey(idxStart);
	pB = GetKey(idxEnd);

	pPrev = GetKey(idxStart -1);
	pPost = GetKey(idxEnd + 1);
	
	if (pPrev == NULL) pPrev = pA;
	if (pPost == NULL) pPost = pB;
	
	
	for (i=0;i<nChannels;i++)
	{
		dest[i] = HermiteInterpolate(pPrev->values[i], pA->values[i], pB->values[i], pPost->values[i], t, pA->tension,pA->bias);
	}	
}

//
KBSpline::KBSpline(int nChannels) : Curve(nChannels, kCurveClass_KBSpline)
{
	ta.reserve(nChannels);
	tb.reserve(nChannels);
}

// TODO: Verify output with GOA
void KBSpline::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{
	int i;
	Key *pPrev, *pA, *pB, *pPost;
	pA = GetKey(idxStart);
	pB = GetKey(idxEnd);

	pPrev = GetKey(idxStart -1);
	pPost = GetKey(idxEnd + 1);

	if (pPrev == NULL)
	{
		// compute start tangent here
		pPrev = pA;
	}
	if (pPost == NULL)
	{
		// compute end tangent here
		pPrev = pB;
	}
	
	vector<double> &ch1 = keys[idxStart]->values;
	vector<double> &ch2 = keys[idxStart+1]->values;
	// ch4 = dest

	double v,tt,ttt, h1,h2,h3,h4;
	//double ta, tb;	// arrays
	double t1, b1, c1, t2, b2, c2;
	double d1,d2,k1,k2;
	double val,adjust;
	v = t;
	// - implementation refactored from GOA

	// Calculate the Hermite coefficients.
	t = v;
	tt = v*v;
	ttt = tt*v;

	h1 = 2*ttt - 3*tt + 1;
	h2 = ttt - 2*tt + t;
	h3 = -2*ttt + 3*tt;
	h4 = ttt - tt;

	// Initialize some variables.
	//ch1 = keys[u  ].values;
	//ch2 = keys[u+1].values;
	//ch4 = k->channels;	// dest

	t1 = keys[idxStart]->tension;
	b1 = keys[idxStart]->bias;
	c1 = keys[idxStart]->continuity;

	t2 = keys[idxStart+1]->tension;
	b2 = keys[idxStart+1]->bias;
	c2 = keys[idxStart+1]->continuity;

	if(keys.size() == 2)
		for(i=0 ; i<nChannels; i++)
		{
			val = ch2[i] - ch1[i];
			tb[i] = val*(1 - t1);
			ta[i] = val*(1 - t2);
		}
	else
	{
		if(idxStart != 0)
		{
			vector<double> &ch0 = keys[idxStart-1]->values;

			// Calculate the tangent adjustment value.
			d1 = keys[idxStart]->t - keys[idxStart - 1]->t; //keys[u  ].t - keys[u-1].t;
			d2 = keys[idxStart+1]->t - keys[idxStart]->t; //keys[u+1].t - keys[u  ].t;
			adjust = 0.5f + (1.0f - (float) fabs(c1))*(d2/(d1+d2) - 0.5f);

			// Calculate the tangent parameter coefficients.
			k1 = (1 + b1)*(1 + c1)*(1 - t1)*adjust;
			k2 = (1 - b1)*(1 - c1)*(1 - t1)*adjust;	// TODO: Verify this line

			// Calculate the tangent.
			//tb[i] = (ch1[i]-ch0[i])*k1 + (ch2[i]-ch1[i])*k2;
			for(i=0 ; i<nChannels ; i++)
				tb[i] = (ch1[i]-ch0[i])*k1 + (ch2[i]-ch1[i])*k2;
		}

		if(idxStart != keys.size()-2)
		{
			vector<double> &ch3 = keys[idxStart+2]->values;
//			ch3 = keys[u+2].channels;

			// Cal.front()culate the tangent adjustment value.
			d1 = keys[idxStart+1]->t - keys[idxStart]->t; // keys[u+1]->t - keys[u  ]->t;
			d2 = keys[idxStart+2]->t - keys[idxStart+1]->t; ///keys[u+2]->t - keys[u+1]->t;
			adjust = 0.5f + (1.0f - (float) fabs(c2))*(d1/(d1+d2) - 0.5f);

			// Calculate the tangent parameter coefficients.
			k1 = (1 + b2)*(1 - c2)*(1 - t2)*adjust;
			k2 = (1 - b2)*(1 + c2)*(1 - t2)*adjust;	// TODO: Verify this

			// Calculate the tangent.
			for(i=0 ; i<nChannels; i++)
				ta[i] = (ch2[i]-ch1[i])*k1 + (ch3[i]-ch2[i])*k2;
		}

		if(idxStart == 0)
			for(i=0 ; i<nChannels; i++)
				tb[i] = ((ch2[i] - ch1[i])*1.5f - ta[i]*0.5f)*(1-t1);

		if(idxStart == keys.size()-2)
			for(i=0 ; i<nChannels; i++)
				ta[i] = ((ch2[i] - ch1[i])*1.5f - tb[i]*0.5f)*(1-t2);
	}

	// Calculate the spline point.
	for(i=0 ; i<nChannels; i++)
		dest[i] = ch1[i]*h1 + tb[i]*h2 + ch2[i]*h3 + ta[i]*h4;

}

KBQuatSpline::KBQuatSpline(int nChannels) : Curve(nChannels, kCurveClass_KBQuatSpline)
{
	assert(nChannels==4);
	ta.reserve(nChannels);
	tb.reserve(nChannels);
	curveType = kCurveType_Quat;	// quat's
}

void KBQuatSpline::Interpolate(double t, int idxStart, int idxEnd, double *dest)
{

	double g1[4],g2[4],g3[4],g4[4],g5[4];	// quats
	double t1, b1, c1, t2, b2, c2;
	double d1,d2,adjust;

	double *q1 = &keys[idxStart]->values.front();
	double *q2 = &keys[idxStart+1]->values.front();

	t1 = keys[idxStart]->tension;
	b1 = keys[idxStart]->bias;
	c1 = keys[idxStart]->continuity;

	t2 = keys[idxStart+1]->tension;
	b2 = keys[idxStart+1]->bias;
	c2 = keys[idxStart+1]->continuity;

	if(keys.size() == 2)
	{
		if(curveType == kCurveType_Quat_3DS)
		{
			qSlerp(&tb.front(),q1,q2,(1-t1)*(1+c1*b1)/3.0f);
			qSlerp(&ta.front(),q2,q1,(1-t2)*(1-c2*b2)/3.0f);
		}
		else
		{
			qSlerp(&tb.front(),q1,q2,(1-t1)/3.0f);
			qSlerp(&ta.front(),q2,q1,(1-t2)/3.0f);
		}
	}
	else
	{

		if(idxStart != 0)
		{
			// Calculate outgoing tangent b.
			double *q0 = &keys[idxStart-1]->values.front();
			d1 = keys[idxStart  ]->t - keys[idxStart-1]->t;
			d2 = keys[idxStart+1]->t - keys[idxStart  ]->t;
			adjust = 0.5f + (1 - (float) fabs(c1))*(d2/(d1+d2) - 0.5f);
			qSlerp(g1, q1, q0, -(1+b1)/3.0f);
			qSlerp(g2, q1, q2,  (1-b1)/3.0f);
			qSlerp(g3, g1, g2, 0.5f-0.5f*c1);
			qSlerp(&tb.front(), q1, g3, -(t1-1)*adjust*2);
		}

		if(idxStart != keys.size()-2)
		{
			// Calculate incoming tangent a.
			double *q3 = &keys[idxStart+2]->values.front();
			d1 = keys[idxStart+1]->t - keys[idxStart  ]->t;
			d2 = keys[idxStart+2]->t - keys[idxStart+1]->t;
			adjust = 0.5f + (1 - (float) fabs(c2))*(d1/(d1+d2) - 0.5f);
			qSlerp(g1, q2, q1, -(1+b2)/3.0f);
			qSlerp(g2, q2, q3,  (1-b2)/3.0f);
			qSlerp(g3, g1, g2, 0.5f+0.5f*c2);
			qSlerp(&ta.front(), q2, g3, (t2-1)*adjust*2);
		}
		if(curveType == kCurveType_Quat_3DS)
		{
			if(idxStart == 0)
				qSlerp(&tb.front(),q1,q2,(1-t1)*(1+c1*b1)/3.0f);

			if(idxStart == keys.size()-2)
				qSlerp(&ta.front(),q2,q1,(1-t2)*(1-c2*b2)/3.0f);
		}
		else
		{
			if(idxStart == 0)
				qSlerp(&tb.front(),q1,&ta.front(),(1-t1)*0.5f);

			if(idxStart == keys.size()-2)
				qSlerp(&ta.front(),q2,&tb.front(),(1-t2)*0.5f);
		}
	}

	// Interpolate the bezier patch on the unit sphere.
	qSlerp(g1,q1,&tb.front(),t);
	qSlerp(g2,&tb.front(),&ta.front(),t);
	qSlerp(g3,&ta.front(),q2,t);
	qSlerp(g4,g1,g2,t);
	qSlerp(g5,g2,g3,t);
	qSlerp(g1,g4,g5,t);

	// Duplicate to destintation (this way I kept the logic of the refactored code)
	qDup(dest, g1);
/*
	k->channels[0] = g1.w;
	k->channels[1] = g1.x;
	k->channels[2] = g1.y;
	k->channels[3] = g1.z;
*/
}

//
// - Quat functionality mainly for lerping quats within KBQuatSplines
//
static double *qDup(double *dst, double *src)
{
	memcpy (dst,src,sizeof (double) * 4);
	return dst;
}
static double qDot (double *q1, double *q2)
{
	return q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];

}
static double* qAdd(double *q, double *q1, double *q2)
{
	int i;
	for (i=0;i<4;i++)
		q[i] = q1[i]+q2[i];

	return q;
}
static double *qScale (double *qr, double *qa, double scale)
{
	int i;
	for (i=0;i<4;i++)
		qr[i]=qa[i] * scale;

	return qr;
}
static double* qSlerp(double *q, double *q1, double *q2, double t)
{
	double q3[4],q4[4];
	double s1,s2,sa,a,d;

	d = qDot(q1,q2);
	if(d < -1.0f)
		d = -1.0f;
	else
		if(d > 1.0f) d = 1.0f;

	a = acos(d);
	sa = sin(a);
	if(sa < 0.0001f)
	{
		int i;
		for (i=0;i<4;i++) q[i] = q1[i];
	}	
	else
	{
		s1 = sin((1.0f-t)*a)/sa;
		s2 = sin(t*a)/sa;
		qScale(q3,q1,s1);
		qScale(q4,q2,s2);	// bug here, fixed by Gnilk 021129
		qAdd(q,q3,q4);
	}

	return q;
}
