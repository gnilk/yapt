/*-------------------------------------------------------------------------
File    : $Archive: PluginScanner_Win32.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Defines all the external interfaces available in the system

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Split this file in order of usage (UI, Plugins, others)
</pre>


\History
- 21.09.09, FKling, Definitions of IO Stream return codes, changed return of "Size" for IStream
- 17.09.09, FKling, Implementation

---------------------------------------------------------------------------*/

#pragma once

#include "yapt/logger.h"
#include <vector>

#include "noice/io/io.h"
using namespace noice::io;

namespace yapt
{
  // Forward declare a few interfaces
  struct ISystem;
  struct IBaseInstance;
  struct IDocument;
  struct IDocNode;
  struct IDocumentController;
  struct IPluginObjectInstance;
  struct IContext;


#ifdef WIN32
#define CALLCONV __stdcall
#undef GetObject
#else
#define CALLCONV
#endif

  extern "C"
  {
    typedef int (CALLCONV *PFNINITIALIZEPLUGIN)(ISystem *pSys);
    typedef int (CALLCONV *PFNENUMBASEFUNC)(void *, IBaseInstance *pInst);
  }

  #define kDefintion_Name ("name")

  #define kConfig_CaseSensitive ("yapt.casesensitive")

  #define kConfig_ResolutionWidth ("screen.resolution.width")
  #define kConfig_ResolutionHeight ("screen.resolution.height")

  // document tags
//	#define kDocument_RootName ("yapt2")
//#define kDocu
  #define kDocument_RootTagName ("yapt2")
  #define kDocument_ResourceTagName ("resources")
  #define kDocument_RenderTagName ("render")
  #define kDocument_ObjectTagName ("object")
  #define kDocument_PropertyTagName ("property")
  #define kDocument_IncludeTagName ("include")
  #define kDocument_TimelineTagName ("timeline")
  #define kDocument_ExecuteTagName ("execute")
  #define kDocument_SignalsTagName ("signals")
  #define kDocument_SignalChannelTagName ("channel")
  #define kDocument_SignalTagName ("signal")


  // defines the module/logical unit of where the error occured
  typedef enum
  {
    kErrorClass_General = 0,
    kErrorClass_ObjectDefinition,
    kErrorClass_Import,
    kErrorClass_StreamIO,
  } kErrorClass;
  
  // Error code, always coupled with the error class
  typedef enum
  {
    kError_NoError = 0,
    kError_Unknown,

    // general
    kError_NoDocument,	// No document assigned to system instance
    kError_NoChildOfType,	// Failed to look up node child of specific type
    kError_NoInstance,	// Instance not attached to node
    kError_URLParseSyntax,
    kError_CreateStreamForURI,
    kError_StreamOpenForURI,

    // object definition
    kError_TokenSyntax,	// Descriptor token has a syntax error, normally no '='
    kError_DescriptorSyntax,	// Descriptor string available, but contains no tokens, syntax error
    kError_MissingIdentifier,	// the "identifier" token was not found in the descriptor string
    kError_NoFactory,	// Factory was NULL when trying to create descriptor
    kError_NoDescriptor,	// Descriptor was NULL
    kError_FactoryFailed,	// failed in factory when creating instance
        kError_PropertyBindFailed,  // Failed to bind a property
    
    // no object matching the given class was found during XML import
    // a node look up failed for the object
    kError_ObjectNotFound,	
    // IO
    kError_StreamReadError,	// fread returned an error
    kError_StreamNotOpen,
    kError_StreamOpenError,

  } kError;

  typedef enum
  {
    kPropertyType_Unknown = 0,
    kPropertyType_Color,
    kPropertyType_Integer,
    kPropertyType_IntegerTuple,
    kPropertyType_Float,
    kPropertyType_FloatTuple,
    kPropertyType_Vector,
    kPropertyType_Quaternion,
    kPropertyType_String,
    kPropertyType_UserPtr,
    kPropertyType_Enum,
    kPropertyType_Bool,
    kPropertyType_FloatArray,
    kPropertyType_Unbound,  // property created first through XML but not yet attached by plugin (after initialization all object properties which are unbound are declared wrong)
  } kPropertyType;

