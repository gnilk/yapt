#include "TreeGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <random>


#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif


class StringUtil {
  static std::string whiteSpaces;

  public:
    static void trimRight( std::string& str, const std::string& trimChars = whiteSpaces );
    static void trimLeft( std::string& str, const std::string& trimChars = whiteSpaces );
    static std::string &trim( std::string& str, const std::string& trimChars = whiteSpaces );
    static std::string toLower(std::string s);
    static bool equalsIgnoreCase(std::string a, std::string b);
};

class Parser {
public:
  Parser(std::string data);
  bool Finished();
  void Rewind();
  int NextChar();
  int PeekNextChar();
protected:
  int idxCurrent;
  std::string data;

};

class Command {
public:
  Command(std::string name);
  void PushArg(std::string arg);
  void Execute();
  void Dump(ILogger *logger);
protected:
  std::string name;
  std::vector<std::string> arguments;
};

class Rule {
public:
  Rule(std::string name, std::string definition);
  std::string Name() { return name; }
  char Apply(float *from, float *dir);
  void Dump(ILogger *logger);
protected:
  void Parse();

  std::string name;
  std::string definition;

  std::vector<Command *> commands;
};

static std::default_random_engine generator;
static std::uniform_real_distribution<float> dist_forward;
static std::uniform_real_distribution<float> rotation;//(-M_PI/3.0,M_PI/3.0);
static std::uniform_int_distribution<> branch;



void TreeGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  this->instance = pInstance;

  // 1.04716667 -> PI/3
  rotrange = pInstance->CreateProperty("rotrange", kPropertyType_Float, "1.04717", "");
  distmin = pInstance->CreateProperty("distmin", kPropertyType_Float, "0.6", "");
  distmax = pInstance->CreateProperty("distmax", kPropertyType_Float, "0.8", "");
  branchmin = pInstance->CreateProperty("branchmin", kPropertyType_Integer, "2", "");
  branchmax = pInstance->CreateProperty("branchmax", kPropertyType_Integer, "6", "");
  seed = pInstance->CreateProperty("seed", kPropertyType_Integer, "6","");
  maxdepth = pInstance->CreateProperty("maxdepth", kPropertyType_Integer, "5", "");
  maxbranches = pInstance->CreateProperty("maxbranches", kPropertyType_Integer, "2048", "");

  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
}


#define MAX_VERTEX 1024
#define MAX_LINES 1024

void TreeGenerator::Render(double t, IPluginObjectInstance *pInstance) {


  float start[3] = {0,0,0};
  float direction[3] = {0,1,0};

  float rr = rotrange->v->float_val;

  generator.seed(seed->v->int_val);

  dist_forward = std::uniform_real_distribution<float>(distmin->v->float_val, distmax->v->float_val);
  rotation = std::uniform_real_distribution<float>(-rr,rr);
  branch = std::uniform_int_distribution<>(branchmin->v->int_val, branchmax->v->int_val);


  instance->GetLogger()->Debug("Max Branches: %d", maxbranches->v->int_val);

  pVertex = (float *)malloc(sizeof(float) * 2 * 3 * (maxbranches->v->int_val+4));
  pLines = (int *)malloc(sizeof(int) * 2 * (maxbranches->v->int_val+4));
  nVertex = 0;
  nLines = 0;

  instance->GetLogger()->Debug("Generating tree");
  float mat[4*4];
  BuildRotationMatrix(mat, 0,0,0);
  // build cirular end of cylinder
  Generate(start, direction, 0);


  // std::string name = "Rule_A";
  // std::string definition =\
  //  "push\n"\
  //   "frand (1,5)\n"\
  //   "forward(4)\n"\
  //   "frand\n";

  // Rule rule(name, definition);
  // pInstance->GetLogger()->Debug("DUMPASDASD");
  // rule.Dump(pInstance->GetLogger());
  instance->GetLogger()->Debug("Vertex: %d", nVertex);
  instance->GetLogger()->Debug("Lines : %d", nLines);

  for(int i=0;i<nLines;i++) {

  }

  vertexCount->v->int_val = nVertex-2;
  vertexData->v->userdata = pVertex;
  lineCount->v->int_val = nLines>>1;
  lineData->v->userdata = pLines;
}

