
#include "glShaderBase.h"
#include "glWavefront.h"
#include "StringUtil.h"
#include "Tokenizer.hpp"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "noice/io/io.h"
#include "noice/io/ioutils.h"
#include "noice/io/TextReader.h"

#include "vec.h"

#include <string>

#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;
using namespace gnilk;

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

static ISystem *pSys = NULL;

void OpenGLWaveFront::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	waveFrontFile = pInstance->CreateProperty("filename", kPropertyType_String, "object.obj", "");
	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
	loader = NULL;
	pSys = ySys;

}
void OpenGLWaveFront::Render(double t, IPluginObjectInstance *pInstance) {
	if (loader->NumObjects() == 0) return;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	  static GLfloat pos[4] = {5.f, 5.f, 10000.f, 0.f};
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_CULL_FACE);
	glPopMatrix();


	int n = loader->NumObjects();
	WaveFront::Object **objects = loader->Objects();

	for(int i=0;i<n;i++) {

		RenderWaveFrontObject(objects[i]);
	}
}

void OpenGLWaveFront::RenderWaveFrontObject(WaveFront::Object *object) {
	int nVert = object->NumVertices();
	int nFaces = object->NumFaces();
	float *vertexBuffer = object->VertexBuffer();
	WaveFront::Face **faces = object->Faces();


	//if (object->Name().compare("Cube.001_Cube.002") != 0) return;


//	glColor3f(1,1,1);

	int nRenderGroups = object->NumRenderGroups();
	WaveFront::RenderFaceGroup **renderGroups = object->RenderGroups();

	//printf("%s, nRg=%d\n",object->Name().c_str(), nRenderGroups);

	for (int rg=0;rg<nRenderGroups;rg++) {
		WaveFront::RenderFaceGroup *renderGroup = renderGroups[rg];

		if (renderGroup->Material() != NULL) {
			SetMaterial(renderGroup->Material());
		}

		nFaces = renderGroup->NumFaces();
		faces = renderGroup->Faces();
		//printf("  nFaces=%d\n", nFaces);

		for(int i=0;i<nFaces;i++) {
			WaveFront::Face *face = faces[i];
			int *vIndex = face->VertexIndexBuffer();
			switch(face->FaceType()) {
				case WaveFront::kFaceType_Quad :
					glBegin(GL_QUADS);
						glNormal3fv(face->FaceNormal());
						glVertex3fv(&vertexBuffer[vIndex[0]*3]);
						glVertex3fv(&vertexBuffer[vIndex[1]*3]);
						glVertex3fv(&vertexBuffer[vIndex[2]*3]);
						glVertex3fv(&vertexBuffer[vIndex[3]*3]);
					glEnd();
					break;
				case WaveFront::kFaceType_Triangle :
					glBegin(GL_TRIANGLES);
						glNormal3fv(face->FaceNormal());
						glVertex3fv(&vertexBuffer[vIndex[0]*3]);
						glVertex3fv(&vertexBuffer[vIndex[1]*3]);
						glVertex3fv(&vertexBuffer[vIndex[2]*3]);
					glEnd();
					break;
			}
		}
	}

	glDisable(GL_LIGHTING);

	// glColor3f(1,1,1);
	// glBegin(GL_POINTS);
	// for(int i=0;i<nVert;i++) {
	// 	glVertex3fv(&vertexBuffer[i*3]);
	// }

	//glEnd();
}
void OpenGLWaveFront::SetMaterial(WaveFront::Material *material) {
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->SpecularColor());
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->DiffuseColor());
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->AmbientColor());
	GLfloat _mat_shininess[] = { material->SpecularCoeff() };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, _mat_shininess);

}

void OpenGLWaveFront::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	pSys->GetLogger("gl.DrawWaveFront")->Debug("Post Initialize");
	loader = new WaveFront::Loader();
	loader->LoadFile(std::string(waveFrontFile->v->string));
}

void OpenGLWaveFront::PostRender(double t, IPluginObjectInstance *pInstance) {

}

//
// -- wavefront handling
//

using namespace WaveFront;




//
// -- loader
//
Loader::Loader() {
}

