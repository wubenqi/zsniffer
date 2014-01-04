// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "base/thread.h"

#include <boost/thread/thread.hpp>

#ifdef WIN32
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace base {

Thread::Thread() {
  thread_running_ = false;
  thread_ = NULL;
  auto_delete_ = false;
}

Thread::~Thread() {
  boost::mutex::scoped_lock scoped_lock(thread_mutex_);

  if (thread_ && thread_running_) {
    scoped_lock.unlock();
    Join();
  }

  if (thread_) {
    delete thread_;
    thread_ = NULL;
  }
}

void Thread::Start() {
  boost::mutex::scoped_lock scoped_lock(thread_mutex_);
  if (thread_) {
    delete thread_;
    thread_ = NULL;
  }
  thread_ = new boost::thread(boost::bind(&Thread::RunThread, this));
}

void Thread::RunThread() {
  {
    boost::mutex::scoped_lock scoped_lock(thread_mutex_);
    thread_running_ = true;
  }

  Run();

  bool auto_delete;
  {
    boost::mutex::scoped_lock scoped_lock(thread_mutex_);
    thread_running_ = false;
    auto_delete = auto_delete_;
  }

  if (auto_delete) {
    delete this;
  }
}

void Thread::Join() {
  boost::mutex::scoped_lock scoped_lock(thread_mutex_);

  if (thread_running_) {
    scoped_lock.unlock();
    thread_->join();
  }
}

void Thread::Sleep(unsigned long milliseconds) {
#ifdef WIN32
  ::Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

void Thread::YieldCurrentThread() {
  Sleep(0);
}

void Thread::SetAutoDelete(bool value) {
  boost::mutex::scoped_lock scoped_lock(thread_mutex_);
  auto_delete_ = value;
}

bool Thread::GetAutoDelete() const {
  boost::mutex::scoped_lock scoped_lock(thread_mutex_);
  return auto_delete_;
}

}
