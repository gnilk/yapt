#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <time.h>
#include <sstream>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"  // only needed for plugin class test
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"

#include "WebService.h"
#include "mongcpp.h"

using namespace yapt;
using namespace mongoose;
using namespace std;

template <class T>
inline std::string toString (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

string ipToString(long ip)
{
    string res;
    long workIp, a, b, c, d;
    workIp = ip;
    d = workIp % 0x100;
    workIp = workIp >> 8;
    c = workIp % 0x100;
    workIp = workIp >> 8;
    b = workIp % 0x100;
    workIp = workIp >> 8;
    a = workIp;
    res = toString(a)+"."+toString(b)+"."+toString(c)+"."+toString(d);
    return res;
}


class TestMongoServer: public MongooseServer, IDocumentTraversalSink {
public:
    TestMongoServer(): MongooseServer() {}
    virtual ~TestMongoServer() {}
protected:
    virtual bool handleEvent(ServerHandlingEvent eventCode, MongooseConnection &connection, const MongooseRequest &request, MongooseResponse &response) {
        bool res = false;

        if (eventCode == MG_NEW_REQUEST) {
            if (request.getUri() == string("/info")) {
            	prepareResponse(response);
            	response.addContent(generateInfoContent(request));
                res = true;
            } else
            if (request.getUri() == string("/plugins")) {
				ILogger *pLogger = Logger::GetLogger("webservice");
				pLogger->Debug("------- RESOLVING REQUEST FOR PLUGINS ---------");
				std::string namearg;
				if (request.getVar("name", namearg)) {
					pLogger->Debug("ARGUMENT name=%s",namearg.c_str());
				}


            	prepareResponse(response);
            	response.addContent(listPlugins());
                res = true;
            }

            if (res) {
           		response.write();
            }

        }

        return res;
    }

    // ITraversal
    string nodeinfo;
    void OnNode(IDocNode *node, int depth) {
		  IBaseInstance *pObject = node->GetNodeObject();
		  nodeinfo += "<br />" + std::string(pObject->GetFullyQualifiedName());
          // How to extract a property value from a specifically named node
            if (pObject->GetInstanceType() == kInstanceType_Property) {
                IPropertyInstance *pProp = dynamic_cast<IPropertyInstance *>(pObject);
                if(pProp != NULL) {
                    char tmp[256];
                    pProp->GetValue(tmp,256);
                    nodeinfo += " (" + std::string(tmp) + ")";
                    if (pProp->IsSourced()) {
                        nodeinfo += " [S]->" + std::string(pProp->GetSourceString());          
                    }

                }

          }
    }

    void prepareResponse(MongooseResponse &response) {
        response.setStatus(200);
        response.setConnectionAlive(false);
        response.setCacheDisabled();
        response.setContentType("text/html");    	
    }
    void handleInfo(const MongooseRequest &request, MongooseResponse &response) {
        response.setStatus(200);
        response.setConnectionAlive(false);
        response.setCacheDisabled();
        response.setContentType("text/html");
        //response.addContent(reply);
        response.write();
    }

    const string generateInfoContent(const MongooseRequest &request) {
        string result;
        result = "<h1>Sample Info Page</h1>";
        result += "<br />Request URI: " + request.getUri();
        result += "<br />Your IP: " + ipToString(request.getRemoteIp());

	time_t tim;
	time(&tim);

        result += "<br />Current date & time: " + toString(ctime(&tim));
        result += "<br /><br /><a href=\"/\">Index page</a>";
        result += generateYaptInfo();
        return result;
    }

    static int glbSysEnumPluginsCallback(void *pThis, IBaseInstance *pInst) {
    	TestMongoServer *pServer = (TestMongoServer *)pThis;
    	pServer->OnPluginEnum(pInst);
    }

	string str_plugins;
	 const std::string ListObjectProperties(IPluginObjectInstance *pInst, bool bOutput) 
	 {
	 	std::string info = "";
		int nProp;
		if (!bOutput) {
			nProp = pInst->GetNumInputProperties();
			if (nProp == 0)
				return info;
			info += "<br /><b>Input properties</b>";
		} else {
			nProp = pInst->GetNumOutputProperties();
			if (nProp == 0)
				return info;
			info += "<br /><b>Output properties</b>";
		}

		for (int i = 0; i < nProp; i++) {
			IPropertyInstance *iPropInst = pInst->GetPropertyInstance(i, bOutput);
			IBaseInstance *pBase = dynamic_cast<IBaseInstance *>(iPropInst);
			if (pBase != NULL) {
				char tmp[256];
				char tmp2[256];
				char tmp3[256];

				sprintf(tmp3,"  %i: [%s] %s (default: %s)\n", i,
						iPropInst->GetPropertyTypeName(tmp2, 256),
						pBase->GetAttribute("name")->GetValue(),
						iPropInst->GetValue(tmp, 256));
				info += "<br />" + std::string(tmp3);
			}
		}
		return info;
	}

    void OnPluginEnum(IBaseInstance *pInst) {

		ILogger *pLogger = Logger::GetLogger("webservice");
		const char *className = pInst->GetAttributeValue("name");
		pLogger->Debug("Resolving info for %s",className);


    	str_plugins += "<br/><b>" + std::string(className) + "</b>";

    	yapt::ISystem *system = GetYaptSystemInstance();

    	//system->GetObjectDefinition(pInst->GetAttributeValue("name"));
    	IPluginObjectDefinition *pDef = system->GetObjectDefinition(className);
		if (pDef == NULL) {
			pLogger->Error("Object not found '%s'\n", className);
			str_plugins += "<br />" + std::string("Error: Object definition not found!");
			return;
		}
		IPluginObjectInstance *pPlugInst = pDef->CreateInstance();
		if (pPlugInst == NULL) {
			pLogger->Error("Can't create instance for %s", className);
			str_plugins += "<br />" + std::string("[!] Error: Unable to create instance of object");	
			return;
		}
		//pLogger->Debug("Instance, %p - ok - initalize external object",pPlugInst);
		pPlugInst->GetExtObject()->Initialize(system, pPlugInst);


		str_plugins += ListObjectProperties(pPlugInst, false);
		str_plugins += ListObjectProperties(pPlugInst, true);
    }
    const string listPlugins() {
    	str_plugins = "";
    	yapt::ISystem *system = GetYaptSystemInstance();
    	system->EnumeratePluginObjects((void *)this, glbSysEnumPluginsCallback);
    	//system->EnumeratePlugins((void *)this, glbSysEnumPluginsCallback);
    	str_plugins += "";
    	return str_plugins;
    }

	
    const string generateYaptInfo() {
    	nodeinfo = "";
    	yapt::ISystem *system = GetYaptSystemInstance();
		system->GetActiveDocumentController()->TraverseDocument(this);
		return nodeinfo;
    }
};

TestMongoServer server;
void StartWebService()
{
    server.setOption("document_root", "html");
    server.setOption("listening_ports", "8080");
    server.setOption("num_threads", "5");

    server.start();
}

void StopWebService()
{
	server.stop();
}
