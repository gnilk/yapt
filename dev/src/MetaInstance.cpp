/*-------------------------------------------------------------------------
File    : $Archive: MetaInstance.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : A transparent instance inserted into the document and serves as a place holder for other things
          Contains it's own document structure - used by Include directive to save a separate copy of the
          include tree in order to preserve that when saving.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 25.04.12, FKling, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


MetaInstance::MetaInstance(IDocument *pOwner) :
	BaseInstance(kInstanceType_MetaNode)
{
  Document *pOwnerDoc = dynamic_cast<Document *>(pOwner);

  Document *pDoc = new Document(pOwnerDoc->GetContext());
  pDoc->SetDocumentController(pOwnerDoc->GetDocumentController());

  pDocument = pDoc;
 
}
MetaInstance::~MetaInstance()
{
	delete pDocument;
}

IDocument *MetaInstance::GetDocument() 
{
  return pDocument;
}
