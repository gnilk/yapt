/*
 * LoadFile.cpp
 *
 *  Created on: Jul 12, 2012
 *      Author: gnilk
 */

#include <string>
#include "yapt/ySystem.h"
#include "noice/io/io.h"
#include "noice/io/ioutils.h"


#include "LoadFile.h"

using namespace yapt;
using namespace noice::io;

LoadFile::LoadFile() :
	fileName(NULL),
	fileData(NULL)
{
	// TODO Auto-generated constructor stub

}

LoadFile::~LoadFile() {
	// TODO Auto-generated destructor stub
}

void LoadFile::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	fileName = pInstance->CreateProperty("fileName", kPropertyType_String, "file.txt", "");
	fileData = pInstance->CreateOutputProperty("fileData", kPropertyType_UserPtr, NULL, "");
	byteSize = pInstance->CreateOutputProperty("byteSize", kPropertyType_Integer, 0, "");
    pSys = ySys;
}

void LoadFile::Render(double t, IPluginObjectInstance *pInstance) {
	// TODO: Load file from fileName into fileData
	std::string name = std::string("file://")+fileName->v->string;
    noice::io::IStream *pStream = pSys->CreateStream(name.c_str(),0);
//	noice::io::IStream *pStream = DeviceManager::GetInstance()->CreateStream(name.c_str(), 0);
	if (pStream != NULL)
	{
		if (pStream->Open(kStreamOp_ReadOnly)) {
			long szData = pStream->Size();
			void *pBuffer = malloc(szData);

			pStream->Read(pBuffer, szData);
			byteSize->v->int_val = (int)szData;			

		} else {
			//pLogger->Error("Unable to open stream");
		}
		pStream->Close();
	}
}

void LoadFile::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void LoadFile::PostRender(double t, IPluginObjectInstance *pInstance) {

}
