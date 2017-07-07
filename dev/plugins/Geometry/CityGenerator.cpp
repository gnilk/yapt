#include "CityGenerator.h"
#include "vec.h"
#include "yapt/ySystem.h"

#include <random>
#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

static ILogger *logger;

void CityGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  pos_range = pInstance->CreateProperty("range", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  max_height = pInstance->CreateProperty("height", kPropertyType_Float, "10.0", "");
  num_objects = pInstance->CreateProperty("count", kPropertyType_Integer, "10", "");

  out_vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  out_vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");
  out_vertexUVData = pInstance->CreateOutputProperty("vertexUVData", kPropertyType_UserPtr, NULL, "");

  out_quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  out_quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  out_lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  out_lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  


  out_roadVertexCount = pInstance->CreateOutputProperty("road_vertexCount", kPropertyType_Integer, "0", "");
  out_roadVertexData = pInstance->CreateOutputProperty("road_vertexData", kPropertyType_UserPtr, NULL, "");

  out_roadQuadCount = pInstance->CreateOutputProperty("road_quadCount", kPropertyType_Integer, "0", "");
  out_roadQuadData = pInstance->CreateOutputProperty("road_quadData", kPropertyType_UserPtr, NULL, "");  

  out_roadLineCount = pInstance->CreateOutputProperty("road_lineCount", kPropertyType_Integer, "0", "");
  out_roadLineData = pInstance->CreateOutputProperty("road_lineData", kPropertyType_UserPtr, NULL, "");  

}

#define MAX_ROAD_VERTEX 2048
#define MAX_ROAD_LINES 512
#define MAX_ROAD_QUADS 512

#define NUM_OBJECTS 5
void CityGenerator::Render(double t, IPluginObjectInstance *pInstance) {
	logger = pInstance->GetLogger();
	logger->Debug("Generate Buildings");
	GenerateBuildings();
	logger->Debug("Generate Roads");
	GenerateRoads();
}
static std::default_random_engine generator;
static std::uniform_int_distribution<> gendir(1,16);
static std::uniform_real_distribution<float> gendist(1,5);

void CityGenerator::GenerateRoads() {
	float p[3] = {0,-1,0};

	numRoadLines = numRoadVertex = 0;
	roadvertex = (float *)malloc(sizeof(float) * 3 * MAX_ROAD_VERTEX);
	roadlines = (int *)malloc(sizeof(int) * 2 * MAX_ROAD_VERTEX);
	roadquads = (int *)malloc(sizeof(int) * 4 * MAX_ROAD_QUADS);

	int dir = gendir(generator);
	float dist = gendist(generator);

	AddRoadVertex(p);
	Road(0, p, 0, dir, dist, 0);

	logger->Debug("Road Vertex: %d", numRoadVertex);
	logger->Debug("Road Lines : %d", numRoadLines);
	logger->Debug("Road Quads : %d", numRoadQuads);

	out_roadVertexCount->v->int_val = numRoadVertex;
  	out_roadVertexData->v->userdata = (void *)roadvertex;

	out_roadQuadCount->v->int_val = numRoadQuads;
	out_roadQuadData->v->userdata = (void *)roadquads;

	out_roadLineCount->v->int_val = numRoadLines;
	out_roadLineData->v->userdata = (void *)roadlines;

}

int CityGenerator::MakeTurn(float *pNext, float *pOrigin, int from, int dir, float dist) {
	vDup(pNext, pOrigin);
	if ((dir & 1) && !(from & 2)) {
		// left
		pNext[0] += dist;
		return 0;
	}
	if ((dir & 2) && !(from & 1)) {
		// right
		pNext[0] -= dist;
		return 1;
	}
	if ((dir & 4) && !(from & 8)) {
		// forward
		pNext[2] += dist;
		return 2;
	}
	if ((dir & 8) && !(from & 4)) {
		// left
		pNext[2] -= dist;
		return 3;
	}
	return -1;
}

static std::string turndir[] = {
	"left", "right", "forward", "backward"
};

