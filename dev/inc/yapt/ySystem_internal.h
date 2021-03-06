/*-------------------------------------------------------------------------
File    : $Archive: ySystem_internal.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Defines the classes used by Yapt. The classes are never exposed
          externally.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Split this file
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/

#pragma once

#include <vector>
#include <map>
#include <stack>
#include <string>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/FileWatcher.h"

#include "noice/io/io.h"
using namespace noice::io;


#ifdef WIN32
#undef GetObject
#endif

namespace yapt
{
  // TODO:
  // Evaluate if we should include a base-object interface
  // which would allow us to query pointers. Like a type safe
  // upcast. Currently we can not properly transists between interfaces
  // and classes. Thus, we would need a Query function.
  //
  // For instance, the parser can not be put outside (as an external) since it requires
  // access to more information than provided by interfaces..  We should fix this!
  //

  struct IPluginScanner
  {
  public:
    virtual void ScanForPlugins(ISystem *ySys, const char *fullpath, bool bRecurse) = 0;
  };
#ifdef WIN32
  class PluginScanner_Win32 :
    public IPluginScanner
  {
  protected:
    ISystem *ySys;
    ILogger *pLogger;
    bool bRecurse;
    bool IsExtensionOk(std::string &extension);
    std::string GetExtension(std::string &pathName);


    void TryLoadLibrary(std::string &pathName);
    void ScanDirectory(std::string path);
  public:
    PluginScanner_Win32();
    void ScanForPlugins(ISystem *ySys, const char *fullpath, bool bRecurse);
  };
#else
  class PluginScanner_Nix :
    public IPluginScanner
    {
    protected:
      ISystem *ySys;
      ILogger *pLogger;
      bool bRecurse;
      bool IsExtensionOk(std::string &extension);
      std::string GetExtension(std::string &pathName);
      
      
      void TryLoadLibrary(std::string &pathName);
      void ScanDirectory(std::string path);
    public:
      PluginScanner_Nix();
      void ScanForPlugins(ISystem *ySys, const char *fullpath, bool bRecurse);
    };
#endif

  // -- classes

  
  class Attribute :
    public IAttribute
  {
  protected:
    char *name;
    char *value;
  public:
    Attribute(const char *name, const char *value);
    virtual ~Attribute();

    virtual const char *GetName();
    virtual const char *GetValue();
    virtual void SetValue(const char *value);
  };


  // define interface for this class in order to provide
  // attribute lookup for externals
  class BaseInstance :
    public IBaseInstance
  {
  protected:
    kInstanceType type;
    char *qualifiedName;
    std::vector<Attribute *> attributes;

    IContext *pContext;	// Context for this object

    Attribute *GetAttributeAsClass(int index);
    Attribute *GetAttributeAsClass(const char *name);

    // TODO: Refactor this one
    int StrConfCaseCmp(const char *sA, const char *sB);
  public:
    BaseInstance(kInstanceType mType);
    virtual ~BaseInstance();

    virtual kInstanceType GetInstanceType();
    virtual const char *GetFullyQualifiedName();

    virtual int GetNumAttributes();
    virtual IAttribute *GetAttribute(int index);
    virtual IAttribute *GetAttribute(const char *name);
    virtual const char *GetAttributeValue(const char *name);
    virtual const char *GetAttributeValue(const char *name, const char *sDefault);
    virtual IAttribute *AddAttribute(const char *name, const char *value);
    virtual IAttribute *UpdateAttribute(const char *name, const char *value);
    virtual IContext *GetContext();
  public:
    // - not exposed through interface
    void SetFullyQualifiedName(const char *qName);
    void SetContext(IContext *pContext);

    virtual void OnAttributeChanged(Attribute *pAttribute);
  };

  class MetaInstance : 
    public IMetaInstance,
    public BaseInstance
    
  {
  protected:
    IDocument *pDocument;
  public:
    MetaInstance(IDocument *pOwner);
    virtual ~MetaInstance();

    virtual IDocument *GetDocument();
  };

  class CommentInstance : 
    public ICommentInstance,
    public BaseInstance
    
  {
  protected:
    char *comment;
  public:
    CommentInstance();
    virtual ~CommentInstance();

    virtual char *GetComment();
    virtual void SetComment(char *);
  };


  typedef std::pair<IPluginObject *, IDocNode *> PluginNodePair;
  typedef std::map<IPluginObject *, IDocNode *> PluginNodeMap;
  //struct INodeMapper
  //{
  //public:
  //	virtual void RegisterNode(IDocNode *pNode, IBaseInstance *pInstance)= 0;
  //	virtual void DeregisterNode(IDocNode *pNode) = 0;
  //};

  //class NodeToPluginObjectLookup :
  //	public INodeMapper
  //{
  //private:
  //	PluginNodeMap nodehash;
  //public:
  //	virtual void RegisterNode(IDocNode *pNode, IBaseInstance *pInstance);
  //	virtual void DeregisterNode(IDocNode *pNode);
  //	virtual IDocNode *FindNode(IPluginObject *pObject);
  //};

  class PluginObjectInstance;
  
  class Signal : 
    public ISignal,
    public BaseInstance 
  {
  public:    
    Signal();
    Signal(float t, int val);
    virtual ~Signal();
    virtual double GetTime();
    virtual int GetValue();
  private:
    float t;
    int value;
  };

  class SignalChannel :
    public ISignalChannel,
    public BaseInstance 
  {
  public:
    SignalChannel();
    SignalChannel(int id, const char *name, float accuracy);
    virtual ~SignalChannel();
    virtual int GetId();
    virtual const char *GetName();
    virtual void SetName(const char *name);
    virtual int GetNumSignals();
    virtual ISignal *GetSignalAt(int idx);
    virtual ISignal *AddSignal(double t, int sigvalue);
    virtual void RemoveSignal(int idx);
    virtual ISignal *RaiseNext(double t);
    virtual void RegisterSinkObject(IPluginObjectInstance *sink);
    virtual void SendToSinks(ISignal *signal);
  private:
    int idxCurrentSignal;
    int id;
    std::string name;
    float accuracy;
    std::vector<Signal *>signals;
    std::vector<IPluginObjectInstance *>sinks;
  };

  class Signals :
    public ISignals,
    public BaseInstance
  {
  public:
    Signals();
    virtual ~Signals();
    virtual int GetNumChannels();
    virtual ISignalChannel *GetChannel(int idx);
    virtual ISignalChannel *GetChannel(const char *name);
    virtual ISignalChannel *AddChannel(int id, const char *name, double accuracy);
    virtual void RemoveChannel(int idChannel);
    virtual void RemoveChannel(const char *name);
  private:
    std::vector<SignalChannel *> channels;
  };


  class TimelineExecute :
    public ITimelineExecute,
    public BaseInstance
  {
  public:
      TimelineExecute();
      virtual ~TimelineExecute();
      // interface
      virtual float GetStart();
      virtual float GetDuration();
      virtual char *GetObjectName();
      virtual bool ShouldRender(double t);
  public:
      void SetParam(float _start, float _duration, char *_objectName);

      IDocNode *GetNode() { return node; };
      void SetNode(IDocNode *node) { this->node = node; }
  private:
      float start;
      float duration;
      char *objectName;
  private:
      // runtime cache variables
      IDocNode *node;
  };

  class Timeline :
    public ITimeline,
    public BaseInstance
  {
  protected:
    std::vector<ITimelineExecute *> executeObjects;
  public:
    Timeline();
    virtual ~Timeline();
    virtual ITimelineExecute *AddExecuteObject(float start, float duration, char *objectName);
    virtual int GetNumExecutors();
    virtual IBaseInstance *GetExecutorAtIndex(int idx);
  public:
    static void RegisterTimelineObjectFactory(ISystem *pSys);
  };

  class ResourceContainer :
    public IResourceContainer, 
    public BaseInstance

  {
  protected:
    IDocNode *resourceRoot;	// TODO: Remove
    
  public:
    ResourceContainer();
    virtual ~ResourceContainer();
    virtual bool Initialize();
    virtual bool InitializeAllResources();
    virtual IPluginObjectInstance *GetResourceInstance(const char *name);
    virtual void AddResourceInstance(IPluginObjectInstance *pInstance);
    virtual void *GetIOHandler();
    virtual noice::io::IStream *OpenStream(const char *sStreamName, unsigned int flags);
  };

  class DocNode :
    public IDocNode
  {
  protected:
    IDocument *document;
    IDocNode *parent;
    std::vector<IDocNode *>children;
    kNodeType nodeType;

    IBaseInstance *userObject;	// user defined object
    
  public:
    DocNode(IDocument *doc);
    virtual ~DocNode();
    virtual bool Dispose();	// deletes the node
    virtual IDocNode *GetParent();
    virtual int GetNumChildren();
    virtual int GetNumChildren(kNodeType ofType);
    virtual void AddChild(IDocNode *child);
    virtual void RemoveChild(IDocNode *child);
    virtual IDocNode *GetChildAt(int idx);
    virtual IDocNode *GetChildAt(int idx, kNodeType ofType);
    virtual IDocument *GetRootDocument();		
    virtual IBaseInstance *GetNodeObject();
    virtual void SetNodeObject(IBaseInstance *pObject, kNodeType type);
    virtual kNodeType GetNodeType();
  };

  class Timer
  {
  protected:
    double t;
    bool bUpdate;
  public:
    Timer();
    virtual ~Timer();

    virtual void Update(double newTime);
    // interface
  public:
    virtual double GetTime();
    virtual void Start();
    virtual void Stop();
  };
  // for timers in render vars
  typedef std::pair<unsigned int, Timer *> TimerIDPair;
  typedef std::map<unsigned int, Timer *> TimerIDMap;
  // promote to own file
  class RenderVars :
    public IRenderVars
  {
  protected:
    TimerIDMap timers;
    std::stack<double> localtime;
    double tGlobal;
    unsigned int renderReference;
  public:
    RenderVars();
    virtual ~RenderVars();

    void SetTime(double tGlobal);
    void PushLocal(double tStart);
    void PopLocal();
    void IncRenderRef();
    unsigned int GetRenderRef();
    // IRenderVars - interface
  public:
    virtual double GetLocalTime();
    virtual double GetTime();
    virtual double GetTimer(unsigned int idTimer);
    virtual void CreateTimer(unsigned int idTimer, unsigned int flags);
    virtual void UpdateTimer(unsigned int idTimer, double newTime);
  };

  #define FPS_CONTROLLER_TIME_WINDOW_SIZE 16  
  class FPSController
  {
  public:
    FPSController();
    virtual ~FPSController();

    void Update(double time);
    void Reset();
    double GetFPS();
    double GetAverageFPS();

  private:
    double lastUpdate;
    double fpsWindow[FPS_CONTROLLER_TIME_WINDOW_SIZE]; // 32 samples in window
    int deltaIndex;
    int deltaCount;
    double fpsCurrent;
    double fpsAverage;
  };


  // A variable is somewhat different from a Property although serving the same purpose
  // instead of a visible intermediate structure the app layer allows direct linkage 
  // to the data of the plugin
  // There is a slight overhead since the app must - per rendering - walk through and
  // update all the variables for you while the intermediate structure allows for automatic updates
  // directly at the data structure level
  //
  // Hence, variables are more convienient to use but a bit more costly
  //
/*
  class Variable {    
  protected:
    std::string name;
    std::string desc;
    kPropertyType type;
    void *ptrData;
    Variable *link;
    bool linked;

  public:
    bool IsType(kPropertyType other);
    bool IsType(Variable *other);
    kPropertyType GetType() { return type; }

    virtual void Set(std::string value);
    virtual std::string Get();
    virtual void Update();

    bool LinkTo(Variable *pOther);
    bool IsLinked() { return linked; };
    void *BasePointer() { return ptrData; }
  };

  class VariableManager {
  private:
    std::vector<Variable *> variables;
  public:
    Variable *BindVariable(std::string name, kPropertyType type, void *pData, std::string description);
  };
*/

  typedef std::pair<IBaseInstance *, IDocNode *> BaseNodePair;
  typedef std::map<IBaseInstance *, IDocNode *> BaseNodeMap;

  class Document :
    public IDocument,
    public BaseInstance
    {
    protected:
      // TODO: replace resourceRoot with list of resource containers
      // resource container should hold the URI and the IO handler
      IDocNode *root;		// the root of the complete document tree
      IDocNode *renderRoot;	// root node of the rendering branch of the document tree		
      ILogger *pLogger;

      IDocumentController *pDocumentController;		// Current assigned engine

      Signals *signals;
      Timeline *timeline;	// timeline object
      ResourceContainer *resources;	// default resource container (TODO: Replace)

      // TODO: Must move this one to context - several documents can share context
      BaseNodeMap treemap;	// lookup baseinstance/inode
    protected:
      void RegisterNode(IDocNode *pNode, IBaseInstance *pObject);
      void DeregisterNode(IDocNode *pNode);
      IDocNode *CreateNode(IDocNode *parent, IBaseInstance *pObject, kNodeType type);
//      IDocNode *AddNode(IDocNode *parent, IBaseInstance *pObject, kNodeType type);
      void AddNode(IBaseInstance *instance, IDocNode *node);

  //		void RenderNode(ISystem *ySys, IDocNode *node, bool bForce);
      
      IDocNode *AddObjectToParentNode(IDocNode *parentNode, PluginObjectInstance *pObject);
      void DumpNode(IDocNode *pNode);

      IDocNode *AddObjectToTree(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType);
      void DisposeTree(IDocNode *root);

      std::string BuildQualifiedName(IDocNode *pNode);
      // void UpdateRenderVars(double sample_time);

      virtual bool RemoveNodeTraceObjects(IDocNode *pNode, std::vector<IBaseInstance *> &nodeObjects);

      void Initialize();	
    public:
      Document();
      Document(IContext *pContext);
      virtual ~Document();
      
      void SetDocumentController(IDocumentController *pDocumentController);
      IDocumentController *GetDocumentController();

    public:	// IDocument interface
      virtual void Write(noice::io::IStream *stream);
      virtual void Read(noice::io::IStream *stream);
      
      virtual IDocNode *GetTree();
      virtual IDocNode *GetRenderTree();		// returns root of renderable tree			

      // TODO: Add "GetResourceTree"
      
      virtual IBaseInstance *GetRenderRoot();
      virtual IResourceContainer *GetResources();	// returns resource container
      virtual ISignals *GetSignals();
      virtual ITimeline *GetTimeline();
      virtual bool HasTimeline();
    /*	
      virtual void InitializeNode(IDocNode *node); 
      virtual void PostInitializeNode(IDocNode *node);
      virtual IRenderVars *GetRenderVars();
     */


      virtual IBaseInstance *SearchFromNode(IDocNode *pRootNode, const char *name);
      virtual IBaseInstance *GetObjectFromSimpleName(const char *name);
      virtual IBaseInstance *GetObject(const char *fullQualifiedName);
      virtual IDocNode *FindNode(IBaseInstance *pObject);
      
      virtual void MoveNode(IDocNode *pNewParent, IDocNode *pNode);
      virtual void SwapNodes (IDocNode *pNodeA, IDocNode *pNodeB);
      virtual bool RemoveNode(IDocNode *pNode);
      virtual bool RemoveObject(IBaseInstance *pObject);
      
      virtual IDocNode *AddObject(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType);
      virtual IDocNode *AddRenderObject(IBaseInstance *parent, IBaseInstance *object);
      virtual IDocNode *AddToTimeline(IBaseInstance *object);
      virtual IDocNode *AddSignalChannel(ISignalChannel *object);
      virtual void AddResourceObject(IBaseInstance *parent, IBaseInstance *object);
      virtual IDocNode *AddMetaObject(IBaseInstance *parent);
      virtual IDocNode *AddCommentObject(IBaseInstance *parent);
      virtual void DumpRenderTree();

      virtual int GetNumChildren(IPluginObjectInstance *pObject);
      virtual int GetNumChildren(IPluginObjectInstance *pObject, kNodeType ofType);
      virtual IPluginObjectInstance* GetChildAt(IPluginObjectInstance *pObject, int index, kNodeType ofType);
      virtual IPluginObjectInstance* GetChildAt(IPluginObjectInstance *pObject, int index);
    };
  
  
  class DocumentController :
    public IDocumentController
  {
  private:
    ILogger *pLogger;
    RenderVars *renderVars;	// holds the rendering variables
    FPSController fpsController;
    ISystem *pSys;
    IDocument *pDocument;
    std::vector<IDocNode *> dirtyNodes;
    float timelineMaxTime;
  private:
    void UpdateRenderVars(double sample_time);
    bool BindAllProperties(IDocNode *node);
    void TraverseNode(IDocumentTraversalSink *sink, IDocNode *node, int depth);
    void RenderSignals(double t);
    void RenderResourcesForDocument(IDocument *pDocument);
    void RenderResourcesForIncludes(IDocNode *node);
  public:
    DocumentController(IDocument *pDocument);
    virtual ~DocumentController();

    virtual IRenderVars *GetRenderVars();

    virtual void SetDirty(IDocNode *pNode);
  
    virtual bool Initialize();
    virtual void InitializeNode(IDocNode *node); 
    virtual bool PostInitializeNode(IDocNode *node);
    virtual void TraverseDocument(IDocumentTraversalSink *sink);
    
    virtual void Render(double sample_time);
    virtual void RenderResources();
    virtual void RenderNode(IDocNode *pNode, bool bForce);
    virtual void RenderTimeline();
    virtual float GetTimeLineMaxTime();
     // internal
  public:
    void SetDocument(IDocument *pDocument);
  };
  
  
  // -- internal	
  // The plugin definition holds the description (XML)
  // and a pointer to the factory object used to create an instance (IPluginObjectInstance)
  class PluginObjectDefinition :
    public BaseInstance,
    public IPluginObjectDefinition
  {
  protected:
    IPluginObjectFactory *factory;
    ILogger *pLogger;
    char *description;

    PluginObjectDefinition();
    bool Initialize(IPluginObjectFactory *factory, const char *description);
    bool ParseDescriptionString();
  public:
    virtual ~PluginObjectDefinition();
    
    static PluginObjectDefinition *Create(IPluginObjectFactory *factory, const char *description);

    IPluginObjectInstance *CreateInstance();
    IPluginObject *CreateExternalInstance();
    
    virtual void Dump();

    IPluginObjectFactory *GetFactory();		
    const char *GetDescription();
  };

  class PluginObjectInstance;
  
  class PropertyInstance :
    public IPropertyInstance,
    public BaseInstance
  {
  protected:
    Property *property;
    PropertyValue *v_unsourced;	// the value backup when sourceing
    ILogger *pLogger;
    PropertyInstance *sourcedProperty;
    PluginObjectInstance *objectInstance;
    kPropertyHint propertyHint; 
    bool isSourced;
    char *unboundRawValue;
    char *sourceString;
    char *sDescription;
    char *textDescription;
    char *userTypeName;
    int iSourceReferences;

    void ParseDescriptionString();

  public:
    static void StrSplit(std::vector<std::string> &strings, const char *strInput, int splitChar);
    char *GetEnumFromValue(char *sDest, int nMax, int val, const char *def);
    bool PrepareEnumString(std::vector<std::string> &strings, const char *def);
    int ParseEnumString(const char *val, const char *def);
    int ParseFloatArrayToProperty(Property *property, const char *val);
  public:
    PropertyInstance(const char *szName, kPropertyType type, const char *sDescription);
    virtual ~PropertyInstance();
    
    Property *GetProperty();
    void SetProperty(Property *property);
    void SetPropertyHint(kPropertyHint hint);
    kPropertyHint GetPropertyHint();
    
    const char *GetName();
    const char *GetDescription();
    bool IsEqualType(PropertyInstance *pOther);    
    void SetDescription(const char *strDesc);
    
    const char *GetUnboundRawValue();
    int IncSourceRef();
    int DecSourceRef();
    void SetSource(PropertyInstance *pSource);
    void BindProperty(char *propertyReference);
    
    
    void SetObjectInstance(PluginObjectInstance *owner);
    PluginObjectInstance *GetPluginObjectInstance();

    std::string GetTypeName();

    virtual void Dump();

  public: // BaseInstance overrides
    void OnAttributeChanged(Attribute *pAttribute);

  public:	// interface
    virtual kPropertyType GetPropertyType();
    virtual char *GetPropertyTypeName(char *sDest, int maxLen);
    virtual void SetValue(const char *sValue);
    virtual char *GetValue(char *sValueDest, int maxlen);
    virtual std::vector<std::string> GetValidEnumValues();
    virtual bool IsSourced();
    virtual bool IsOutputProperty();
    virtual char *GetSourceString();
    virtual IPropertyInstance *GetSource();
    virtual IBaseInstance *GetObjectInstance();


  };

  typedef std::pair<std::string, void *> CtxNameObjectPair;
  typedef std::map<std::string, void *> CtxNameObjectMap;

  class Context :
    public IContext
  {
  private:
    ILogger *pLogger;
    Document *pDocument;
    DocumentController *pDocumentController;
    CtxNameObjectMap objects;
    std::string namePrefix;
    //void *contextParamObject;
    std::stack<CtxNameObjectPair> contexParamObjectStack;
    std::stack<IBaseInstance *> renderObjectStack;

    BaseNodeMap baseNodeMap;  // lookup baseinstance/inode
  public:
    Context();
    virtual ~Context();
    
    void SetDocumentController(DocumentController *pController);
    // interface
  public:
    virtual IDocument *GetDocument();
    virtual void SetDocument(IDocument *pDocument);
    virtual IDocumentController *GetDocumentController();
    virtual void SetObject(const char *name, void *pObject);
    virtual void *GetObject(const char *name);


    virtual void PushRenderObject(IBaseInstance *);
    virtual void PopRenderObject();
    virtual IBaseInstance *TopRenderObject();

    virtual void PushContextParamObject(void *pObject, const char *name);
    virtual void PopContextParamObject();
    virtual void *TopContextParamObject();

    virtual void SetNamePrefix(const char *prefix);
    virtual char *GetNamePrefix(char *pdest, int nmaxlen);
    virtual char *CreatePrefixName(const char *name, char *prefixedname, int nmaxlen);

  public:
    virtual void AddNode(IBaseInstance *instance, IDocNode *node);
    virtual IDocNode *FindNode(IBaseInstance *pObject);
    virtual void EraseNode(IBaseInstance *instance);
  };
  

  // This is the internal PluginObjectInstance where we keep track of things
  class PluginObjectInstance :
    public IPluginObjectInstance,
    public BaseInstance
  {
  protected:
    typedef enum 
    {
      kExtState_None,
      kExtState_Created,
      kExtState_Initialized,
    } kExtState;


    PluginObjectDefinition *pDef;
    IDocument *pDocument;
    
    IPluginObject *extObject;	// external object
    
    std::vector<PropertyInstance *> input_properties;
    std::vector<PropertyInstance *> output_properties;

    IPropertyInstance *FindPropertyInstance(const char *propertyReference, IDocNode *pRootNode);
    IPropertyInstance *FindPropertyInstanceTopDown(const char *propertyReference, IDocNode *pRootNode);
    IPropertyInstance *FindPropertyInstance_TEMPNAME(const char *propertyReference, IDocNode *pRootNode);
    IPropertyInstance *FindPropertyInstanceFromRoot(const char *propertyReference);
    PropertyInstance *GetPropertyInstance(std::vector<PropertyInstance *> *pList, const char *szName);
    PropertyInstance *GetPropertyInstance(std::vector<PropertyInstance *> *pList, int index);

    void CreateDefaultAttributes();

    kExtState extState;
    unsigned int lastRenderRef;
    // 'dirtyFlag' controls synchronization between Render/PostRender (render set's to true, post set's to false) 
    // Done to avoid multiple calls to postRender for the same render counter  
    bool dirtyFlag;
    bool enablePropertyRefRendering;

    bool IsPropertyRefRendering();
    void SetPropertyRefRendering(bool _enablePropertyRefRendering);
    bool IsDirty();
    void SetDirty(bool dirty);
    void RenderPropertyDependencies(RenderVars *pRenderVars);
    Property *GetProperty(const char *name, bool isOutput);
    bool IsEqualPropertyTypes(PropertyInstance *p1,  PropertyInstance *p2);

  public:
    PluginObjectInstance(PluginObjectDefinition *definition);
    PluginObjectInstance(IPluginObjectDefinition *definition);
    virtual ~PluginObjectInstance();
    
    PluginObjectDefinition *GetDefinition();
    void SetDefinition(PluginObjectDefinition *pDef);
    // TODO: Rename, this is creating the ext object
    bool CreateInstance();
    bool IsResource();
    virtual IPluginObject *GetExtObject();
    void SetExtObject(IPluginObject *pObject);
    
    void SetDocument(IDocument *pDoc);
    //Document *GetDocument();

    void Dump();
    // Property Instance functions
    IPropertyInstance *GetPropertyInstance(const char *szName, bool bOutput);
    void AddPropertyInstance(PropertyInstance *property, bool bOutput);
    bool IsPropertyInstanceOutput(PropertyInstance *pInst);
  
    bool BindProperties();
    //bool ShouldRender(double glbTime);
    bool ShouldRender(RenderVars *pRenderVars);

    // Functions to access most common attributes
    double GetStartTime();
    void SetStartTime(double time_sec);
    double GetDuration();
    void SetDuration(double durtaion_sec);
    void SetInstanceName(const char *sNewName);

  public:	// external functions
    void ExtRender(RenderVars *pRenderVars);
    void ExtInitialize();
    void ExtPostRender();
    void ExtPostInitialize();
  public:
    // external interface
    virtual Property *CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription);
    virtual Property *CreateOutputProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription);
    virtual bool BindVariable(const char *sName, kPropertyType type, const char *sDescription);
    virtual void SetPropertyHint(const char *sName, kPropertyHint hint);
    virtual kPropertyHint GetPropertyHint(const char *sName);
    virtual Property *GetProperty(const char *name);
    virtual void SetPropertyValue(Property *prop, const char *value);
    virtual IPropertyInstance *GetPropertyInstance(const char *name);
    virtual IPropertyInstance *GetPropertyInstance(int index, bool bOutput);
    virtual int GetNumInputProperties();
    virtual int GetNumOutputProperties();
    virtual Property *GetInputPropertyAt(int index);
    virtual Property *GetOutputPropertyAt(int index);

    virtual const char *GetClassName();
    virtual const char *GetInstanceName();


    virtual IDocument *GetDocument();
    virtual IDocNode *GetDocumentNode();
    virtual ILogger *GetLogger();
  
  public:	// overrides from BaseInstance
    virtual void OnAttributeChanged(Attribute *pAttribute);


  protected:
    Property *CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription, bool bOutput);
  };

  // holds information about library
  class PluginContainer :
    public BaseInstance
  {
  protected:
    std::vector<IBaseInstance*> objects;
    PFNINITIALIZEPLUGIN pFuncInitialize;
  public:
    PluginContainer(PFNINITIALIZEPLUGIN pFuncInitialize);
    virtual ~PluginContainer();
    void AddObjectDefinition(IBaseInstance *pDef);

  };
  
  class SystemHookHandler :
    ISystemDocumentHooks,
    ISystemObjectHooks
  {
  protected:
    ILogger *pLogger;
    bool bInProgress;
    std::vector<ISystemDocumentHooks *>docHookList;
    std::vector<ISystemObjectHooks *>objHookList;

    bool BeginCall();
    void EndCall();
  public:
    SystemHookHandler();
    virtual ~SystemHookHandler();

    void RegisterHooks(ISystemDocumentHooks *pHooks);
    void RemoveHooks(ISystemDocumentHooks *pHooks);
    void GetAllHooks(std::vector<ISystemDocumentHooks*> &hooklist);

    void RegisterHooks(ISystemObjectHooks *pHooks);
    void RemoveHooks(ISystemObjectHooks *pHooks);
    void GetAllHooks(std::vector<ISystemObjectHooks*> &hooklist);
    // document hook adaptor
  public:
    virtual void BeginDocumentImport();
    virtual void DocumentImportProgress(double progress);
    virtual void EndDocumentImport();

    // always synchronous
    virtual void BeginDocumentRender();
    virtual void EndDocumentRender();

    // can be async, use URL to track which resource call belongs to
    virtual void BeginResourceImport(const char *url);
    virtual void ResourceImportProgress(const char *url, double progress);
    virtual void EndResourceImport(const char *url);

    virtual void OnError(kErrorClass eClass, kError eCode);

    // object hook adapter
  public:
    virtual void BeginObjectInitialize(IBaseInstance *pInstance);
    virtual void EndObjectInitialize(IBaseInstance *pInstance);
    virtual void BeginObjectPostInitialize(IBaseInstance *pInstance);
    virtual void EndObjectPostInitialize(IBaseInstance *pInstance);
    virtual void BeginObjectRender(IBaseInstance *pInstance);
    virtual void EndObjectRender(IBaseInstance *pInstance);
    virtual void BeginObjectPostRender(IBaseInstance *pInstance);
    virtual void EndObjectPostRender(IBaseInstance *pInstance);

  };


  typedef std::pair<std::string, IPluginObjectDefinition *> PluginNameDefPair;
  typedef std::map<std::string, IPluginObjectDefinition *> PluginNameDefMap;
  typedef std::pair<std::string, std::string> StrStrPair;	// config pair/map
  typedef std::map<std::string, std::string> StrStrMap;
  typedef std::pair<Property *, IPropertyInstance *> PropInstPair;
  typedef std::map<Property *, IPropertyInstance *> PropInstMap;
  typedef std::pair<std::string, IBaseInstance *> StrBasePair;	// fqname <-> base instance lookup
  typedef std::map<std::string, IBaseInstance *> StrBaseMap;


  class System :
    public ISystem
  {
  protected:
    ILogger *pLogger;

    PluginNameDefMap definitions;
    StrStrMap configuration;

    std::vector<PluginContainer *> plugins;

    FileWatcher fileWatcher;    

    PluginContainer *pLastLoadedContainer;
    IContext *pContext;	// Current context - created indirectly when creating a document
    
  protected:
    //PluginObjectInstance *GetCurrentObject();
    //void SetCurrentObject(PluginObjectInstance *pObject);				

  public:
    System();
    virtual ~System();

    // not exposed through ISystem, called by TopLevel after built-in I/O libraries 
    // has been registered
    int ReadAndStoreDefaultSettings();
    
    virtual IDocument *CreateNewDocument(bool bSetActive);
    virtual void DisposeActiveDocument();
    virtual IDocument *GetActiveDocument();
    virtual IDocumentController *GetActiveDocumentController();
    virtual void SetActiveDocument(IDocument *pDocument);
    virtual IDocument *LoadNewDocument(const char *url);
    virtual void SaveDocumentAs(const char *url, IDocument *pDoc);

    IContext *GetCurrentContext();

    
    bool RegisterObject(IPluginObjectFactory *factory, const char *sDescription);
    IPluginObjectDefinition *GetObjectDefinition(const char *name);

    virtual IBaseInstance *GetControllerObject(IPluginObject *pObject);
    virtual IBaseInstance *GetControllerObject(const char *name);

    // File watching functions
    virtual void WatchFile(const char *filename, IFileWatcher *callback);
    virtual void RemoveFileWatcher(const char *filename);


    // Stream interface
    virtual void *LoadData(const char *filename, unsigned int flags, long *outszdata);
    virtual noice::io::IStream *CreateStream(const char *uri, unsigned int flags);
    virtual noice::io::IStreamDevice *GetIODevice(const char *url_identifier);
    virtual void SetIODevice(noice::io::IStreamDevice *pDevice, const char *url_identifier);
    virtual noice::io::IStreamDevice *CreateIODevice(const char *url_identifier);
    virtual bool RegisterIODevice(noice::io::IStreamDeviceFactory *pDeviceFactory, const char *url_identifier, const char *initparam, bool bCreate);


    // Configuration
    virtual const char *GetConfigValue(const char *key);
    virtual const bool GetConfigBool(const char *key);
    virtual const int GetConfigInt(const char *key);
    virtual const char *GetConfigValue(const char *key, const char *defValue);
    virtual const bool GetConfigBool(const char *key, bool bDefault);
    virtual const int GetConfigInt(const char *key, int iDefault);
    virtual void SetConfigValue(const char *key, const char *value);
    virtual void SetConfigValue(const char *key, const bool value);
    virtual void SetConfigValue(const char *key, const int value);
    
    // TEST
  //void TestObjectFactories();

    // Plugin
    virtual void ScanForPlugins(const char *absPath, bool bRecursive);
    virtual IBaseInstance *RegisterAndInitializePlugin(PFNINITIALIZEPLUGIN pInitializeFunc, const char *name);
    // enumeration function
    virtual void EnumeratePlugins(void *pUser, PFNENUMBASEFUNC pEnumFunc);
    virtual void EnumeratePluginObjects(void *pUser, PFNENUMBASEFUNC pEnumFunc);
    virtual void EnumerateURIHandlers(noice::io::PFNENUMIODEVICE pEnumFunc);

    // hooking
    virtual void RegisterDocumentHook(ISystemDocumentHooks *pHook);
    virtual void UnregisterDocumentHook(ISystemDocumentHooks *pHook);


    virtual yapt::ILogger *GetLogger(const char *loggerName);
  };

  // This class holds mappings available for everybody, everywhere... 
  // don't put everything here
  class Lookup
  {
  protected:
    static PropInstMap propertyInstanceMap;
    static PluginNodeMap nodehash;
    static StrBaseMap strBaseMap;

  public:
    static void RegisterPropInst(Property *prop, IPropertyInstance *pInst);
    static IPropertyInstance *GetPropertyInstance(Property *prop);

    static void RegisterNodeExtInst(IDocNode *pNode, IBaseInstance *pInstance);
    static void DeregisterNodeExtInst(IDocNode *pNode);
    static IDocNode *GetNodeForExtInst(IPluginObject *pObject);

    static void RegisterStrBase(std::string fqname, IBaseInstance *pInst);
    static void DeregisterStrBase(std::string fqname);
    static IBaseInstance *GetBaseFromString(std::string fqname);

  };

  // internal top level functions
  SystemHookHandler *GetHooks();	// returns the global hook handler

};
