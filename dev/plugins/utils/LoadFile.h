/*
 * LoadFile.h
 *
 *  Created on: Jul 12, 2012
 *      Author: gnilk
 */

#ifndef LOADFILE_H_
#define LOADFILE_H_

#include "yapt/ySystem.h"

using namespace yapt;

class LoadFile : public IPluginObject {
public:
	LoadFile();
	virtual ~LoadFile();
	// IPluginObject
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
private:
	Property *fileName;
	Property *fileData;
	Property *byteSize;
    ISystem *pSys;
};

#endif /* LOADFILE_H_ */
