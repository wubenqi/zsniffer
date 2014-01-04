// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#ifndef BASE_THREAD_H_
#define BASE_THREAD_H_

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace base {

class Thread : 
  private boost::noncopyable {
public:
  Thread();
  virtual ~Thread();

  void Start();
  void Join();

  void SetAutoDelete(bool);
  bool GetAutoDelete() const;

  static void Sleep(unsigned long milliseconds);
  static void YieldCurrentThread();

protected:
  virtual void Run() = 0;
  void Terminate();

  mutable boost::mutex thread_mutex_;
  bool auto_delete_;

private:
  void RunThread();

  bool thread_running_;
  boost::thread * thread_;
};

}

#endif //__THREAD_H_

