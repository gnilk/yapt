#include <stdio.h>
#include <stdlib.h>
#include <GL/glfw.h>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"	// only needed for plugin class test
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"

using namespace yapt;

static void perror() {
	kErrorClass eClass;
	kError eCode;
	char estring[256];

	GetYaptLastError(&eClass, &eCode);
	GetYaptErrorTranslation(estring, 256);
	Logger::GetLogger("main")->Error("(%d:%d); %s\n", eClass, eCode, estring);
	exit(1);
}

static void loadDocument(char *filename) {
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Loading: %s", filename);
	yapt::ISystem *system = GetYaptSystemInstance();
	if (!system->LoadNewDocument(filename)) {

		fprintf(stderr, "Unable to load: %s\n", filename);
		exit(1);
	}

	if (system->GetActiveDocument()) {
		pLogger->Debug("PostInitialize");
		IDocNode *pRoot = system->GetActiveDocument()->GetTree();
		if (!system->GetActiveDocumentController()->PostInitializeNode(pRoot)) {
			perror();
		}
		pLogger->Debug("Initialize/Render Resources");
		system->GetActiveDocumentController()->RenderResources();

	}
}
static void testBindParser() {

	// 1) Build a FQ cache when post-initializing the document
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
static void initializeYapt() {

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
	printf(
			"Filename is default 'file://gl_text.xml' (don't forget the URI specifier)\n");
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

int main(int argc, char **argv) {

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

	Logger::SetAllSinkDebugLevel(logLevel);
	initializeYapt();
	if (objName != NULL) {
		printObjectProperties(objName);
		exit(1);
	}
	loadDocument(docFileName);

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Open a window and create its OpenGL context
	if (!glfwOpenWindow(640, 480, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		fprintf(stderr, "Failed to open GLFW window\n");

		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowTitle("YAPT2 - glfw player");

	// Ensure we can capture the escape key being pressed below
	glfwEnable(GLFW_STICKY_KEYS);

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval(1);
	yapt::ISystem *ySys = GetYaptSystemInstance();

	do {
		ySys->GetActiveDocumentController()->Render(glfwGetTime());
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS
			&& glfwGetWindowParam(GLFW_OPENED));

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
