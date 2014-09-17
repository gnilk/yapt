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

#include "ConsoleWindow.h"
#include "PlayerWindow.h"
#include "WebService.h"

using namespace yapt;

int64_t mint = 0;
std::vector<int> vmv { 1, 2, 3, 4 };


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
	Logger::AddSink(Logger::CreateSink("LogConsoleSink"), "console", 0, NULL);
	Logger::SetAllSinkDebugLevel(logLevel);

	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("YAPT2 - running tests");
	yapt::ISystem *system = GetYaptSystemInstance();

	system->SetConfigValue(kConfig_CaseSensitive, false);
	system->ScanForPlugins(".\\", true);
}

static void printHelp(char *execname) {

	printf("Usage: %s [options] <filename>\n", execname);
	printf("Options\n");
	printf(" l <num>    Set log level (higher means less, default is 0)\n");
	printf(" o <class>  Dump info for object of type 'class'\n");
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

	char *saveName = NULL;
	int logLevel = Logger::kMCDebug;
	char *docFileName = "file://gl_test.xml";
	char *objName = NULL;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
				switch (argv[i][1]) {
				case 'l':
					logLevel = atoi(argv[++i]);
					break;
				case 'o':
					objName = argv[++i];
					break;
				case 's' :
					saveName = argv[++i];
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

	Initialize(logLevel);
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Initialize glfw");

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Get window height, can be overridden by the system
	pLogger->Debug("Load document");

	LoadDocument(docFileName);	
	StartWebService();
	if (objName != NULL) {
		printObjectProperties(objName);
		exit(1);
	}
	

	yapt::ISystem *system = GetYaptSystemInstance();
	window_width = system->GetConfigInt(kConfig_ResolutionWidth,640);  
  	window_height = system->GetConfigInt(kConfig_ResolutionHeight,360);  

	if (saveName != NULL) {
		pLogger->Info("Saving document to: %s", saveName);
		system->SaveDocumentAs(saveName, system->GetActiveDocument());
	}


	pLogger->Debug("Opening Console Window");
	ConsoleWindow console;
	console.Open(640, 500, "console");
	console.SetPos(640,0);
	if (!console.InitializeFreeType()) {
		pLogger->Error("Failed to initalize text rendering");
		exit(1);
	}

	pLogger->Debug("Opening Rendering Window");
	pLogger->Debug("  Width: %d", window_width);
	pLogger->Debug("  Height: %d", window_height);  	

	PlayerWindow player;
	player.Open(window_width, window_height, "player");
	player.InitalizeYapt();
	player.SetPos(0,0);
	pLogger->Debug("Running render loop");

	console.WriteLine("YAPT2 v0.1 - (c) Fredrik Kling 2009");
	console.WriteLine("Document: "+ std::string(docFileName));
	console.WriteLine("Running render loop");

	// Need to initialize late if GL plugins are using GLEW - initialized by player window
	if (system->GetActiveDocument()) {
		pLogger->Debug("Initialize");
		if (!system->GetActiveDocumentController()->Initialize()) {
			perror();
		}
	}

	if (system->GetActiveDocument()) {
		system->GetActiveDocumentController()->RenderResources();
	}



	while (!player.ShouldClose())
	{
		player.Update();
		console.Update();
		glfwPollEvents();
	}

	pLogger->Debug("Cleaning up");

	// Close OpenGL window and terminate GLFW
	console.Close();
	player.Close();
	glfwTerminate();
	StopWebService();
	system->DisposeActiveDocument();
	pLogger->Debug("Disposed");

	exit(EXIT_SUCCESS);
}


