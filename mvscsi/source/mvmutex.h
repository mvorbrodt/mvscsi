#ifndef _MVMUTEX_
#define _MVMUTEX_

#include <windows.h>
#include <exception>
#include <boost/shared_ptr.hpp>

class mvMutex;
typedef boost::shared_ptr<mvMutex> LPmvMutex;

class mvInvalidMutex : public std::exception {};

class mvMutex {
public:
	mvMutex() : mutex_(CreateMutex(NULL, FALSE, NULL)) {
		if(mutex_ == NULL) throw mvInvalidMutex(); }
	~mvMutex() { CloseHandle(mutex_); }

	void Lock() const { WaitForSingleObject(mutex_, INFINITE); }
	void Unlock() const { ReleaseMutex(mutex_); }

private:
	mvMutex(const mvMutex&);
	mvMutex& operator=(const mvMutex&);

	HANDLE mutex_;
};

class mvMutexLock {
public:
	mvMutexLock(const LPmvMutex& mutex) : mutex_(mutex) { mutex_->Lock(); }
	~mvMutexLock() { mutex_->Unlock(); }

private:
	mvMutexLock(const mvMutexLock&);
	mvMutexLock& operator=(const mvMutexLock&);

	LPmvMutex mutex_;
};

#endif