Loader::~Loader() {

}
static int nParsedVtx = 0;
void Loader::LoadFile(std::string filename) {
	pSys->GetLogger("WaveFrontLoader")->Error("Trying file '%s'", filename.c_str());

	this->currentObject = NULL;
	this->currentMaterial = NULL;
	this->currentRenderGroup = NULL;

	FILE *f = fopen(filename.c_str(), "rb");
	if (f != NULL) {
		while(!feof(f)) {
			char buffer[256];
			fgets(buffer,256,f);
			ParseLine(std::string(buffer));
		}
		pSys->GetLogger("WaveFrontLoader")->Debug("ok, file loaded, numObjects=%d",NumObjects());
		pSys->GetLogger("WaveFrontLoader")->Debug("Num Vertices: %d",this->objects.at(0)->NumVertices());
		pSys->GetLogger("WaveFrontLoader")->Debug("Parser Vertices: %d",nParsedVtx);
		pSys->GetLogger("WaveFrontLoader")->Debug("Num groups: %d", this->objects.at(0)->NumRenderGroups());
		fclose(f);
	} else {
		pSys->GetLogger("WaveFrontLoader")->Error("Failed to open file '%s'", filename.c_str());
	}
}

void Loader::ParseLine(std::string line) {
//	pSys->GetLogger("WaveFrontLoader")->Debug("Parse: '%s'",line.c_str());

	Tokenizer tokenizer(line, "/*()");
	std::string tok = tokenizer.Next();
	if (tok.empty()) return;

	kLineToken tokenType = ClassifyLineToken(tok);
	switch(tokenType) {
		case kLineToken_Comment :
			break;
		case kLineToken_Object : 
			ParseObject(tokenizer);
			break;
		case kLineToken_Vertex :
			ParseVertex(tokenizer);
			nParsedVtx++;
			break;
		case kLineToken_VertexNormal :
			ParseVertexNormal(tokenizer); 
			break;
		case kLineToken_VertexTexture : 
			ParseVertexTexture(tokenizer);
			break;
		case kLineToken_Face :
			ParseFace(tokenizer);
			break;
		case kLineToken_MaterialLib :
			//pSys->GetLogger("WaveFrontLoader")->Debug("Material library - skipping");
			LoadMaterialLib(tokenizer);
			break;
		case kLineToken_UseMaterial :
			UseMaterial(tokenizer);
			break;
		case kLineToken_Unknown :
			pSys->GetLogger("WaveFrontLoader")->Error("Unknown token '%s' on line '%s'", tok.c_str(), line.c_str());
			break;
	}
}

kLineToken Loader::ClassifyLineToken(std::string &tok) {
	static std::string tokens("# v f vn vt o mtllib usemtl");
	kLineToken result = kLineToken_Unknown;
	switch(Tokenizer::Case(tok, tokens)) {
		case 0 : // #
			result = kLineToken_Comment;
			break;
		case 1 : // v
			result = kLineToken_Vertex;
			break;
		case 2 : // f
			result = kLineToken_Face;
			break;
		case 3 : // vn
			result = kLineToken_VertexNormal;
			break;
		case 4 : // vt
			result = kLineToken_VertexTexture;
			break;
		case 5 : // o
			result = kLineToken_Object;
			break;
		case 6 :	// mtllib
			result = kLineToken_MaterialLib;
			break;
		case 7 : // usemtl
			result = kLineToken_UseMaterial;
			break;
		default :
			result = kLineToken_Unknown;
	}
	return result;
}
void Loader::ParseObject(Tokenizer &tokenizer) {
	std::string name = tokenizer.Next();
	Object *object = new Object(name);
	pSys->GetLogger("WaveFrontLoader")->Debug("New active object: '%s'", name.c_str());
	SetCurrentObject(object);
}

RenderFaceGroup *Loader::GetCurrentRenderGroup() {
	if (currentRenderGroup == NULL) {
		currentRenderGroup = new RenderFaceGroup(NULL);
		GetCurrentObject()->AddRenderGroup(currentRenderGroup);
	}
	return currentRenderGroup;
}