  typedef enum
  {
    kInstanceType_Unknown = 0,
    kInstanceType_Property = 1,
    kInstanceType_Document = 2,
    kInstanceType_Object = 3,
    kInstanceType_ObjectDefinition = 4,
    kInstanceType_ResourceContainer = 5,
    kInstanceType_RenderNode = 6,
    kInstanceType_PluginContainer = 7,
    kInstanceType_URLFactory = 8,
    kInstanceType_MetaNode = 9,
    kInstanceType_Timeline = 10,
    kInstanceType_TimelineExecute = 11,
    kInstanceType_Signals = 12,
    kInstanceType_SignalChannel = 13,
    kInstanceType_Signal = 14,
    kInstanceType_Comment = 15,
  } kInstanceType;

  typedef enum
  {
    kNodeType_Unknown = 0,
    kNodeType_Any = 0,
    kNodeType_Regular = 1,
    kNodeType_ObjectInstance = 2,
    kNodeType_PropertyInstance = 3,
    kNodeType_RenderNode = 4,
    kNodeType_ResourceContainer = 5,
    kNodeType_Document = 6,
    kNodeType_XMLRAW = 7,	// RAW XML, should dump RAW during serialization (comments and unknown tokens etc)
    kNodeType_Meta = 8,	// This is a meta node
    kNodeType_Include = 9,
    kNodeType_Timeline = 10,
    kNodeType_Signals = 11,
    kNodeType_Comment = 12,    
  } kNodeType;

  typedef enum  
  {  
    kPropertyHint_None = 0,
    kPropertyHint_File,
  } kPropertyHint;

  typedef enum {
    kPluginInterfaceId_Core = 0,    // this is the base interface
    kPluingInterfaceId_UI = 1,      // UI interaction interface
  } kPluginInterfaceIdentifier;

  union PropertyValue
  {
    float matrix[16];
    float rgba[4];
    int boolean;
    int int_val;
    int int_tuple[2];
    float float_val;
    float float_tuple[2];
    float vector[3];
    float quaternion[4];
    float angleaxis[4];
    char *string;
    void *userdata;
    struct
    {
      int size;
      float *items;
    } float_array;
  };

  
  struct Property
  {
    kPropertyType type;
    char *infostring;	
    PropertyValue *v;
  };

  // This interface must be implemented by the external objects
  struct IPluginObject
  {
  public:
    // Called once after the constructor has been called
    // You should create your properties in this call
    virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) = 0;

