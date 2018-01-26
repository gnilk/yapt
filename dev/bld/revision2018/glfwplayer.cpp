#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <vector>
#include <GL/GLEW.h>
#include <GLFW/glfw3.h>


#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"	// only needed for plugin class test
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"

#ifndef WIN32
#include "ConsoleWindow.h"
//#include "WebService.h"
#endif

#include "PlayerWindow.h"

using namespace yapt;

int64_t mint = 0;
//std::vector<int> vmv { 1, 2, 3, 4 };


extern "C" 
{
int CALLCONV InitializeAnimPlayerPlugin(ISystem *ySys);
}

static int window_width = 1280;
static int window_height = 720;

static void perror() {
	kErrorClass eClass;
	kError eCode;
	char estring[256];

	GetYaptLastError(&eClass, &eCode);
	GetYaptErrorTranslation(estring, 256);
	Logger::GetLogger("main")->Error("(%d:%d); %s\n", eClass, eCode, estring);
	exit(1);
}

static void LoadDocument(char *filename) {
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Loading: %s", filename);
	yapt::ISystem *system = GetYaptSystemInstance();
	if (!system->LoadNewDocument(filename)) {
		perror();
		//fprintf(stderr, "Unable to load: %s\n", filename);
		//exit(1);
	}
}
static void testBindParser() {

	// 1) Build a FQ (FQ - fully qualified) cache when post-initializing the document
	// 2) Classify source
	//      a) Contain no '.' - single name, with first output, like today
	//      b) Contain one or more '.' -> find best match according to
	//         1) Last item is output-variable-name, if found make sure right hand side matches
	//         2) Last item is effect-name and we want only first output, find object with matching effect name

	char *sFQObjectName = "doc.render.context.rotspeed.expression";
	char *sSourceName = "rotspeed";

	std::string source(sSourceName);
	std::string fqPropName(sFQObjectName);

	size_t pos = fqPropName.find_last_of('.');
	std::string fqObjectName = fqPropName.substr(0, pos);
	std::string propName = fqPropName.substr(pos + 1);
	// primitive sourceing - just fetch first
	if (source.find('.') == std::string::npos) {
		// not found, source is object name only..
		size_t pos = fqObjectName.find_last_of('.');
		std::string objectName = fqObjectName.substr(pos + 1);
		if (!objectName.compare(source)) {
			// found
			printf("found\n");
		}
	}

}
static void Initialize(int logLevel) {

	// Initialize the logger - set up console
	Logger::Initialize();
	if (logLevel != Logger::kMCNone) {
		Logger::AddSink(Logger::CreateSink("LogConsoleSink"), "console", 0, NULL);
	}
	Logger::SetAllSinkDebugLevel(logLevel);

	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Noice 2018 - Revision - YAPT2 Booting up");
	yapt::ISystem *system = GetYaptSystemInstance();

	system->SetConfigValue(kConfig_CaseSensitive, false);
	system->ScanForPlugins(".\\", true);
	pLogger->Debug("Initializing built in plugin for vector animations");
	InitializeAnimPlayerPlugin(system);
}



static void printHelp(char *execname) {

	printf("Usage: %s [options] <filename>\n", execname);
	printf("Options\n");
	printf(" w Window mode\n");
	printf(" d  Show debug/console window\n");
	printf(" m <num>    Select monitor\n");
	printf(" l <num>    Set log level (higher means less, default is 0)\n");
	printf(" r <file>   Record to movie file (requires ffmpeg)\n");
	printf(" o <class>  Dump info for object of type 'class'\n");
	printf(" s <file>   Save document\n");
	printf("Filename is default 'file://gl_test.xml' (don't forget the URI specifier)\n");
}
static void dumpProperties(IPluginObjectInstance *pInst, bool bOutput) {
	int nProp;
	if (!bOutput) {
		nProp = pInst->GetNumInputProperties();
		if (nProp == 0)
			return;
		printf("Input properties\n");
	} else {
		nProp = pInst->GetNumOutputProperties();
		if (nProp == 0)
			return;
		printf("Output properties\n");
	}
	for (int i = 0; i < nProp; i++) {
		IPropertyInstance *iPropInst = pInst->GetPropertyInstance(i, bOutput);
		IBaseInstance *pBase = dynamic_cast<IBaseInstance *>(iPropInst);
		if (pBase != NULL) {
			char tmp[256];
			char tmp2[256];
			printf("  %i: [%s] %s (default: %s)\n", i,
					iPropInst->GetPropertyTypeName(tmp2, 256),
					pBase->GetAttribute("name")->GetValue(),
					iPropInst->GetValue(tmp, 256));
		}
	}
}
static void printObjectProperties(char *className) {
	yapt::ISystem *ySys = GetYaptSystemInstance();
	IPluginObjectDefinition *pDef = ySys->GetObjectDefinition(className);
	if (pDef == NULL) {

		printf("[!] Error: Object not found '%s'\n", className);
		return;
	}
	IPluginObjectInstance *pInst = pDef->CreateInstance();
	if (pInst == NULL) {

		printf("[!] Error: Unable to create instance of object\n");
		return;
	}
	pInst->GetExtObject()->Initialize(ySys, pInst);

	printf("Object '%s', created ok\n", className);

	dumpProperties(pInst, false);
	dumpProperties(pInst, true);
}
void OnGlfwFramebufferSizeChanged(GLFWwindow* window, int width, int height)
{
	//Logger::GetLogger("main")->Debug("Window Size Change: %d,%d", width, height);	

}
void OnGlfwShouldClose(GLFWwindow* window)
{
	//glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char **argv) {

	int monitor = 0;
	char *saveName = NULL;
	//int logLevel = Logger::kMCDebug;
	int logLevel = Logger::kMCNone;
	char *docFileName = "file://gl_test.xml";
	char *objName = NULL;
	char *movieName = NULL;
	bool showConsoleWindow = false;
	bool fullScreen = true;
	int fullscreen_width = 0;
	int fullscreen_height = 0;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
				switch (argv[i][1]) {
				case 'w' :
					fullScreen = false;
					break;
				case 'x' :
					fullscreen_width = atoi(argv[++i]);
					break;
				case 'y' :
					fullscreen_height = atoi(argv[++i]);
					break;
				case 'm' :
					monitor = atoi (argv[++i]);
					break;
				case 'd' :
					logLevel = Logger::kMCDebug;
					showConsoleWindow = true;
					break;
				case 'l':
					logLevel = atoi(argv[++i]);
					break;
				case 'o':
					objName = argv[++i];
					break;
				case 's' :
					saveName = argv[++i];
					break;
				case 'r' :
					movieName = argv[++i];
					break;
				case 'h':
				case 'H':
					printHelp(argv[0]);
					exit(1);
					break;
				default:
					printf("Unknown option: %s\n", argv[i]);
					printHelp(argv[0]);
					exit(1);
					break;
				}
			} else {
				docFileName = argv[i];
			}

		}
	}

	// Switch off the bloody console window!
	if (showConsoleWindow == true) {
		fullScreen = false;
		showConsoleWindow = false;
	}

	Initialize(logLevel);
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Initialize glfw");

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	} else {
		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);

		pLogger->Info("GLFW Version %d.%d.%d",major,minor,rev);
	}

	// Get window height, can be overridden by the system
	pLogger->Debug("Load document");

	LoadDocument(docFileName);	
