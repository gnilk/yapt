#pragma once

#include <stack>
#include <string>

#ifdef WIN32
#include "expat/lib/expat.h"
#else
#include <expat.h>
#endif

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "noice/io/io.h"

namespace yapt
{
	typedef enum
	{
		kParserState_Root,
		kParserState_Doc,
		kParserState_Configuration,
		kParserState_Resources,
		kParserState_Render,
		kParserState_Object,
		kParserState_Property,
		kParserState_Include,
		kParserState_Timeline,
		kParserState_Timeline_Execute,

	} kParserState;

	class ExpatXMLParser :
		public IDocumentImporter

	{
	protected:
		ISystem *ySys;
		int depthPtr;
		bool bCaptureCData;
		std::string cdata;
		Property *pCdataProperty;
		IDocument *pDocument;
    IMetaInstance *pRoot;
		ILogger *pLogger;
		std::stack<kParserState> stateStack;
		std::stack<IBaseInstance *> instanceStack;
		XML_Parser parser;
	protected:
		bool IsElementAllowed(const char *name);
		void ParseError(const char *errString);
		IPluginObjectInstance *CreateObjectInstance(const char *name, const char **atts);
		IBaseInstance *CreateExecuteInstance(const char *name, const char **atts);
		void IncludeFromURL(const char *url, const char **atts);

		bool ReadStream(noice::io::IStream *pStream);
	public:
		ExpatXMLParser(ISystem *pSys);
		virtual ~ExpatXMLParser();

		bool ImportFromStream(noice::io::IStream *pStream, bool bCreateNewDocument);
		bool ImportFile(const char *filename, bool bCreateNewDocument);
		IDocument *GetDocument();
    void SetRoot(IMetaInstance *_pRoot) { this->pRoot = _pRoot; }

		// - 
		void doStartElement(const char *name, const char **atts);
		void doEndElement(const char *name);
		void doHandleCharData(const XML_Char *s,int len);
	};
}