bool CityGenerator::CheckRange(float *v) {
	if (v[0] < -pos_range->v->vector[0]) return false;
	if (v[0] > pos_range->v->vector[0]) return false;

	if (v[2] < -pos_range->v->vector[2]) return false;
	if (v[2] > pos_range->v->vector[2]) return false;

	return true;
}

void CityGenerator::Road(int idxCurrent, float *pOrigin, int from, int direction, float dist, int depth) {
	float pNext[3];
	if (depth > 16) {
		return;
	}
	float dorg = dist;
	dist = floor(dist)*4;

	for(int i = 0;i<4;i++) {

		int turn = (direction & (1<<i));
		//if (turn == from) continue;

		int idxTurn = MakeTurn(pNext, pOrigin, from, turn, dist);
		if (!CheckRange(pNext)) continue;	// don't allow roads to expand outside the city core..

		if (idxTurn >= 0) {
	//printf("%f (%f)\n",dist, dorg);

			// printf("%d:%d - dir: %d, turn: %d - %s, dist: %f\n",depth, i, direction, 1<<i, turndir[idxTurn].c_str(), dist);
			int idxNext = AddRoadSegment(idxCurrent, pNext);
			if (idxNext < 0) {
				logger->Debug("Road segment buffer full, QUIT!");
				return;
			}
			
			int nextdir = gendir(generator);
			float nextdist = gendist(generator);
			//printf("ND: %f\n", nextdist);
			Road(idxNext, pNext, turn, nextdir, nextdist, depth+1);
		}
	}
}

int CityGenerator::AddRoadSegment(int idxCurrent, float *pNext) {
	int idxVertex = AddRoadVertex(pNext);
	if (idxVertex > 0) {
		//printf("AddLine: %d, %d - %d\n", numRoadLines, idxCurrent, idxVertex);
		int res = AddRoadLine(idxCurrent, idxVertex);
		if (res < 0) {
			logger->Debug("ROAD LINE BUFFER FULL!!");
			return -1;
		}
	} else {
		logger->Debug("ROAD VERTEX BUFFER FULL!!!");
	}

	if (AddRoadQuad(idxCurrent, idxVertex) < 0) {
		logger->Debug("ROAD QUAD BUFFER FULL!!!");
	}

	return idxVertex;
}

int CityGenerator::AddRoadQuad(int idxFrom, int idxTo) {

	float *pVFrom = &roadvertex[idxFrom * 3];
	float *pVTo = &roadvertex[idxTo *3];
	float dir[3], expand[3];
	bool swap = false;

	if ((numRoadQuads-1) > MAX_ROAD_QUADS) return -1;
	//printf("Add Road Quad\n");



	vNorm(dir, vSub(dir, pVTo, pVFrom));
	if (fabs(dir[0])>fabs(dir[2])) {
		swap = true;
	}

	float p1[3];
	float p2[3];
	float p3[3];
	float p4[3];

	float start[3];
	float end[3];

	vSub(start, pVFrom, dir);
	vAdd(end, pVTo, dir);

	vDup(p1, start);
	vDup(p2, start);
	vDup(p3, end);
	vDup(p4, end);


	float tmp;
	tmp = dir[0];
	dir[0] = dir[2];
	dir[2] = tmp;

	vAdd(p1,p1,dir);
	vSub(p2,p2,dir);
	vAdd(p3,p3,dir);
	vSub(p4,p4,dir);


	int i1, i2, i3, i4;
	i1 = AddRoadVertex(p1);
	i2 = AddRoadVertex(p2);
	i3 = AddRoadVertex(p3);
	i4 = AddRoadVertex(p4);

	if ((i1 < 0) || (i2 < 0) || (i3 < 0) || (i4 < 0)) {
		logger->Debug("Vertex buffer full");
		return -1;
	}

	if (!swap) {
		roadquads[numRoadQuads*4+0]=i1;
		roadquads[numRoadQuads*4+1]=i3;
		roadquads[numRoadQuads*4+2]=i4;
		roadquads[numRoadQuads*4+3]=i2;		
	} else {
		roadquads[numRoadQuads*4+0]=i2;
		roadquads[numRoadQuads*4+1]=i4;
		roadquads[numRoadQuads*4+2]=i3;
		roadquads[numRoadQuads*4+3]=i1;
	}
	numRoadQuads++;
	if ((numRoadQuads-1) > MAX_ROAD_QUADS) return -1;
	return numRoadQuads;
}

