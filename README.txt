c++ (98) reflection playground

there are 3 "separate" efforts:

mo_full:
---------
To visit a class and/or interface and build
a "meta" of it; 
   
This meta should allow use of
member methods&fields of given class
when given instance of implementation of interface
or instance of class/subclass.

status:
	* POC
	* method visitor stub ready

reflection:
---------------

To create RTTI featuring instrumented calls. i.e
I can invoke object methods with interface:

some_object
method_info mi = get_from_somewhere(...)
any ret = mi.invoke(some_object, 1, std::string("abc"))

status:
	* basic method_info with invoke working ready
	

proxy
--------

to create java.lang.reflect.Proxy like infra, so
one can build an object which can fake actual implementation

this fake implementation have to implement:
	any invoke(void*, vector<any>)
to receive all calls.

status: 
	* cannot be made 100% portable since we fake virtual
	  table!
	* works on gcc, 
	* problems on msvs due to plethora of different calling
	  conventions (see proxy.h & vtable_research.txt)