void TreeGenerator::Generate(float *start, float *direction, int depth) {
  float point[3];

  if (depth > maxdepth->v->int_val) {
    instance->GetLogger()->Debug("Max depth reached, quitting");
    return;
  }

  float dist = 1;
  vMul (point, direction, dist);
  vAdd(point, point, start);
  if (!AddSegment(start, point)) {
    return;
  }

  float org_direction[3];
  vDup(org_direction, direction);

  //Generate(point, direction, depth+1);

  int branches = branch(generator);
  //instance->GetLogger()->Debug("Branches: %d", branches);
 
  for(int i=0;i<branches;i++) {

    // allow distance multiplier to be different per branch
    dist = dist_forward(generator);
    vMul(direction, org_direction, dist);

    // float tmp_x[3], tmp_y[3], newdir[3];
    // RotateX(tmp_x, direction, xv);
    // RotateY(tmp_y, tmp_x, yv);
    // RotateZ(newdir, tmp_y, zv);

    float mat[4*4], newdir[3];
    BuildRotation(mat);
    vApplyMat(newdir, direction, mat);

    Generate(point, newdir, depth+1);


    if ((nLines-2) > maxbranches->v->int_val) return;
    if ((nVertex-2) > maxbranches->v->int_val) return;

  }
}

float *TreeGenerator::RotateX (float *out_v, float *v, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, ang,0,0);
  vApplyMat(out_v, v, mat);
}

float *TreeGenerator::RotateY (float *out_v, float *v, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, 0,ang,0);
  vApplyMat(out_v, v, mat);
}

float *TreeGenerator::RotateZ (float *out_v, float *v, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, 0,0,ang);
  vApplyMat(out_v, v, mat);
}
void TreeGenerator::BuildRotation(float *out_m) {
    float xv,yv,zv;
    xv = rotation(generator);
    yv = rotation(generator);
    zv = rotation(generator);  
    BuildRotationMatrix(out_m, xv,yv,zv);

}
void TreeGenerator::BuildRotationMatrix(float *out_m, float xv, float yv, float zv) {
    float xm[4*4],ym[4*4],tmp_mat[4*4];
    matIdentity(xm);
    matIdentity(ym);
    matIdentity(out_m);
    matIdentity(tmp_mat);
    RotateXm(xm, tmp_mat, xv);
    RotateYm(ym, xm, yv);
    RotateZm(out_m, ym, zv);

}


float *TreeGenerator::RotateXm (float *out_m, float *m, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, ang,0,0);
  matMul(out_m, m, mat);
  return out_m;
}

float *TreeGenerator::RotateYm (float *out_m, float *m, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, 0,ang,0);
  matMul(out_m, m, mat);
  return out_m;
}

float *TreeGenerator::RotateZm (float *out_m, float *m, float ang) {
  float mat[4*4];
  matIdentity(mat);
  matRotateAngles(mat, 0,0,ang);
  matMul(out_m, m, mat);
  return out_m;
}


bool TreeGenerator::AddSegment(float *p1, float *p2) {

  if ((nVertex-2) > maxbranches->v->int_val) {
    instance->GetLogger()->Debug("Max vertex reached, quitting");
    return false;
  }
  if ((nLines-2) > maxbranches->v->int_val) {
    instance->GetLogger()->Debug("Max lines reached, quitting");
    return false;
  }
  int l1, l2;

  l1 = AddVertex(p1);
  l2 = AddVertex(p2);


  pLines[nLines++]=l1;
  pLines[nLines++]=l2;

  return true;


}

int TreeGenerator::AddVertex(float *p) {
  vDup(&pVertex[nVertex*3],p);
  nVertex++;
  return nVertex-1;
}


void TreeGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void TreeGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}


Rule::Rule(std::string name, std::string definition) {
  this->name = name;
  this->definition = definition;
  Parse();
}

Command::Command(std::string name) {
  this->name = name;
}

