#include <stdio.h>
#include <stdlib.h>
#include <GL/glfw.h>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"	// only needed for plugin class test
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"

using namespace yapt;


static void loadDocument(char *filename) {
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Loading: %s",filename);
	yapt::ISystem *system = GetYaptSystemInstance();
	if (!system->LoadNewDocument(filename)) {
		
		fprintf(stderr,"Unable to load: %s\n",filename);
		exit(1);
	}
	
	
	if (system->GetActiveDocument())
	{
		pLogger->Debug("PostInitialize");
		IDocNode *pRoot = system->GetActiveDocument()->GetTree();		
		system->GetActiveDocumentController()->PostInitializeNode(pRoot);
		pLogger->Debug("Initialize/Render Resources");
		system->GetActiveDocumentController()->RenderResources();

	}
	
}
static void initializeYapt() {
	
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("YAPT2 - running tests");
	yapt::ISystem *system = GetYaptSystemInstance();
	
	system->SetConfigValue(kConfig_CaseSensitive,false);
	system->ScanForPlugins(".\\", true);
	
//	pLogger->Debug("Initializing built in functionality");
//	IBaseInstance *pPlugin = system->RegisterAndInitializePlugin(yFxInitializePlugin, "built-in");
}

int main(int argc, char **argv) {

	initializeYapt();
	loadDocument("file://gl_test.xml");
	
	
	if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }
	
    // Open a window and create its OpenGL context
    if( !glfwOpenWindow( 640, 480, 0,0,0,0, 0,0, GLFW_WINDOW ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
		
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
	
    glfwSetWindowTitle( "YAPT2 - glfw player" );
	
    // Ensure we can capture the escape key being pressed below
    glfwEnable( GLFW_STICKY_KEYS );
	
    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );
	yapt::ISystem *ySys = GetYaptSystemInstance();
	
    do
    {
		ySys->GetActiveDocumentController()->Render(glfwGetTime());

	      glfwSwapBuffers();
	
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		  glfwGetWindowParam( GLFW_OPENED ) );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	exit( EXIT_SUCCESS );
}