int CityGenerator::AddRoadVertex(float *pVertex) {
	if (numRoadVertex > (MAX_ROAD_VERTEX-1)) return -1;
	int retVertex = numRoadVertex;
	//printf("AddVertex: %f, %f, %f\n", pVertex[0], pVertex[1], pVertex[2]);
	memcpy(&roadvertex[numRoadVertex*3], pVertex, sizeof(float)*3);
	numRoadVertex++;
	return retVertex;
}
int CityGenerator::AddRoadLine(int idx1, int idx2) {
	if (numRoadLines*2 > (MAX_ROAD_LINES-1)) return -1;
	roadlines[numRoadLines*2+0] = idx1;
	roadlines[numRoadLines*2+1] = idx2;
	numRoadLines++;
	return numRoadLines;
}

void CityGenerator::GenerateBuildings() {
	int numObjects = num_objects->v->int_val;
	std::vector<CityObject> objects(numObjects);

	std::default_random_engine generator;
	std::uniform_real_distribution<float> x_distribution(-pos_range->v->vector[0],pos_range->v->vector[0]);
	std::uniform_real_distribution<float> y_distribution(-pos_range->v->vector[1],pos_range->v->vector[1]);
	std::uniform_real_distribution<float> z_distribution(-pos_range->v->vector[2],pos_range->v->vector[2]);
	std::uniform_real_distribution<float> h_distribution(1,max_height->v->float_val);

	int numCreated = 0;

	int nVertex = 0;
	int nLines = 0;
	int nQuads = 0;
	for(int i=0;i<numObjects;i++) {
		float xp = Bin(x_distribution(generator)) + 2;
		float yp = Bin(y_distribution(generator));
		float zp = Bin(z_distribution(generator)) + 2;

		if (ShouldCreate(xp,yp,zp)) 
		{
			float vpos[3] = {xp,yp,zp};
			float h = h_distribution(generator);
			float maxlen = vAbs(pos_range->v->vector);
			//vSub(vpos, pos_range->v->vector, vpos);

			float len = maxlen / (2 * vAbs(vpos));
			if (len > 3) len = 3.0;
			h = h * len;

			objects[numCreated].SetPos(xp,yp,zp);
			objects[numCreated].SetHeight(h);
			objects[numCreated].Prepare();

			nVertex += objects[numCreated].GetVertexCount();
			nLines  += objects[numCreated].GetLineCount();
			nQuads  += objects[numCreated].GetQuadCount();

			numCreated++;
		}
	}

	logger->Debug("Objects: %d (of %d requested)", numCreated, numObjects);
	logger->Debug("Vertex: %d", nVertex);
	logger->Debug("Lines : %d", nLines);
	logger->Debug("Quads : %d", nQuads);

	float *pVertex = (float *)malloc(sizeof(float) * 3 * nVertex);
	float *pUVVertex = (float *)malloc(sizeof(float) * 3 * nVertex);
	int *pLines = (int *)malloc(sizeof(int) * 2 * nLines);
	int *pQuads = (int *)malloc(sizeof(int) * 4 * nQuads);

	int vIdx = 0;
	int lIdx = 0;
	int qIdx = 0;
	for(int i=0;i<numCreated;i++) {
		objects[i].Generate(&pVertex[vIdx*3], &pUVVertex[vIdx*3], &pLines[lIdx*2], &pQuads[qIdx*4], vIdx);

		vIdx += objects[i].GetVertexCount();
		lIdx += objects[i].GetLineCount();
		qIdx += objects[i].GetQuadCount();
	}

  out_vertexCount->v->int_val = nVertex;
  out_vertexData->v->userdata = (void *)pVertex;
  out_vertexUVData->v->userdata = (void *)pUVVertex;

  out_quadCount->v->int_val = nQuads;
  out_quadData->v->userdata = (void *)pQuads;

  out_lineCount->v->int_val = nLines;
  out_lineData->v->userdata = (void *)pLines;

}

