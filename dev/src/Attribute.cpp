/*-------------------------------------------------------------------------
File    : $Archive: yAttribute.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements attributes; key-value's (string:string) for objects
	  normally used by XML input and maps to element attributes 

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/logger.h"

#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;


Attribute::Attribute(const char *name, const char *value)
{
	this->name = strdup(name);
	this->value = strdup(value);
}
Attribute::~Attribute()
{
	free (this->name);
	free (this->value);
}

const char *Attribute::GetName()
{
	return name;
}
const char *Attribute::GetValue()
{
	return value;
}
void Attribute::SetValue(const char *value)
{
	free (this->value);
	this->value = strdup(value);
}