Object *Loader::GetCurrentObject(void) {
	if (currentObject == NULL) {
		pSys->GetLogger("WaveFrontLoader")->Debug("No object, creating default object with name 'noname'");
		Object *pObj = new Object(std::string("noname"));
		SetCurrentObject(pObj);
	} 

	return this->currentObject;
}

void Loader::SetCurrentObject(Object *object) {
	this->currentObject = object;
	this->objects.push_back(object);
}

Material *Loader::MaterialFromName(std::string name) {
	for(int i=0;i<materials.size();i++) {
		if (name.compare(materials[i]->Name()) == 0) {
			return materials[i];
		}
	}
	return NULL;
}

void Loader::ParseVertex(Tokenizer &tokenizer) {
	float vertex[3];
	ParseFloatArray(tokenizer, vertex, 3);
	GetCurrentObject()->AddVertex(vertex);
}

void Loader::ParseVertexNormal(Tokenizer &tokenizer) {
	float normal[3];
	ParseFloatArray(tokenizer, normal, 3);
	GetCurrentObject()->AddVertexNormal(normal);
}

void Loader::ParseVertexTexture(Tokenizer &tokenizer) {
	float uv[3];
	ParseFloatArray(tokenizer, uv, 3);
	GetCurrentObject()->AddVertexUV(uv);
}

void Loader::ParseFace(Tokenizer &tokenizer) {
	Face *face = DoParseFace(tokenizer);
	GetCurrentObject()->AddFace(face);
	GetCurrentRenderGroup()->AddFace(face);
}

void Loader::UseMaterial(Tokenizer &tokenizer) {
	std::string mtrlName = tokenizer.Next();
	currentMaterial = NULL;
	if (!mtrlName.empty()) {
		Material *mtrl = MaterialFromName(mtrlName);
		if (mtrl!=NULL) {
			currentMaterial = mtrl;
			currentRenderGroup = new RenderFaceGroup(currentMaterial);
			GetCurrentObject()->AddRenderGroup(currentRenderGroup);
		} 
	}
}

void Loader::LoadMaterialLib(Tokenizer &tokenizer) {
	std::string filename = tokenizer.Next();
	if (filename.empty()) return;
	MaterialLoader loader;
	loader.LoadFile(filename);

	// Copy to Object loader
	int n = loader.NumMaterials();
	Material **srcList = loader.Materials();
	for(int i=0;i<n;i++) {
		this->materials.push_back(srcList[i]);
	}
}

Face *Loader::DoParseFace(Tokenizer &tokenizer) {
	int idx=0;
	int idxComponent = 0;
	int dataFlags = 0;
	bool bExpectOp = false;
	std::string tmp;
	Face *face = new Face();


	//
	// face format is (index based):
	//  v/vt/vn
	// where v - vertex, vt - vertex texture, vn - vertex normal
	//
	// also possible is: v//vn (i.e. vertex and normal but no texture)
	//
	while(!(tmp = tokenizer.Next()).empty()) {
		if (tmp == "/") {
			bExpectOp = false;
			idxComponent++;	// next component for vertex (v, vt, vn)
		} else {
			if (bExpectOp == true) {
				// two numericals after each-other -> new face vertex!
				idxComponent=0;
				idx++;	// next index in face
			}
			int val = (int)strtol(tmp.c_str(), NULL,10);
			switch (idxComponent) {
				case 0:	// vertex
					face->AddVertex(val - 1);
					break;
				case 1 :	// texture
					face->AddVertexUV(val - 1);
					break;
				case 2 : // normal
					face->AddVertexNormal(val - 1);
				default:
					break;
			}
			bExpectOp = true;
			dataFlags |= 1 << (idxComponent);
		}
		if (idx >= 3) break;
	}
	// Zero based.. (0..3=quad, 0..2=tri)
	if (idx == 3) {
		face->SetFaceType(kFaceType_Quad);
	}

	return face;
	
}

float *Loader::ParseFloatArray(Tokenizer &tokenizer, float *dst, int maxItems) {
	int idx = 0;
	std::string tmp;

	while(!(tmp = tokenizer.Next()).empty()) {
		dst[idx] = strtod(tmp.c_str(), NULL);
		++idx;
		if (idx >= maxItems) break;
	}

	return dst;
}