    // Called anytime after initialized when the render tree has changed
    // is called bottom up (i.e. first all children) an object can use this
    // in order to cache data from its children. If any data is changed the
    // whole tree is normally post initialized. Resources need not to care about
    // this function.
    virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) = 0;

    // Called when the object should render it self, can be called any time after
    // the object has been initialized (this is a tree down stream call)
    virtual void Render(double t, IPluginObjectInstance *pInstance) = 0;

    // Called after all childs of the object has rendered them selfs
    // on the way back from the recursive rendering. Objects implementing
    // rendering surfaces have a change to store texture data here
    // Render states can be restored, otherwise no rendering should
    // be done during PostRender
    virtual void PostRender(double t, IPluginObjectInstance *pInstance) = 0;

    // Called when a signal was raised in a channel
    virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) = 0;

    // Called by the system to check if the plugin wan't it interact with additional parts of the system
    // other than the main rendering part
    // NULL as return means that the interface is unsupported    
    //virtual void *QueryInterface(kPluginInterfaceIdentifier interfaceIdentifier) = 0;
  };

  // Interface for UI callback's
  struct IPluginUi {
  public:
    virtual void MouseMove();
    virtual void MouseButtonPress();
    virtual int HitTest();
  };

  // The factory must be implemented and registered to the system
  struct IPluginObjectFactory
  {
  public:
    virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier) = 0;
  };
  
  struct IPropertyInstance
  {
  public:
    virtual kPropertyType GetPropertyType() = 0;
    virtual char *GetPropertyTypeName(char *sDest, int maxLen) = 0;
    virtual void SetValue(const char *sValue) = 0;
    virtual char *GetValue(char *sValueDest, int maxlen) = 0;
    virtual std::vector<std::string> GetValidEnumValues() = 0;
//        virtual char **GetValidEnumValues(int *outNumValues) = 0;
    virtual bool IsSourced() = 0;
    virtual bool IsOutputProperty() = 0;
    virtual char *GetSourceString() = 0;  
    virtual IPropertyInstance *GetSource() = 0;
    virtual IBaseInstance *GetObjectInstance() = 0;

  };
  
  struct IPluginObjectDefinition
  {
  public:
    virtual IPluginObjectInstance *CreateInstance() = 0;
    virtual IPluginObjectFactory *GetFactory() = 0;
  };

  struct IPluginObjectInstance
  {
  public:
    virtual IPluginObject *GetExtObject() = 0;
    virtual Property *CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription) = 0;
    virtual Property *CreateOutputProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription) = 0;

    virtual bool BindVariable(const char *sName, kPropertyType type, const char *sDescription) = 0;

    virtual void SetPropertyHint(const char *sName, kPropertyHint hint) = 0;
    virtual kPropertyHint GetPropertyHint(const char *sName) = 0;
    virtual Property *GetProperty(const char *name) = 0;
    virtual IPropertyInstance *GetPropertyInstance(const char *name) = 0;
    virtual IPropertyInstance *GetPropertyInstance(int index, bool bOutput) = 0;
    virtual void SetPropertyValue(Property *prop, const char *value) = 0;
    virtual int GetNumInputProperties() = 0;
    virtual int GetNumOutputProperties() = 0;
    virtual Property *GetInputPropertyAt(int index) = 0;
    virtual Property *GetOutputPropertyAt(int index) = 0;

    virtual const char *GetClassName() = 0;
    virtual const char *GetInstanceName() = 0;

    virtual IDocument *GetDocument() = 0;
    virtual IDocNode *GetDocumentNode() = 0;
    virtual ILogger *GetLogger() = 0;
    //virtual int GetNumChildren(kNodeType ofType) = 0;
    //virtual IPluginObjectInstance *GetChildAt(int index, kNodeType ofType) = 0;
  };

  struct IRenderContextParams
  {
  public:
    virtual int GetFrameBufferWidth() = 0;
    virtual int GetFrameBufferHeight() = 0;
    virtual int GetWindowWidth() = 0;
    virtual int GetWindowHeight() = 0;
  };

  struct IContext
  {
  public:
    virtual IDocument *GetDocument() = 0;
    virtual void SetDocument(IDocument *pDocument) = 0;
    virtual IDocumentController *GetDocumentController() = 0;
    virtual void SetObject(const char *name, void *pObject) = 0;
    virtual void *GetObject(const char *name) = 0;

    virtual void PushRenderObject(IBaseInstance *) = 0;
    virtual void PopRenderObject() = 0;
    virtual IBaseInstance *TopRenderObject() = 0;

    virtual void PushContextParamObject(void *pObject, const char *name) = 0;
    virtual void PopContextParamObject() = 0;
    virtual void *TopContextParamObject() = 0;

    virtual void SetNamePrefix(const char *prefix) = 0;
    virtual char *GetNamePrefix(char *pdest, int nmaxlen) = 0;
    virtual char *CreatePrefixName(const char *name, char *prefixedname, int nmaxlen) = 0;

    virtual void AddNode(IBaseInstance *instance, IDocNode *node) = 0;
    virtual IDocNode *FindNode(IBaseInstance *pObject) = 0;
    virtual void EraseNode(IBaseInstance *instance) = 0;

  };

  struct IRenderVars
  {
  public:
    // returns an object local time since start of rendering
    // basically this is the global_time - object_start_time
    virtual double GetLocalTime() = 0;
    virtual double GetTime() = 0;
    // this allows plugins to create their own timers for various purposes
    virtual double GetTimer(unsigned int idTimer) = 0;
    virtual void CreateTimer(unsigned int idTimer, unsigned int flags) = 0;
    virtual void UpdateTimer(unsigned int idTimer, double newTime) = 0;
  };

  struct ITimer
  {
  public:
    virtual double GetTime() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
  };

  struct ISignal {
  public:
    virtual double GetTime() = 0;
    virtual int GetValue() = 0;
  };

  struct ISignalChannel {
  public:
    virtual int GetId() = 0;
    virtual const char *GetName() = 0;
    virtual void SetName(const char *name) = 0;
    virtual int GetNumSignals() = 0;
    virtual ISignal *GetSignalAt(int idx) = 0;
    virtual ISignal *AddSignal(double t, int sigvalue) = 0;
    virtual void RemoveSignal(int idx) = 0;
    virtual ISignal *RaiseNext(double t) = 0;
    virtual void RegisterSinkObject(IPluginObjectInstance *sink) = 0;
    virtual void SendToSinks(ISignal *signal) = 0;
  };

  struct ISignals {
  public:
    virtual int GetNumChannels() = 0;
    virtual ISignalChannel *GetChannel(int idx) = 0;
    virtual ISignalChannel *GetChannel(const char *name) = 0;
    virtual ISignalChannel *AddChannel(int id, const char *name, double accuracy) = 0;
    virtual void RemoveChannel(int idChannel) = 0;
    virtual void RemoveChannel(const char *name) = 0;
  };

  struct IResourceContainer
  {
  public:
    virtual bool Initialize() = 0;
    virtual bool InitializeAllResources() = 0;
    virtual IPluginObjectInstance *GetResourceInstance(const char *name) = 0;
    virtual void AddResourceInstance(IPluginObjectInstance *pInstance) = 0;
    virtual void *GetIOHandler() = 0;
    virtual noice::io::IStream *OpenStream(const char *sStreamName, unsigned int flags) = 0;

  };

  struct IAttribute
  {
  public:
    virtual const char *GetName() = 0;
    virtual const char *GetValue() = 0;
    virtual void SetValue(const char *value) = 0;
  };

  struct IBaseInstance
  {
  public:
    virtual kInstanceType GetInstanceType() = 0;
    virtual const char *GetFullyQualifiedName() = 0;
    virtual int GetNumAttributes() = 0;
    virtual IAttribute *GetAttribute(int index) = 0;
    virtual IAttribute *GetAttribute(const char *name) = 0;
    virtual const char *GetAttributeValue(const char *name) = 0;
    virtual IAttribute *AddAttribute(const char *name, const char *value) = 0;
    virtual IAttribute *UpdateAttribute(const char *name, const char *value) = 0;
    virtual IContext *GetContext() = 0;
  };

  struct ITimelineExecute {
    virtual float GetStart() = 0;
    virtual float GetDuration() = 0;
    virtual char *GetObjectName() = 0;
    virtual bool ShouldRender(double t) = 0;
  };
  struct ITimeline {
    virtual ITimelineExecute *AddExecuteObject(float start, float duration, char *objectName) = 0;
    virtual int GetNumExecutors() = 0;
    virtual IBaseInstance *GetExecutorAtIndex(int idx) = 0;
  };

  struct IMetaInstance
  {
  public:
    virtual IDocument *GetDocument() = 0;
  };

  struct ICommentInstance
  {
  public:
    virtual char *GetComment() = 0;
    virtual void SetComment(char *) = 0;
  };

   
  struct IDocNode
  {
  public:
    virtual bool Dispose() = 0;	// deletes the node
    virtual kNodeType GetNodeType() = 0;
    virtual IDocNode *GetParent() = 0;
    virtual int GetNumChildren() = 0;
    virtual int GetNumChildren(kNodeType ofType) = 0;
    virtual void RemoveChild(IDocNode *child) = 0;
    virtual IDocNode *GetChildAt(int idx) = 0;
    virtual IDocNode *GetChildAt(int idx, kNodeType ofType) = 0;
    virtual void AddChild(IDocNode *child) = 0;
    virtual IDocument *GetRootDocument() = 0;		
    virtual IBaseInstance *GetNodeObject() = 0;
    virtual void SetNodeObject(IBaseInstance *pObject, kNodeType type) = 0;
  };

  struct IDocumentTraversalSink
  {
  public:
    virtual void OnNode(IDocNode *node, int depth) = 0;
  };

  struct IDocumentController
  {
  public:
    virtual IRenderVars *GetRenderVars() = 0;

    virtual void SetDirty(IDocNode *pNode) = 0;
    
    virtual bool Initialize() = 0;
    virtual void InitializeNode(IDocNode *node) = 0;
    virtual bool PostInitializeNode(IDocNode *node) = 0;
    
    virtual void TraverseDocument(IDocumentTraversalSink *sink) = 0;

    virtual void Render(double sample_time) = 0;
    virtual void RenderResources() = 0;
    virtual void RenderNode(IDocNode *node, bool bForce) = 0;
    
  };

  struct IDocument
  {
  public:
    virtual void Write(noice::io::IStream *stream) = 0;
    virtual void Read(noice::io::IStream *stream) = 0;
    
    virtual IDocNode *GetTree() = 0;
    virtual IDocNode *GetRenderTree() = 0;
    virtual IBaseInstance *GetRenderRoot() = 0;
    virtual IResourceContainer *GetResources() = 0;	// returns root of resource tree
    virtual ITimeline *GetTimeline() = 0;
    virtual ISignals *GetSignals() = 0;
    virtual bool HasTimeline() = 0;

    virtual void MoveNode (IDocNode *pNewParent, IDocNode *pNode) = 0;
    virtual void SwapNodes (IDocNode *pNodeA, IDocNode *pNodeB) = 0;
    virtual bool RemoveNode(IDocNode *pNode) = 0;

    virtual bool RemoveObject(IBaseInstance *pObject) = 0;
    virtual IDocNode *AddObject(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType) = 0;
    virtual IDocNode *AddRenderObject(IBaseInstance *parent, IBaseInstance *object) = 0;
    virtual IDocNode *AddToTimeline(IBaseInstance *object) = 0;
    virtual IDocNode *AddSignalChannel(ISignalChannel *object) = 0;
    virtual void AddResourceObject(IBaseInstance *parent, IBaseInstance *object) = 0;
    virtual IDocNode *AddMetaObject(IBaseInstance *parent) = 0;
    virtual IDocNode *AddCommentObject(IBaseInstance *parent) = 0;

    virtual IBaseInstance *SearchFromNode(IDocNode *pRootNode, const char *name) = 0;
    virtual IBaseInstance *GetObjectFromSimpleName(const char *name) = 0;

    virtual IBaseInstance *GetObject(const char *fullQualifiedName) = 0;
    virtual IDocNode *FindNode(IBaseInstance *pObject) = 0;

    virtual void DumpRenderTree() = 0;
    // traverse functions with user object input
    virtual int GetNumChildren(IPluginObjectInstance *pObject) = 0;
    virtual int GetNumChildren(IPluginObjectInstance *pObject, kNodeType ofType) = 0;
    virtual IPluginObjectInstance* GetChildAt(IPluginObjectInstance *pObject, int index, kNodeType ofType) = 0;
    virtual IPluginObjectInstance* GetChildAt(IPluginObjectInstance *pObject, int index) = 0;
  };
  
  struct IDocumentImporter
  {
  public:
    virtual bool ImportFromStream(noice::io::IStream *pStream, bool bCreateNewDocument) = 0;
    virtual bool ImportFile(const char *filename, bool bCreateNewDocument) = 0;
    virtual IDocument *GetDocument() = 0;
  };

  // --TODO: Define better hook interfaces we want - move to own definition file
  //  clarify function parameters. Allow for abortion

  // Object hooks are used to track Object Instance calls
  // should not be set unless debugging, or at least be disabled during rendering
  struct ISystemObjectHooks
  {
  public:
    // external instance hooks
    virtual void BeginObjectInitialize(IBaseInstance *pInstance) = 0;
    virtual void EndObjectInitialize(IBaseInstance *pInstance) = 0;
    virtual void BeginObjectPostInitialize(IBaseInstance *pInstance) = 0;
    virtual void EndObjectPostInitialize(IBaseInstance *pInstance) = 0;
    virtual void BeginObjectRender(IBaseInstance *pInstance) = 0;
    virtual void EndObjectRender(IBaseInstance *pInstance) = 0;
    virtual void BeginObjectPostRender(IBaseInstance *pInstance) = 0;
    virtual void EndObjectPostRender(IBaseInstance *pInstance) = 0;
    // other hooks
    //virtual void PropertyCreated(IBaseInstance *pInstance) = 0;
    //virtual void ObjectDefinitionRegistered(IBaseInstance *pDefinition) = 0;
  };

  struct IFileWatcher
  {
  public:
    virtual void OnFileChanged(const char *filename) = 0;
    //virtual void OnFileRemoved(const char *filename) = 0;
  };

  // provides system hook interface - must implement all
  struct ISystemDocumentHooks
  {
  public:
    // Document import is always synchronous
    virtual void BeginDocumentImport() = 0;
    virtual void DocumentImportProgress(double progress) = 0;
    virtual void EndDocumentImport() = 0;

    // always synchronous
    virtual void BeginDocumentRender() = 0;
    virtual void EndDocumentRender() = 0;

    // can be async, use URL to track which resource call belongs to
    virtual void BeginResourceImport(const char *url) = 0;
    virtual void ResourceImportProgress(const char *url, double progress) = 0;
    virtual void EndResourceImport(const char *url) = 0;

    // error handler
    virtual void OnError(kErrorClass eClass, kError eCode) = 0;

  };

  struct ISystem
  {
  public:
    // Global data routines
    virtual IDocument *CreateNewDocument(bool bSetActive) = 0;
    virtual void DisposeActiveDocument() = 0;
    virtual IDocument *GetActiveDocument() = 0;
    virtual IDocument *LoadNewDocument(const char *url) = 0;
    virtual void SaveDocumentAs(const char *url, IDocument *pDoc) = 0;
    virtual IDocumentController *GetActiveDocumentController() = 0;
    virtual void SetActiveDocument(IDocument *pDocument) = 0;
    virtual IContext *GetCurrentContext() = 0;


    virtual bool RegisterObject(IPluginObjectFactory *factory, const char *sDescription) = 0;
    virtual IPluginObjectDefinition *GetObjectDefinition(const char *name) = 0;

    // Lookup and translation routine
    virtual IBaseInstance *GetControllerObject(IPluginObject *pObject) = 0;
    virtual IBaseInstance *GetControllerObject(const char *name) = 0;

    // --> back to global
    // Stream management interface
    virtual void WatchFile(const char *filename, IFileWatcher *callback) = 0;
    virtual void RemoveFileWatcher(const char *filename) = 0;
    virtual void *LoadData(const char *filename, unsigned int flags, long *outszdata) = 0;
    virtual noice::io::IStream *CreateStream(const char *uri, unsigned int flags) = 0;
    virtual noice::io::IStreamDevice *GetIODevice(const char *url_identifier) = 0;
    virtual void SetIODevice(noice::io::IStreamDevice *pDevice, const char *url_identifier) = 0;
    virtual bool RegisterIODevice(noice::io::IStreamDeviceFactory *pDeviceFactory, const char *url_identifier, const char *initparam, bool bCreate) = 0;
    virtual noice::io::IStreamDevice *CreateIODevice(const char *url_identifier) = 0;


    // Configuration options - ok, global funcs, promote to own interface?
    virtual const char *GetConfigValue(const char *key) = 0;
    virtual const bool GetConfigBool(const char *key) = 0;
    virtual const int GetConfigInt(const char *key) = 0;
    virtual void SetConfigValue(const char *key, const char *value) = 0;
    virtual void SetConfigValue(const char *key, const bool value) = 0;
    virtual void SetConfigValue(const char *key, const int value) = 0;
    virtual const char *GetConfigValue(const char *key, const char *defValue) = 0;
    virtual const bool GetConfigBool(const char *key, bool bDefault) = 0;
    virtual const int GetConfigInt(const char *key, int iDefault) = 0;
    
    // Plugin handling - ok, global stuff
    virtual void ScanForPlugins(const char *absPath, bool bRecursive) = 0;
    virtual IBaseInstance *RegisterAndInitializePlugin(PFNINITIALIZEPLUGIN pInitializeFunc, const char *name) = 0;

    // enumerators - ok, global stuff
    virtual void EnumeratePlugins(void *pUser, PFNENUMBASEFUNC pEnumFunc) = 0;
    virtual void EnumeratePluginObjects(void *pUser, PFNENUMBASEFUNC pEnumFunc) = 0;
    virtual void EnumerateURIHandlers(noice::io::PFNENUMIODEVICE pEnumFunc) = 0;

    // hooking functions - ok, global stuff
    virtual void RegisterDocumentHook(ISystemDocumentHooks *pHook) = 0;
    virtual void UnregisterDocumentHook(ISystemDocumentHooks *pHook) = 0;

    virtual yapt::ILogger *GetLogger(const char *loggerName) = 0;
  };

  // TODO: Also provide a class wrapper for this
  extern "C"
  {
    ISystem * CALLCONV GetYaptSystemInstance();
    void CALLCONV DisposeYaptSystemInstance();
    void CALLCONV SetYaptLastError(kErrorClass eClass, kError eCode);
    void CALLCONV GetYaptLastError(kErrorClass *eClass, kError *eCode);
    char * CALLCONV GetYaptErrorTranslation(char *dst, int maxlen);
  }

};