float CityGenerator::Bin(float v) {
	v = (((int)v)>>2) << 2;
	return v;
}
float CityGenerator::BinSpec(float v, int bs) {
	//v = floor(v);
	v = (((int)v)>>bs) << bs;
	return v;
}
bool CityGenerator::ShouldCreate(float x, float y, float z) {
	int hash = x+y*1024+z*1024*1024;
	auto it = objectmap.find(hash);
	if (it == objectmap.end()) {
		objectmap.insert(std::make_pair(hash, true));
		return true;
	}
	return false;
}

void CityGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  
}

void CityGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}


CityObject::CityObject() {
	vIni(position,0,0,0);
	height = 1;
}

void CityObject::SetPos(float x, float y, float z) {

	vIni(position, x, y, z);
}

void CityObject::SetHeight(float h) {
	height = h;
	if (height < 1) {
		height = 1.0f;		
	}
}

// should be set before generate has been called
void CityObject::Prepare() {
	vcount = 8;
	qcount = 6;
	lcount = 12;
}
void CityObject::Generate(float *vtx_dest, float *uv_dest, int *line_dest, int *quad_dest, int vIndexOffset) {
  static float cubeVertex[] = {
     1,  1, -1 , 
     1, -1, -1,
    -1, -1, -1,
    -1,  1, -1,
     1,  1,  1,
     1, -1,  1,
    -1, -1,  1,
    -1,  1,  1,
  };

  static float cubeUV[] = {
     0.25,  1, 0 , 
     0.25,  0, 0,
     0,  0, 0,
     0,  1, 0,
     1,  1, 0,
     1,  0, 0,
     0.5,  0, 0,
     0.5,  1, 0,
  };

  static int cubeQuadIndex[] = 
  { 3,2,1,0, 
    4,5,6,7, 
    1,5,4,0,
    7,6,2,3, 
    3,0,4,7, 
    2,6,5,1, 
  };

  static int cubeQuadMtrlp[] = {
  	0,0,1,1,1,1                	// top/bottom has material = 0, sides = 1
  };

  static int cubeLinesIndex[] = 
  { 0,1,
    1,2,
    2,3,
    3,0,
    4,5,
    5,6,
    6,7,
    7,4,
    0,4,
    1,5,
    2,6,
    3,7,
  };

  // setup vertices
  for(int i=0;i<8;i++) {
  	float v[3];
  	vDup(v, &cubeVertex[i*3]);
  	if (v[1] > 0) {
  		float fac = 1.0f;

  		if (height > 10.0f) {
  			fac = ((i>3)?1.1:1.0);	// tilt roofs on tall buildings only..
  		}

 		v[1] *= height * fac;
  	}
  	vAdd(v, v, position);
 	vDup(&vtx_dest[i*3],v);				// store modified vertex

 	vDup(&uv_dest[i*3], &cubeUV[i*3]);	// store UV coords (needs to apply scaling on UV)
 	uv_dest[i*3+0]*=10;
 	uv_dest[i*3+1]*=10;
  }
  for(int i=0;i<lcount;i++) {
  	line_dest[i*2+0] = cubeLinesIndex[i*2+0] + vIndexOffset;
  	line_dest[i*2+1] = cubeLinesIndex[i*2+1] + vIndexOffset;
  }
  for(int i=0;i<qcount;i++) {
  	quad_dest[i*4+0] = cubeQuadIndex[i*4+0] + vIndexOffset;
  	quad_dest[i*4+1] = cubeQuadIndex[i*4+1] + vIndexOffset;
  	quad_dest[i*4+2] = cubeQuadIndex[i*4+2] + vIndexOffset;
  	quad_dest[i*4+3] = cubeQuadIndex[i*4+3] + vIndexOffset;
  }
}

int CityObject::GetLineCount() {
	return lcount;
}
int CityObject::GetVertexCount() {
	return vcount;
}
int CityObject::GetQuadCount() {
	return qcount;
}


