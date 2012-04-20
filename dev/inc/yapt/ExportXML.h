#pragma once

#include <stack>
#include <string>

#include "yapt/logger.h"
#include "yapt/ySystem.h"

namespace yapt
{
	class ExportXML
	{
	protected:
		ILogger *pLogger;
		std::stack<std::string> tagStack;
		std::stack<bool> tagNewLineStack;
		IDocument *pDoc;
		IStream *pStream;
		int iIndent;
		std::string sIndent;

		void PushIndent();
		void PopIndent();
		void CreateIndentString();

		void Begin(const char *tagName, IBaseInstance *pBase, bool bNewLine);
		void End();
		void WritePropertyData(IBaseInstance *pBase);
		bool ExportDocument();
		bool WriteNode(IDocNode *pNode);
	public:
		ExportXML(IDocument *pDocument);
		virtual ~ExportXML();

		virtual bool ExportToStream(noice::io::IStream *pStream);
		virtual bool ExportToFile(const char *sFilename);
	};
}