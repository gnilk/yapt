#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

Timeline::Timeline() {

}
Timeline::~Timeline() {

}

int Timeline::GetNumExecutors() {
	return 0;
}
IBaseInstance *Timeline::GetExecutorAtIndex(int idx) {
	return NULL;
}
