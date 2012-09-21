/*
 * WriteFile.h
 *
 *  Created on: Sept 21, 2012
 *      Author: gnilk
 */

#ifndef WRITEFILE_H_
#define WRITEFILE_H_

#include "yapt/ySystem.h"

using namespace yapt;

class WriteLine : public IPluginObject {
public:
	WriteLine();
	virtual ~WriteLine();
	// IPluginObject
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	Property *string;
};

#endif /* LOADFILE_H_ */
