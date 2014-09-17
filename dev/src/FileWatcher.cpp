#include <sys/stat.h>
#include <time.h>

#include <vector>

#include "yapt/ySystem.h"
#include "yapt/logger.h"
#include "yapt/thread.h"
#include "yapt/event.h"
#include "yapt/FileWatcher.h"

using namespace yapt;

FileWatcher::FileWatcher() {
	thread = NULL;	
	pLogger = Logger::GetLogger("FileWatcher");
}

FileWatcher::~FileWatcher() {
	Stop();
}

void FileWatcher::Start() {
	if (thread == NULL) {
		pLogger->Debug("Starting up monitor thread");
		stopThread.Reset();
		itemlock.Initialize();
		thread = Thread::Create(fileWatcherThread, (void *)this, Thread::kThreadCreateDefault);
	} 
}

void FileWatcher::Stop() {
	if (thread != NULL) {
		// set stop flag
		pLogger->Debug("Stop thread trigger");
		stopThread.Trigger();		
		thread->Join();
		pLogger->Debug("Monitor thread stopped");
		delete thread;
		thread = NULL;
	}
}

void FileWatcher::AddFile(const char *filename, IFileWatcher *callback) {

	Start();

	pLogger->Debug("Flagging file '%s' for change notifications", filename);

	itemlock.Enter();
	FileWatchItem *item = new FileWatchItem(filename, callback);
	watchitems.push_back(item);
	itemlock.Leave();
}

void FileWatcher::RemoveFile(const char *filename) {
	itemlock.Enter();
	std::vector<FileWatchItem *>::iterator it = watchitems.begin();
	for(;it != watchitems.end();it++) {
		FileWatchItem *item = *it;		
	}
	itemlock.Leave();
}

void FileWatcher::CheckItems() {
	itemlock.Enter();
	for(int i=0;i<watchitems.size();i++) {
		watchitems[i]->NotifyOnChange();
	}
	itemlock.Leave();
}

int FileWatcher::fileWatcherThread(void *param) {
	((FileWatcher *)param)->MonitorThreadLoop();
	return 0;
}

void FileWatcher::MonitorThreadLoop() {
	pLogger->Debug("Monitor thread running");
	while(!stopThread.IsTriggered()) {
		CheckItems();
		Thread::Sleep(1000);
	}
	pLogger->Debug("Monitor thread stop was triggered, leaving..");
}

//////// --------
FileWatchItem::FileWatchItem(const char *filename, IFileWatcher *callback) {
	this->filename = std::string(filename);
	this->callback = callback;
	lstat(filename, &previous);	// initial stat
}

FileWatchItem::~FileWatchItem() {

}

void FileWatchItem::NotifyOnChange() {
	if (HasChanged()) {
		Logger::GetLogger("FileWatchItem")->Debug("Change detected on file '%s', calling sink at %p", filename.c_str(), callback);

		callback->OnFileChanged(filename.c_str());
	}
}

bool FileWatchItem::HasChanged() {
	bool changed = false;
	struct stat _stat;
	lstat(filename.c_str(), &_stat);

	if (_stat.st_mtimespec.tv_sec != previous.st_mtimespec.tv_sec) {
		changed = true;
	}

	previous = _stat;

	return changed;
}


