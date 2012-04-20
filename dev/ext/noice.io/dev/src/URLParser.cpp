/*-------------------------------------------------------------------------
File    : $Archive: URLParser.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Parses URL's on the form "service://resource" separates them into
	  Service and Resource. Very simplistic but serves my purpose

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <malloc.h>			// needed for alloca
#endif

#include "noice/io/ioutils.h"

using namespace noice::io;

URLParser::URLParser(const char *url)
{
	service = NULL;
	path = NULL;
	if (!DoParseURL(url))
	{
		if (service) free(service);
		if (path) free(path);
		service = NULL;
		path = NULL;
	}
}
URLParser::~URLParser()
{
	if (service) free(service);
	if (path) free(path);
	service = NULL;
	path = NULL;
}
bool URLParser::DoParseURL(const char *url)
{
	bool bRes = false;
	char *tmp = (char *)alloca(strlen(url)+1);

	strcpy(tmp,url);

	char *pServiceStart = tmp;
	while((*tmp != ':') && (*tmp!='\0')) tmp++;
	if (*tmp == ':')
	{

		*tmp = '\0';
		service = strdup(pServiceStart);
		tmp++;
		if (strlen(tmp) > 2)	// have atleast two chars (//) ?
		{
			// make sure we strip away '/' you can be lazy and specify file:filename
			if (*tmp == '/')
			{
				while ((*tmp=='/') && (*tmp!='\0')) tmp++;
			}

			if (*tmp != '\0')
			{			
				path = strdup(tmp);
				bRes = true;
			}
		}
	}
	return bRes;
}

URLParser *URLParser::ParseURL(const char *url)
{
	URLParser *parser = new URLParser(url);
	if(parser->GetService() == NULL)
	{
		delete parser;
		parser = NULL;
	}
	return parser;
}

const char *URLParser::GetService()
{
	return service;
}
const char *URLParser::GetPath()
{
	return path;
}

char *URLParser::GetServiceCopy(char *dst, int maxlen)
{
	return strncpy(dst, service, maxlen);
}
char *URLParser::GetPathCopy(char *dst, int maxlen)
{
	return strncpy(dst, path, maxlen);
}