//
// -- material loader
//
MaterialLoader::MaterialLoader() {

}

MaterialLoader::~MaterialLoader() {
	// dont' delete Material vector here
	// owner ship of materials taken over by object loader
}

void MaterialLoader::LoadFile(std::string filename) {
	pSys->GetLogger("WaveFrontMtrlLoader")->Error("Trying file '%s'", filename.c_str());

	this->currentMaterial = NULL;

	FILE *f = fopen(filename.c_str(), "rb");
	if (f != NULL) {
		while(!feof(f)) {
			char buffer[256];
			fgets(buffer,256,f);
			ParseLine(std::string(buffer));
		}
		pSys->GetLogger("WaveFrontMtrlLoader")->Debug("ok, file loaded, numMaterials=%d",NumMaterials());
		fclose(f);
	} else {
		pSys->GetLogger("WaveFrontMtrlLoader")->Error("Failed to open file '%s'", filename.c_str());
	}
}

void MaterialLoader::ParseLine(std::string line) {
	Tokenizer tokenizer(line, "/*()");
	std::string tok = tokenizer.Next();
	if (tok.empty()) return;

	kMaterialToken tokenType = ClassifyLineToken(tok);
	switch(tokenType) {
		case kMaterialToken_Material :
			ParseNewMaterial(tokenizer);
			break;
		case kMaterialToken_SpecularCoeff :
			ParseSpecularCoeff(tokenizer);
			break;
		case kMaterialToken_AmbientColor :
			ParseAmbientColor(tokenizer);
			break;
		case kMaterialToken_DiffuseColor :
			ParseDiffuseColor(tokenizer);
			break;
		case kMaterialToken_SpecularColor :
			ParseSpecularColor(tokenizer);
			break;
		case kMaterialToken_Shading :
			ParseShading(tokenizer);
			break;
		default :
			break;
	}
}

kMaterialToken MaterialLoader::ClassifyLineToken(std::string &tok) {
	static std::string tokens("# newmtl Ns Ka Kd Ks Ni d illum");
	static kMaterialToken vals[] = 
	{
		kMaterialToken_Comment,				// #
		kMaterialToken_Material,			// newmtl
		kMaterialToken_SpecularCoeff,		// Ns
		kMaterialToken_AmbientColor,		// Ka
		kMaterialToken_DiffuseColor,		// Kd
		kMaterialToken_SpecularColor,		// Ks
		kMaterialToken_Unknown,				// Ni
		kMaterialToken_DissolveFactor,		// d
		kMaterialToken_Shading,				// illum
	};

	// would be cool to return a function here instead
	int idx = Tokenizer::Case(tok, tokens);
	if (idx == -1) {
		pSys->GetLogger("WaveFrontMtrlLoader")->Error("Unknown token '%s'", tok.c_str());
		return kMaterialToken_Unknown;
	}
	return vals[idx];
}

void MaterialLoader::ParseNewMaterial(Tokenizer &tokenizer) {
	std::string name = tokenizer.Next();
	Material *object = new Material(name);
	pSys->GetLogger("WaveFrontMtrlLoader")->Debug("New active material: '%s'", name.c_str());
	SetCurrentMaterial(object);

}

void MaterialLoader::ParseSpecularCoeff(Tokenizer &tokenizer) {
	std::string tmp = tokenizer.Next();
	if (tmp.empty()) return;
	double ns = strtod(tmp.c_str(), NULL);
	GetCurrentMaterial()->SetSpecularCoeff(ns);
}

void MaterialLoader::ParseAmbientColor(Tokenizer &tokenizer) {
	float col[3];
	if (ParseColor(col, tokenizer)) {
		GetCurrentMaterial()->SetAmbientColor(col);		
	}
}

void MaterialLoader::ParseDiffuseColor(Tokenizer &tokenizer) {
	float col[3];
	if (ParseColor(col, tokenizer)) {
		GetCurrentMaterial()->SetDiffuseColor(col);		
	}
}

