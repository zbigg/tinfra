#include <iostream>
#include <functional>
#include <set>
#include <cassert>
#include <memory>

#define AV_TRACE(p, args) std::cerr << "av(" << (p) << "): " << args << "\n"

class avb;

template <typename V>
class av;

template <typename V>
class cav;

typedef std::function<void()> av_callback;

class av_ref_base {
	// upon construction
	// it register itself in 
	// av->refs
	// oposite in destruction
public:
	av_ref_base(avb*, avb*, av_callback callback);
	av_ref_base(av_ref_base const& second);
    ~av_ref_base();

	avb* get() const { return ref; }
	avb* operator->() const {return ref; }

	avb* get_source() const { return source; } 
	void notify() { callback(); }
private:
	avb*        ref;
	avb*        source;
	av_callback callback;
};

template <typename V>
class av_ref: public av_ref_base {
public:
	av_ref(cav<V>* p, avb* s, av_callback callback): av_ref_base(p, s, callback) {}
	~av_ref() {}
	
	cav<V>* get() const { return reinterpret_cast<cav<V>*>(av_ref_base::get()); }
	cav<V>* operator->() const { return this->get(); } 
};

class avb {
	std::set<av_ref_base*> refs;
public:
	avb();
	~avb();
	void send_update();
	void add_ref(av_ref_base* p);
	void remove_ref(av_ref_base* p );
};

//
// av_ref_base impl
//
inline av_ref_base::av_ref_base(avb* p, avb* s, av_callback cb): 
    ref(p),
    source(s),
    callback(cb)
{
	this->ref->add_ref(this);
}

inline av_ref_base::av_ref_base(av_ref_base const& second): 
    ref(second.ref),
    source(second.source),
    callback(second.callback)
{
	this->ref->add_ref(this);
}

inline av_ref_base::~av_ref_base()
{
	this->ref->remove_ref(this);
}

//
// avb
//
inline avb::avb()
{
}
inline avb::~avb()
{
	assert(this->refs.size() == 0);
}
inline void avb::send_update()
{
    AV_TRACE(this, "send_update: ref_cnt " << this->refs.size());
	for( std::set<av_ref_base*>::const_iterator i = this->refs.begin(); i != this->refs.end(); ++i ) 
	{
		av_ref_base* r = *i;
		r->notify();
	}
}

inline void avb::add_ref(av_ref_base* p) {
    
	this->refs.insert(p);
	AV_TRACE(this, "add_ref " << this->refs.size() << " p=(" << p->get_source() <<")");
}
inline void avb::remove_ref(av_ref_base* p ) {
	this->refs.erase(p);
	AV_TRACE(this, "remove_ref " << this->refs.size() << " p=(" << p->get_source() <<")");
}

template <typename V>
std::function<V()> null_function() {
	return std::function<V()>();
}

template <typename V>
class av_constant {
	V  v;
public:
	av_constant(V const& v): v(v) {}
	
	V operator()() const {
	    return v;
	}
};
/*
template <typename V>
class av_reference {
	av_ref<V> ref;
public:
	av_reference(av<V> const& v, av_callback c): 
	    ref(v, c)
    {
    }
	
	V operator()() const {
	    return ref->get();
	}
};
*/
template <typename V>
class av_plus {
	av_ref<V> a;
	av_ref<V> b;
public:
	av_plus(av<V> const& v, av<V> const& b, av_callback c): 
	    a(v,c),
	    a(v,c)
    {
    }
	
	V operator()() const {
	    return a->get() + b->get();
	}
};

template <typename V>
class cav: public avb {
public:
    cav():
        cached(false)
    {
    }
	cav(std::function<V()> gen):
	    cached(false),
		gen(gen)
	{
	}
	
	cav(V const& val):
	    cached(true),
		gen(av_constant<V>(val)),
		val(val)
	{
	}
	// called by other values to update
	// values
	void update()
	{
		const V tmp = this->gen();
		if( cached ) { 
		    if( tmp == this->val ) {
		        AV_TRACE(this, "update: no change");
		        return; // no update
		    }
		}
		this->val = tmp;
		this->cached = true;
		AV_TRACE(this, "update: changed to " << this->val << ", sending_update");
		send_update();
	}
	
