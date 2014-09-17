#pragma once

#include <sys/stat.h>

#include "yapt/ySystem.h"
#include "yapt/logger.h"
#include "yapt/thread.h"
#include "yapt/event.h"
#include "yapt/mutex.h"

#include <vector>
#include <string>

namespace yapt {

  // helper class
  class FileWatchItem {
  protected:
  	struct stat previous;

    std::string filename;
    IFileWatcher *callback;
  public:
    FileWatchItem(const char *filename, IFileWatcher *callback);
    virtual ~FileWatchItem();
    void NotifyOnChange();
   private:
   	bool HasChanged();
  };

  class FileWatcher
  {
  private:
  	  ILogger *pLogger;
      Thread *thread;
      Event stopThread;
      Mutex itemlock;
      std::vector<FileWatchItem *> watchitems;
      static int fileWatcherThread(void *param);     
  public:
      FileWatcher();
      virtual ~FileWatcher();

      void MonitorThreadLoop();

      void Start();
      void Stop();

      void AddFile(const char *filename, IFileWatcher *callback);
      void RemoveFile(const char *filename);
   private:
   	  void CheckItems();

  };
}