void MaterialLoader::ParseSpecularColor(Tokenizer &tokenizer) {
	float col[3];
	if (ParseColor(col, tokenizer)) {
		GetCurrentMaterial()->SetSpecularColor(col);		
	}
}

bool MaterialLoader::ParseColor(float *col, Tokenizer &tokenizer) {
	for(int i=0;i<3;i++) {
		std::string tmp = tokenizer.Next();
		if (tmp.empty()) {
			pSys->GetLogger("WaveFrontMtrlLoader")->Error("Ambient color num token mismatch - token empty");
			return false;
		}
		col[i] = strtod(tmp.c_str(), NULL);
	}
	return true;
}

void MaterialLoader::ParseShading(Tokenizer &tokenizer) {
	std::string tmp = tokenizer.Next();
	if (tmp.empty()) return;
	int shading = atoi(tmp.c_str());
	GetCurrentMaterial()->SetShading(shading);
}

Material *MaterialLoader::GetCurrentMaterial() {
	if (currentMaterial == NULL) {
		pSys->GetLogger("WaveFrontMtrlLoader")->Debug("No material, creating default material with name 'noname'");
		Material *pMtrl = new Material(std::string("noname"));
		SetCurrentMaterial(pMtrl);
	}	
	return this->currentMaterial;
}

void MaterialLoader::SetCurrentMaterial(Material *material) {
	this->currentMaterial = material;
	this->materials.push_back(material);
}


//
// -- material
//

Material::Material(std::string name) {
	this->name = name;
}
Material::~Material() {

}


//
// -- Object
//

Object::Object(std::string name) {
	this->name = name;
	this->boundingRadius = -1;
}

Object::~Object() {

}

void Object::AddVertex(float *vertex) {
	vertices.push_back(vertex[0]);
	vertices.push_back(vertex[1]);
	vertices.push_back(vertex[2]);
}
void Object::AddVertexNormal(float *normal) {
	normals.push_back(normals[0]);
	normals.push_back(normals[1]);
	normals.push_back(normals[2]);
}
void Object::AddVertexUV(float *uvcoord) {
	uvcoords.push_back(uvcoord[0]);
	uvcoords.push_back(uvcoord[1]);
	uvcoords.push_back(uvcoord[2]);
}
void Object::AddFace(Face *face) {
	face->CalculateFaceNormal(VertexBuffer());
	faces.push_back(face);
}

void Object::AddRenderGroup(RenderFaceGroup *renderGroup) {
	this->renderGroups.push_back(renderGroup);
}

float Object::BoundingRadius() {

	if (boundingRadius > 0) return boundingRadius;

	float *coords = VertexBuffer();
	float cmax[3],cmin[3];
	float tmp[3];
	vDup(cmax,&coords[0]);
	vDup(cmin,&coords[1*3]);
	// Find max/min vertex
	for(int i=0;i<NumVertices();i++) {
		cmax[0] = fmax(coords[i*3+0], cmax[0]);
		cmax[1] = fmax(coords[i*3+1], cmax[1]);
		cmax[2] = fmax(coords[i*3+2], cmax[2]);

		cmin[0] = fmin(coords[i*3+0], cmin[0]);
		cmin[1] = fmin(coords[i*3+1], cmin[1]);
		cmin[2] = fmin(coords[i*3+2], cmin[2]);
	}

	// Calculate radius and mid-point
	// Note: Why 2.5???
	double r = vAbs(vSub(tmp, cmax, cmin)) / 2.5;
	vAdd(tmp, cmax, cmin);
	vMul(midPoint, tmp, 0.5);

	this->boundingRadius = r;

	return boundingRadius;
}


//
// -- face
//

void Face::CalculateFaceNormal(float *vertexBuffer) {

	float *pV1 = &vertexBuffer[vertexIndex[0]*3];
	float *pV2 = &vertexBuffer[vertexIndex[1]*3];
	float *pV3 = &vertexBuffer[vertexIndex[2]*3];

	float vE1[3];
	float vE2[3];
	vSub(vE1, pV3, pV1);
	vSub(vE2, pV2, pV1);
	vCross(faceNormal, vE2, vE1);
	vNorm(faceNormal,faceNormal);	
}


