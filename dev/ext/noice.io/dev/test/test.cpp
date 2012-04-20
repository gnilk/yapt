#include <stdio.h>
#include <stdlib.h>
#include "noice/io/io.h"
#include "noice/io/ioutils.h"

using namespace noice::io;

static int IO_CALLCONV DevListFunc(const char *id)
{
	static int counter =0;
	printf("[%d]: %s\n",counter++,id);
	return 0;
}

static int LoadFile(const char *filename)
{
	int res = 0;
	int n;
	char tmp[129];
	IStream *pStream = DeviceManager::GetInstance()->CreateStream(filename,0);
	if (pStream != NULL)
	{
		if (pStream->Open(kStreamOp_ReadOnly))
		{
			while((n=pStream->Read(tmp, 128))>0)
			{
				//tmp[n]='\0';
				//fprintf(stdout,"%d:%s\n",n,tmp);
			}
		} else
		{
			//printf("failed open\n");
			res = -1;
		}
		pStream->Close();
	} else
	{
		//printf("failed Create\n");
		res = -2;
	}
	return res;
}

#define CHECK(exp,passcode) (((exp)==(passcode))?"*ok; "#exp"; ret="#passcode:"*failed: "#exp)
#define TEST(checkexp) printf("%s\n",checkexp);
int main(int argc, char **argv)
{
	printf("Dumping devices\n");
	DeviceManager::GetInstance()->EnumerateURIHandlers(DevListFunc);

	printf("Testing Simple Loading\n");
	// Test with wrong device, expect "-2" (device not found) from LoadFile
	TEST(CHECK(LoadFile("zip://dummy.txt"),-2));
	// Test with wrong filename expect "-1" (file not found) from LoadFile
	TEST(CHECK(LoadFile("file://dummy.txt"),-1));
	// Test with proper device description and filename, expect ok
	TEST(CHECK(LoadFile("file://file.txt"),0));
	// Test with just filename (default device), expect ok
	TEST(CHECK(LoadFile("file.txt"),0));

	// Set the relative path of the file device and try loading again
	printf("Testing Device Relative Path change with loading\n");
	IStreamDevice *pDevice = DeviceManager::GetInstance()->GetIODevice("file");
	pDevice->SetParam("relpath","..");
	IStream *pStream = pDevice->CreateStream("bin/file.txt",0);
	TEST(CHECK(pStream->Open(kStreamOp_ReadOnly),true));

	pStream->Close();
}