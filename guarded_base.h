#ifndef guarded_base_h____
#define guarded_base_h__

#include <zthread/Guard.h>
#include <zthread/Mutex.h>
#include <zthread/ReadWriteLock.h>

/*
    ZThread native read-write locks are fake. Use 
    read-excluisive/write exclusive lock for safety.
*/
class FakeReadWriteLock: public ReadWriteLock {
    FastMutex _lock;

  public:

    PriorityWriteRWLock() {}

    //! Destroy this ReadWriteLock
    virtual ~BiasedReadWriteLock() {}

    /**
     * @see ReadWriteLock::getReadLock()
     */
    virtual Lockable& getReadLock() { return _lock; }

    /**
     * @see ReadWriteLock::getWriteLock()
     */
    virtual Lockable& getWriteLock() { return _lock; }
};

// guards for objects following concept rw_lockable_
// concept requirements:
//   - const correctness
//   - Lockable& get_read_lock() const
//   - Lockable& get_write_lock() 
//


/**
	Guard for shared read access (read only).
	
	Example usage:
	
		const_guard<my_scruct> guard(s);
			// creates guard for s's read-only lock 
			// s may be const qualified
*/
template <typename T>
struct const_guard: public ZThread::Guard<ZThread::Lockable> {	
	const_guard(T const& rw_lockable)
		: ZThread::Guard<ZThread::Lockable>(rw_lockable.get_read_lock())
	{}
};

/**
	Guard for mutating, exclusive access .
	
	Example usage:
		mutable_guard<my_struct> guard(s);
			// creates guard for s's exclusive write lock
			// s musn't be const-qualified (compiler error)
*/
template <typename T>
struct mutable_guard: public ZThread::Guard<ZThread::Lockable> {	
	mutable_guard(T& rw_lockable)
		: ZThread::Guard<ZThread::Lockable>(rw_lockable.get_write_lock())
	{}
};


class base_rw_lockable {
public:
	typedef const_guard< base_rw_lockable >   read_guard;
	typedef mutable_guard< base_rw_lockable > write_guard;
	
	// implement Concept of rw_lockable_
	ZThread::Lockable& get_read_lock() const {
		return const_cast<base_rw_lockable&>(*this).lock_.getReadLock();
	}
	
	ZThread::Lockable& get_write_lock() {
		return lock_.getWriteLock();
	}
private:
	ZThread::FakeReadWriteLock lock_;
};

template <typename T>
class guarded: public base_rw_lockable {
public:
	typedef T delegate_type;
	
	// all at once accessor
	delegate_type get_content() const {
		read_guard g(*this);
		return delegate();
	}
	
	void set_content(delegate_type const& other) {
		write_guard g(*this);
		delegate() = other;
	}
	
protected:
	T const& delegate() const { 
		return delegate_; 
	}
	
	T& delegate() { 
		return delegate_; 
	}
private:
	T delegate_;
};


#endif // guarded_base_h__

