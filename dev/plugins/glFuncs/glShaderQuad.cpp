#include "glShaderQuad.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <math.h>

//#include <gl/glew.h>
#include <GL/GLEW.h>
//#include <OpenGl/glu.h>


using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLShaderQuad::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  width = ySys->GetConfigInt(kConfig_ResolutionWidth,1280);  
  height = ySys->GetConfigInt(kConfig_ResolutionWidth,720);  
  useblend = pInstance->CreateProperty("useblend", kPropertyType_Bool,"false","");

  texture = pInstance->CreateProperty("texture", kPropertyType_Integer, "0", "");  
  pixelshader_source = pInstance->CreateProperty("pixelshader", kPropertyType_String, "", "");
  vertexshader_source = pInstance->CreateProperty("vertexshader", kPropertyType_String, "", "");
  depthwrite = pInstance->CreateProperty("depthwrite", kPropertyType_Bool, "true", "");
  flipy = pInstance->CreateProperty("flipvertical", kPropertyType_Bool, "false", "");
  this->ySys = ySys;
}

void OpenGLShaderQuad::BeginOrtho() {

  glViewport(0,0,width,height); // Ensure,if someone did change it
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  if (!flipy->v->boolean) {
    gluOrtho2D(-1, 1, -1, 1);
  } else {
      gluOrtho2D(-1, 1, 1, -1);
  }
  //glScalef(1, -1, 1);
  //glTranslatef(0, -height, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void OpenGLShaderQuad::EndOrtho(){
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void OpenGLShaderQuad::SetParametersFromList(IPluginObjectInstance *pInstance) {
  for(int i=0;i<shaderParams.size();i++) {
    OpenGLShaderParameter *param = shaderParams[i];
    std::string name = param->GetName();

    int idParam = program->GetUniform(name.c_str());
    if (idParam == -1) {  // missing in shader
      if (!param->WarningIssued()) {
        pInstance->GetLogger()->Error("Parameter '%s' not found in shader!", name.c_str());
        param->SetWarningFlag();
        exit(1);
      }
      continue;
    }

    // printf("%d:%s\n",i,name.c_str());

    switch(param->GetType()) {
      case kParamType_Float :
        glUniform1f(idParam, param->GetFloat()); 
        break;
      case kParamType_Vec3 :
        {
          float tmp[3];
          param->GetVec3(tmp);
          glUniform3f(idParam, tmp[0], tmp[1], tmp[2]);
        }
        break;
      case kParamType_Texture :
        {
          glActiveTexture(GL_TEXTURE0+texture_count);        
          glBindTexture(GL_TEXTURE_2D,param->GetTexture());
          glUniform1i(idParam, texture_count); // texture_count);  
          int err = glGetError();
          if (err != GL_NO_ERROR) {
            pInstance->GetLogger()->Error("Unable to bind texture for param: %s (error=%d)",name.c_str(),err);
          }
          texture_count++;

        }
      default :
        // not supported just yet...
        break;
    }


  }
}

void OpenGLShaderQuad::Render(double t, IPluginObjectInstance *pInstance) {
  IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
  IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
  width = contextParams->GetFrameBufferWidth();
  height = contextParams->GetFrameBufferHeight();

  BeginOrtho();

  OpenGLShaderBase::ReloadIfNeeded();
  texture_count = 0;

  glGetError();
  program->Attach();
  unsigned int idResolution = program->GetUniform("iResolution");
  unsigned int idGlobalTime = program->GetUniform("iGlobalTime");

  // set user parameters
  SetParametersFromList(pInstance);

  if (idResolution != -1) {
    glUniform2f(idResolution, (float)width, (float)height);    
  }
  if (idGlobalTime != -1) {
    glUniform1f(idGlobalTime, (float)t);    
  }


  if (useblend->v->boolean) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // glColor4f(1,1,1, alpha->v->float_val);
      // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
      //glBlendFunc(GL_ONE, GL_ONE);
      //printf("%f:%f\n",t,alpha->v->float_val);
    }


  if (depthwrite->v->boolean) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
  }

  if (texture->v->int_val != 0) {
      glActiveTexture(GL_TEXTURE0+texture_count);
      glBindTexture(GL_TEXTURE_2D,texture->v->int_val);
      texture_count++;
  }



  glBegin(GL_QUADS);
  /*
    glTexCoord2f(0.0f,0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f,0.0f);
    glVertex2f(1, -1.0f);
    glTexCoord2f(1.0f,1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f,1.0f);
    glVertex2f(-1.0f, 1.0f);
    */
    for(int i=0;i<texture_count;i++) glMultiTexCoord2f(GL_TEXTURE0+i,0.0f,0.0f);
    glVertex2f(-1.0f, -1.0f);
    for(int i=0;i<texture_count;i++) glMultiTexCoord2f(GL_TEXTURE0+i,1.0f,0.0f);
    glVertex2f(1, -1.0f);
    for(int i=0;i<texture_count;i++) glMultiTexCoord2f(GL_TEXTURE0+i,1.0f,1.0f);
    glVertex2f(1.0f, 1.0f);
    for(int i=0;i<texture_count;i++) glMultiTexCoord2f(GL_TEXTURE0+i,0.0f,1.0f);
    glVertex2f(-1.0f, 1.0f);
  glEnd();  //glBegin(GL_QUADS);


  if (texture_count > 0) {
      for(int i=0;i<texture_count;i++) {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      glActiveTexture(GL_TEXTURE0);    // set this back to 0 other wise the active texture is still valid
      glDisable(GL_TEXTURE_2D);

  }
  if (useblend->v->boolean) {
      glDisable(GL_BLEND);
  }

  program->Detach();

  EndOrtho();
  //glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
}

void OpenGLShaderQuad::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  OpenGLShaderBase::Initialize(ySys, 
    std::string(vertexshader_source->v->string),
    std::string(pixelshader_source->v->string));

  OpenGLShaderBase::ReloadIfNeeded(true);
  if (program == NULL) {
    ySys->GetLogger("GLShaderQuad")->Error("Shader program load error");
    exit(1);
  }

  // TODO: Go through children and find any shader parameters
  IDocNode *pNode = pInstance->GetDocumentNode();
  int nChildren = pNode->GetNumChildren(kNodeType_ObjectInstance);
  if (nChildren == 0) return;

  pInstance->GetLogger()->Debug("Checking %d child nodes for shader parameters",nChildren);
  for (int i=0;i<nChildren;i++)
  {
    //    IPluginObjectInstance *pObject = pDoc->GetChildAt(pInstance, i, kNodeType_ObjectInstance);
    IDocNode *pChildNode = pNode->GetChildAt(i, kNodeType_ObjectInstance);
    IPluginObjectInstance *pObject = dynamic_cast<IPluginObjectInstance *>(pChildNode->GetNodeObject());
    OpenGLShaderParameter *shaderParam = dynamic_cast<OpenGLShaderParameter *> (pObject->GetExtObject());
    if (shaderParam != NULL)
    {

      shaderParams.push_back(shaderParam);
    } else
    {
      pInstance->GetLogger()->Error("Unsupported child type, gl.ShaderQuad only supports 'gl.ShaderParam'");
    }
  }

  pInstance->GetLogger()->Debug("Dump params");
  for(int i=0;i<shaderParams.size();i++) {
    OpenGLShaderParameter *param = shaderParams[i];
    std::string name = param->GetName();
    std::string tname = param->GetTypeName();
    ShaderParamType tParam = param->GetType();

    pInstance->GetLogger()->Debug("  name: %s, type=%s (%d)", name.c_str(), tname.c_str(), tParam);
  }

}

void OpenGLShaderQuad::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}


