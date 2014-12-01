#pragma once

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_set>


#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include "PluginObjectImpl.h"
#include "glShaderBase.h"

#include "Tokenizer.hpp"

using namespace yapt;
using namespace gnilk;



namespace WaveFront {


typedef enum {
	kLineToken_Unknown = 0,
	kLineToken_Comment,
	kLineToken_Object,
	kLineToken_Vertex,
	kLineToken_VertexNormal,
	kLineToken_VertexTexture,
	kLineToken_Face,
	kLineToken_MaterialLib,
	kLineToken_UseMaterial,
} kLineToken;

typedef enum {
	kMaterialToken_Unknown = 0,
	kMaterialToken_Comment,
	kMaterialToken_Material,
	kMaterialToken_SpecularCoeff,
	kMaterialToken_AmbientColor,
	kMaterialToken_DiffuseColor,
	kMaterialToken_SpecularColor,
	kMaterialToken_DissolveFactor,
	kMaterialToken_Shading,
} kMaterialToken;

typedef enum {
	kFaceType_Triangle,
	kFaceType_Quad,
} kFaceType;


class Material {
private:
	std::string name;
	double specularCoeff;
	float ambientColor[3];
	float diffuseColor[3];
	float specularColor[3];
	int shading;

public:
	Material(std::string name);
	virtual ~Material();

	__inline void SetSpecularCoeff(double ns) { specularCoeff = ns; }
	__inline void SetAmbientColor(float *col) { memcpy(ambientColor, col, sizeof(float)*3); }
	__inline void SetDiffuseColor(float *col) { memcpy(diffuseColor, col, sizeof(float)*3); }
	__inline void SetSpecularColor(float *col) { memcpy(specularColor, col, sizeof(float)*3); }
	__inline void SetShading(int s) { shading = s; }
	__inline double SpecularCoeff() { return specularCoeff; }	
	__inline float *AmbientColor() { return ambientColor; }
	__inline float *DiffuseColor() { return diffuseColor; }
	__inline float *SpecularColor() { return specularColor; }
	__inline int Shading() { return shading; }
	__inline std::string Name() { return name; }
};


class Face {
private:
	kFaceType faceType;
	float faceNormal[3];
	std::vector<int> vertexIndex;
	std::vector<int> vertexNormalIndex;
	std::vector<int> uvIndex;
public:
	Face() { faceType = kFaceType_Triangle; }
	virtual ~Face() {};

	__inline void AddVertex(int idx) { vertexIndex.push_back(idx); }
	__inline void AddVertexNormal(int idx) { vertexNormalIndex.push_back(idx); };
	__inline void AddVertexUV(int idx) { uvIndex.push_back(idx); };

	__inline int *VertexIndexBuffer() { return &vertexIndex[0]; }
	__inline int *VertexNormalIndexBuffer() { return &vertexNormalIndex[0]; }
	__inline int *VertexUVIndexBuffer() { return &uvIndex[0]; }

	__inline float *FaceNormal() { return faceNormal; }
	void CalculateFaceNormal(float *vertexBuffer);

	kFaceType FaceType() { return faceType; }
	void SetFaceType(kFaceType type) { faceType = type; }
};

class RenderFaceGroup {
private:
	Material *material;
	std::vector<Face *> faces;
public:
	RenderFaceGroup(Material *mtrl) { material = mtrl; }
	__inline void AddFace(Face *face) { faces.push_back(face); }
	__inline int NumFaces() { return faces.size(); }
	__inline Face **Faces() { return &faces[0]; }
	__inline Material *Material() { return material; }
};


class Object {
private:
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> uvcoords;
	std::vector<Face *> faces;

	std::vector<RenderFaceGroup *> renderGroups;

	std::string name;

	float boundingRadius;
	float midPoint[3];

public:
	Object(std::string name);
	virtual ~Object();

	void AddVertex(float *vertex);
	void AddVertexNormal(float *normal);
	void AddVertexUV(float *uvcoord);
	void AddFace(Face *face);
	void AddRenderGroup(RenderFaceGroup *renderGroup);

	__inline int NumVertices() { return vertices.size()/3; }
	__inline int NumNormals() { return normals.size()/3; }
	__inline int NumUVCoords() { return uvcoords.size()/2; }
	__inline int NumFaces() { return faces.size(); }
	__inline int NumRenderGroups() { return renderGroups.size(); }

	__inline float *VertexBuffer() { return &vertices[0]; }
	__inline float *NormalBuffer() { return &normals[0]; }
	__inline float *UVBuffer() { return &uvcoords[0]; }
	__inline Face **Faces() { return &faces[0]; }
	__inline RenderFaceGroup **RenderGroups() { return &renderGroups[0]; }

	__inline std::string Name() { return name; }

	float BoundingRadius();
	__inline float *MidPoint() { if (boundingRadius < 0) BoundingRadius(); return midPoint; }
};

class Loader {
private:
	Object *currentObject;
	Material *currentMaterial;
	RenderFaceGroup *currentRenderGroup;
	std::vector<Object *>objects;
	std::vector<Material *>materials;
public:
	Loader();
	virtual ~Loader();
	void LoadFile(std::string filename);

	__inline int NumObjects() { return objects.size(); }
	__inline Object **Objects() { return &objects[0]; }

private:
	void ParseLine(std::string line);
	kLineToken ClassifyLineToken(std::string &tok);
	void ParseObject(Tokenizer &tokenizer);
	void ParseVertex(Tokenizer &tokenizer);
	void ParseVertexNormal(Tokenizer &tokenizer);
	void ParseVertexTexture(Tokenizer &tokenizer);
	void ParseFace(Tokenizer &tokenizer);
	void LoadMaterialLib(Tokenizer &tokenizer);
	void UseMaterial(Tokenizer &tokenizer);
	Face *DoParseFace(Tokenizer &tokenizer);
	float *ParseFloatArray(Tokenizer &tokenizer, float *dst, int maxItems);

	Material *MaterialFromName(std::string name);
	Material *GetCurrentMaterial() { return currentMaterial; }
	RenderFaceGroup *GetCurrentRenderGroup();

	Object *GetCurrentObject();
	void SetCurrentObject(Object *object);
};

class MaterialLoader {
private:
	Material *currentMaterial;
	std::vector<Material *>materials;
public:
	MaterialLoader();
	virtual ~MaterialLoader();

	void LoadFile(std::string filename);

	__inline int NumMaterials() { return materials.size(); }
	__inline Material **Materials() { return &materials[0]; }
private:
	void ParseLine(std::string line);
	kMaterialToken ClassifyLineToken(std::string &tok);

	void ParseNewMaterial(Tokenizer &tokenizer);
	void ParseSpecularCoeff(Tokenizer &tokenizer);
	void ParseAmbientColor(Tokenizer &tokenizer);
	void ParseDiffuseColor(Tokenizer &tokenizer);
	void ParseSpecularColor(Tokenizer &tokenizer);
	void ParseShading(Tokenizer &tokenizer);

	bool ParseColor(float *col, Tokenizer &tokenizer);

	Material *GetCurrentMaterial();
	void SetCurrentMaterial(Material *material);
};


} // wavefront namespace



class OpenGLWaveFront: public PluginObjectImpl, public OpenGLShaderBase {
private:
	// input
	Property *waveFrontFile;
	Property *vshader;
	Property *fshader;
	bool useShaders;
	WaveFront::Loader *loader;

public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	void RenderWaveFrontObject(WaveFront::Object *object);
	void SetMaterial(WaveFront::Material *material);
	//void RenderWaveFrontFace(WaveFront::Face *face)
};


