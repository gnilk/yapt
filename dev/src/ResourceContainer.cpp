/*-------------------------------------------------------------------------
File    : $Archive: yResourceContainer.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : A resource container is a holder of the IO resource locater in order
          to allow resource object to load data from different resources.
	  All objects in the resource containers are loaded through the same
	  IO handler.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Solve actual sharing of the file handler
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


ResourceContainer::ResourceContainer() :
	BaseInstance(kInstanceType_ResourceContainer)
{
//	resourceRoot = new DocNode(dynamic_cast<IDocument *> (this));
}
ResourceContainer::~ResourceContainer()
{
	
}

bool ResourceContainer::Initialize()
{
	return false;
}
bool ResourceContainer::InitializeAllResources()
{
	return false;
}
IPluginObjectInstance *ResourceContainer::GetResourceInstance(const char *name)
{
	return NULL;
}
void ResourceContainer::AddResourceInstance(IPluginObjectInstance *pInstance)
{
}
void *ResourceContainer::GetIOHandler()
{
	return NULL;
}
noice::io::IStream *ResourceContainer::OpenStream(const char *sStreamName, unsigned int flags)
{
	const char *serviceLocator = GetAttributeValue("service");
	std::string URL;
	if (serviceLocator == NULL)
	{
		URL.append("file://");
	} else
	{
		// TODO: verify "://" token at the end of service locator
		URL.append(serviceLocator);
	}
	URL.append(sStreamName);

	return yapt::GetYaptSystemInstance()->CreateStream(URL.c_str(),flags);
}