#ifndef WIN32
	if (showConsoleWindow) {
//		StartWebService();
	}
#endif
	if (objName != NULL) {
		printObjectProperties(objName);
		exit(1);
	}
	

	yapt::ISystem *system = GetYaptSystemInstance();
	if (fullScreen) {
		window_width = system->GetConfigInt(kConfig_ResolutionWidth,fullscreen_width);  
	  	window_height = system->GetConfigInt(kConfig_ResolutionHeight,fullscreen_height);  		
	} else {
		window_width = system->GetConfigInt(kConfig_ResolutionWidth,1280);  
	  	window_height = system->GetConfigInt(kConfig_ResolutionHeight,720); 
	}

#ifndef WIN32
	ConsoleWindow console;
	if (showConsoleWindow) {
		pLogger->Debug("Opening Console Window");
		console.Open(640, 500, "console");
		console.SetPos(640,0);
		if (!console.InitializeFreeType()) {
			pLogger->Error("Failed to initalize text rendering");
			exit(1);
		}		
	}
#endif
	pLogger->Debug("Opening Rendering Window");
	pLogger->Debug("  Width: %d", window_width);
	pLogger->Debug("  Height: %d", window_height);  	

	PlayerWindow player;
	if (!fullScreen) {
		player.Open(window_width, window_height, "player");
	} else {
		player.SetPreferredMonitor(monitor);
		player.Open(window_width, window_height, "player", true);		
	}
	player.InitalizeYapt();
#ifndef WIN32
	player.SetPos(0,0);
#endif
	pLogger->Debug("Running render loop");

#ifndef WIN32
	if (showConsoleWindow) {
		console.WriteLine("YAPT2 v0.1 - (c) Fredrik Kling 2009");
		console.WriteLine("Document: "+ std::string(docFileName));
		console.WriteLine("Running render loop");		
	}
#endif

	// Need to initialize late if GL plugins are using GLEW - initialized by player window
	if (system->GetActiveDocument()) {
		pLogger->Debug("Initialize");
		if (!system->GetActiveDocumentController()->Initialize()) {
			perror();
		}
	}

	// Can only save document after it has been initalized.
	if (saveName != NULL) {
		pLogger->Info("Saving document to: %s", saveName);
		system->SaveDocumentAs(saveName, system->GetActiveDocument());
	}


	if (system->GetActiveDocument()) {
		system->GetActiveDocumentController()->RenderResources();
	}

	if (movieName != NULL)
	{
		player.EnableRecordToMovie();
	}



	while (!player.ShouldClose())
	{
		player.Update();
#ifndef WIN32
		if (showConsoleWindow) {
			console.Update();
		}
#endif
		glfwPollEvents();
	}

	pLogger->Debug("Cleaning up");

	// Close OpenGL window and terminate GLFW
#ifndef WIN32
	if (showConsoleWindow) {
		console.Close();
	  	//StopWebService();
	}
#endif
	player.Close();
	glfwTerminate();
	system->DisposeActiveDocument();
	pLogger->Debug("Disposed");

	exit(EXIT_SUCCESS);
}







