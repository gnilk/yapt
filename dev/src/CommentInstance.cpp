/*-------------------------------------------------------------------------
File    : $Archive: CommentInstance.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2014-09-04, 15:50
Descr   : Holds a comment. Main purpose is to preserv position in the tree for comments in the XML
          during load/save.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 04.09.14, FKling, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


CommentInstance::CommentInstance() :
	BaseInstance(kInstanceType_Comment)
{
	this->comment = NULL; 
}
CommentInstance::~CommentInstance()
{
	if (this->comment != NULL) {
		free(this->comment);		
	}
}

char *CommentInstance::GetComment() 
{
  return this->comment;
}

void CommentInstance::SetComment(char *comment)
{
	if (this->comment != NULL) {
		free(this->comment);
	}
	this->comment = strdup(comment);
}