void Command::PushArg(std::string arg) {
  arguments.push_back(arg);
}
void Command::Dump(ILogger *logger) {

  if (name.length() == 0) return;
  std::string tolog;
  tolog = "C:"+name;
  //logger->Debug("%s",name.c_str());
  if (arguments.size() > 0) {
    //logger->Debug(" (");
    tolog += "(";
    for(int i=0;i<arguments.size();i++) {
      //logger->Debug("%s, ",arguments[i]);
      tolog += arguments[i];
      if (!(i==arguments.size()-1)) {
        tolog += ",";
      }
    }
    //logger->Debug(")");
    tolog += ")";
  } else {

  }
  logger->Debug("%s",tolog.c_str());
}
void Command::Execute() {

}

Parser::Parser(std::string data) {
  this->data = data;
}

bool Parser::Finished() {
  if (idxCurrent >= data.length()) return true;
  return false;
}

void Parser::Rewind() {
  idxCurrent--;
  if (idxCurrent < 0) idxCurrent = 0;
}

int Parser::NextChar() {
  if (idxCurrent >= (int)data.length()) return EOF;
  return data.at(idxCurrent++);
}

int Parser::PeekNextChar() {
  if (idxCurrent >= (int)data.length()) return EOF;
  return data.at(idxCurrent);
}

char Rule::Apply(float *from, float *dir) {

}

void Rule::Dump(ILogger *logger) {
  logger->Debug("DUMPING RULE");
  logger->Debug("%s",name.c_str());
  for(int i=0;i<commands.size();i++) {
    Command *cmd = commands[i];
    cmd->Dump(logger);
  }
}

typedef enum {
  psToken,
  psArgument,
} ParseState;

void Rule::Parse() {
  Parser parser(definition);
  Command *command;
  std::string token;
  ParseState state = psToken;
  int c;
  while ((c = parser.NextChar()) != EOF) {
    printf("%d,%s\n",state, token.c_str());
    switch(state) {
      case psToken :
        if (c=='(') {
          command = new Command(StringUtil::trim(token));
          token = "";
          state = psArgument;
        } else if (c=='\n') {
            token = StringUtil::trim(token);
            command = new Command(token);
            printf("Add command: '%s'\n", token.c_str());
            commands.push_back(command);
            token = "";          
        } else {
          token += c;
        }
        break;
      case psArgument :
        if (c==')') {
          if (!token.empty()) {
            command->PushArg(StringUtil::trim(token));
          }
          commands.push_back(command);
          token = "";
          state = psToken;
        } else if (c==',') {          
          command->PushArg(StringUtil::trim(token));
          token = "";
        } else {
          token += c;
        }
        break;
    } // switch
  } // while
}

///////////////
std::string StringUtil::whiteSpaces( " \f\n\r\t\v" );

void StringUtil::trimRight( std::string& str, const std::string& trimChars /*= whiteSpaces*/ )
{
  std::string::size_type pos = str.find_last_not_of( trimChars );
  str.erase( pos + 1 );    
}


void StringUtil::trimLeft( std::string& str, const std::string& trimChars /*= whiteSpaces*/ )
{
  std::string::size_type pos = str.find_first_not_of( trimChars );
  str.erase( 0, pos );
}

std::string &StringUtil::trim( std::string& str, const std::string& trimChars /*= whiteSpaces*/ )
{
  trimRight( str, trimChars );
  trimLeft( str, trimChars );
  return str;
}

std::string StringUtil::toLower(std::string s) {
  std::string res = "";
  for(size_t i=0;i<s.length();i++) {
    res+=((char)tolower(s.at(i)));
  }
  return res;
}
bool StringUtil::equalsIgnoreCase(std::string a, std::string b) {
  std::string sa = toLower(a);
  std::string sb = toLower(b);
  return (sa==sb);
}

/*
Rule A
  push
  frand 
  forward
  frand
  rotate x
  frand
  rotate z
  build
  pop
  push
  branch a,b,c
  pop
  push
  branch a,b,c
  pop
  push
  branch a,b,c
  pop


}
*/