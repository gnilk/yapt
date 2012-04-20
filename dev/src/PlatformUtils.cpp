
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "yapt/yPlatformUtils.h"

using namespace yapt;

// Stupid lower case transformation
char *strlwr(char *string)
{
	int i,n;
	n = strlen(string);
	for(i=0;i<n;i++)
	{
		string[i] = toupper(string[i]);
	}
	return string;
}