	av_callback get_update_callback() {
	    return std::bind(&cav<V>::update, this);
	}
	
    V get() {
        if( cached ) {
            AV_TRACE(this, "get: using cached " << val);
            return this->val;
        }
        
        
        val = this->gen();
        AV_TRACE(this, "get: saving value " << val);
        cached = true;
        return val;
    }
    
    V get() const {
        if( cached ) {
            AV_TRACE(this, "get: using cached " << val);
            return this->val;
        }
        V tmp = tmp;
        AV_TRACE(this, "get: returning tmp " << tmp);
        return tmp;
    }
    
    void set(V const& new_value) {
        if( get() == new_value ) 
            return;
        gen = av_constant<V>(new_value);
        cached = true;
        val = new_value;
        AV_TRACE(this, "set: changed to " << this->val << ", sending_update");
        send_update();
    }
    
    void set(std::function<V()> gen) {
        this->gen = gen;
        cached = false;
        AV_TRACE(this, "set: changed generator, sending_update");
        send_update();
    }
    
private:
	bool               cached;
	V                  val;
	std::function<V()> gen;
};

template <typename V>
class av {
    std::shared_ptr<cav<V>> v;
public:
    explicit av(std::shared_ptr<cav<V>> const& pcv): v(pcv) {}
    explicit av(V const& v): v(new cav<V>(v)) {}
    explicit av(std::function<V()> const& gen): v(new cav<V>(gen)) {}
    
    V get() const { return v->get(); }
    void set(V const& new_value) {
        v->set(new_value);
    }
    
    av<V>& operator=(V const& new_value) {
        this->set(new_value);
        return *this;
    }
    cav<V>* get_cav() const { return v.get(); }
};

template <typename V, typename X>
struct plus1 {
	av_ref<V> av;
	X         x;
	
	V operator()() {
		return x + av->get();
	}
};

template <typename V, typename X>
av<V> operator+(av<V> const& a, X b)
{
	std::shared_ptr<cav<V>> ptr(new cav<V>());
	AV_TRACE(ptr.get(), "::= plus(av(" << a.get_cav() << "),const(" << b << "))"); 
	plus1<V,X> gen = { av_ref<V>(a.get_cav(), ptr.get(), ptr->get_update_callback()), b };
	ptr->set(gen);
	return av<V>(ptr);
}

template <typename X, typename V>
av<V> operator+(X a, av<V> const& b)
{    
	std::shared_ptr<cav<V>> ptr(new cav<V>());
	AV_TRACE(ptr.get(), "::= is plus(av(" << b.get_cav() << "),const(" << a << "))");
	plus1<V,X> gen = {
	    av_ref<V>(b.get_cav(), ptr.get_cav(), ptr->get_update_callback()), a 
	};
	ptr->set(gen);
	return av<V>(ptr);
}

template <typename V>
struct plus2 {
	av_ref<V> a;
	av_ref<V> b;

	V operator()() {
		return a->get() + b->get();
	}
};

template <typename V>
av<V> operator+(const av<V>& a, const av<V>& b)
{    
	std::shared_ptr<cav<V>> ptr(new cav<V>());
	plus2<V> gen = { 
	    av_ref<V>(a.get_cav(), ptr.get(), ptr->get_update_callback()),
	    av_ref<V>(b.get_cav(), ptr.get(), ptr->get_update_callback())
	};
	ptr->set(gen);
	return av<V>(ptr);
}


av<int> x(1);

int main()
{
	av<int> y = x + 3;
	av<int> z = x + y;
	
	std::cout << "x=" << x.get() << "\n";
	std::cout << "y=" << y.get() << "\n";
	std::cout << "z=" << z.get() << "\n";

	x = 10;
	std::cout << "x=" << x.get() << "\n";
	std::cout << "y=" << y.get() << "\n";
	std::cout << "z=" << z.get() << "\n";
	
	//av<int> bad = x+y;
	// this breaks referencing scheme
	// probably during "unreferencing" of former x->get_cav()
	//x = y+z;
	std::cout << "x=" << x.get() << "\n";
}

