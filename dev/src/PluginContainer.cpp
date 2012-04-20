/*-------------------------------------------------------------------------
File    : $Archive: yPluginContainer.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Small class to hold a list of plugins and their associated items

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
- 
</pre>


\History
- 23.09.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include <map>
#include <assert.h>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;

PluginContainer::PluginContainer(PFNINITIALIZEPLUGIN pFuncInitialize) :
	BaseInstance(kInstanceType_PluginContainer)
{
	this->pFuncInitialize = pFuncInitialize;
	AddAttribute("name","<noname>");
}
PluginContainer::~PluginContainer()
{
}
void PluginContainer::AddObjectDefinition(IBaseInstance *pDef)
{
	objects.push_back(pDef);
}
