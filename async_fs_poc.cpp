#include <string>
#include <iostream>

#include "tinfra/shared_ptr.h"
#include "tinfra/callback.h"

#include "tinfra/symbol.h"
#include "callfwd.h"



using std::string;

struct read_file_completion {
	bool status;
	string content;
};

std::ostream& operator<<(std::ostream& out, read_file_completion const& v)
{
	if( v.status )
		return out << "success, " << v.content;
	else
		return out << "failed";
}

// general idea for ASYNC file system access

// create async-fs wrapper over generic synchronous FS:
// when retrieving something one must have
// async_fs_client instance.
// async_fs_client manages lifecycle of FS requests
//  * when it's deleted, all outstanding requests are cancelled
//    so after completion no dangling pointers are left in async_fs dispatcher
//  * async_fs_client has vfs similar api, but receives function objects that
//    receive results
//  * example:

//    async_fs_client.read_file(tstring const& path, kurna_mac_bind(&MyControl::fileLoadCompleted, *this, _1))

//    async_fs_client.list_folder(tstring const& path, tinfra::callback(*this, &MyControl::fileLoadCompleted)

struct async_fs {
	void read_file(string const& name, callback<read_file_completion> callback)
	{
		stored = callback;
	}
	
	void process()
	{
		read_file_completion result = { true, "content" };
		
		stored(result);
	}
	
	callback<read_file_completion> stored;
};

struct writer {
	template <typename T>
	void operator()(tinfra::symbol const& sym, T const& v)
	{
		std::cout << "pushing message: " << typeid(T).name() << "\n";
		std::cout << v << "\n";
	}
	
	template <typename T>
	void managed_struct(T const& v, tinfra::symbol const&)
	{
		tinfra::process(v, *this);
	}
};

int main(int argc, char** argv)
{
	async_fs fs;
	writer wr;
	callfwd::call_sender<writer> target(wr);
	fs.read_file("test", make_callback<read_file_completion>(target));
	fs.process();
	return 0;
}


