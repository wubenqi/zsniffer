// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//
//	设计模式单件实现类文件
//

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

#include <cstdlib>
#include <assert.h>

template<typename T>
class Singleton  {
public:
	static T& Instance() {
		if(Singleton::instance_==0) {
			Singleton::instance_ = CreateInstance();
		}
		return *(Singleton::instance_);
	}
	
	static T* GetInstance() {
		if(Singleton::instance_==0) {
			Singleton::instance_ = CreateInstance();
		}
		return Singleton::instance_;
	}

	static void DestroyInstance() {
		if(Singleton::instance_!=0) {
			DestroyInstance2(Singleton::instance_);
			Singleton::instance_=0;
		}
	}

protected:
	inline explicit Singleton()	{
		assert(instance_==0);
		Singleton::instance_ = static_cast<T*>(this);
	}

	inline ~Singleton() {
		Singleton::instance_ = 0;
	}
	
private:
	static T* CreateInstance(){
		return new T();
	}

	static void DestroyInstance2(T* p) {
		delete p;
	}
	
private:
	static T *instance_;

private:
	inline explicit Singleton(Singleton const &){}
	inline Singleton& operator=(Singleton const&){return *this;}
};

template<typename T>
T* Singleton<T>::instance_=0;

#endif // BASE_SINGLETON_H_
