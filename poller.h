// tinfra/async
// 
// to provide universal 
//   timer waiter
//   fd (socket, pipe) ready-waiter
//   subprocess waiter
//   signal waiter

/////////
//
// signal usage
//
// tinfra::poller P; 
// tinfra::simple_pipe sigchld_pipe;
//
// sigchld_handler() {
//   sigchld_pipe.write("C",1);
// }
// sigchld_handler2(poller& P, poll_result R, int fd, buf, size) {
//    !!! handle that fact, that signal
//    occurred, once and only once without any race !!!
//    install_sigchld(P)
// }
// install_sigchld(poller& P)
//   signal(SIGCHLD, sigchld_handler);
//   static char buf[2];
//   P.async_read(sigchld_pipe.get_read_descriptor(), deadline, buf, 2, sigchld_handler2)
// 
// main pool & timer usage //
//
//   poller_tag some_timer_tag = P.install_timer(deadline)
// in loop:
//   poll_result R = P.poll(optional timeout if you really want restart in some intervals)
//   if( R.is_timeout) ...
//   if( R.is_interrupted ) ...
//   if( R.is_error) ...
//   if( R == some_timer_tag ) {
//   }
// }
//
// ProcessManager usage
//
//   process usage
//   poller P;
//   ProcessManager pm(P); -- creates sigchld_pipe, installs SIGCHLD, and registers callback 
//
// 
//   pm.start_process(params, daeadline, optional callback)
//   P.poll()
//      will do one of:
//         internally trigger sigchld_handler2 for child exit and handle wait()
//         intenally trigger deadline (timer) and kill(TERM) process and create next timer and will
//         eventually trigger next deadline and will kill(KILL) the process
//         if callback is given, it will have signature:
//
//                process_exit_callback(some_custom_params, process_exit_result const& per)
//         
//    ProcessManager destruction / kill
//       will create custom Poller which will only wait for 
//         deadline timers
//         sigchld_pipe
//       and manage it till deadlines occur and all processes are ended (or! stalled in zombie 
//       state)
//
//  FD dispatching
// 
//    there will exist several 
//         connection FDs (mostly read, few WRITE)
//         few listen FDs
//         some other timers ? mostly managed in async_read/write deadlines (?)
//
//    some_object will add async LISTENER to poller (how shall it communicate "read" ?)
//
//       object init:
//          this->read_wait_tag = P.wait(Poller::READ, fd, callback?)
//
//       in read_callback
//          read it and rearm ??
//          write results to buffer ...
//
//       in buffer handling
//          this->write_wait_tag = P.wait(Poller::WRITE, fd, callback?)
//          NOTE:
//            rearming may be inefficient with epoll (2 extra syscalls, or 1 with EPOLLONESHOT) maybe waits 
//            shall not automatically rearm-able
//
//       in destructor
//          P.remove_tag(this->read_wait_tag)
//          P.remove_tag(this->write_wait_tag)
//
// static char buf[2];
//
/////////

class poll_tag {
public:    
    explicit poll_tag(long id)
    explicit poll_tag(poll_tag pt);
    poll_tag& operator=(poll_tag const& pt);
    
    bool operator==(poll_tag const& pt);
    bool operator!=(poll_tag const& pt);
    
    long idx() const { return id; }
private:    
    long id;
};

class poll_result {
    poller*  source;
    poll_tag tag;    // internal poller TAG
    int      fd;     // on which FD it occurred
    int      type;   // error, read ok, write ok, timeout_fd, timeout_poll
    int      error_code;  // in case of error, an errno
    
    void*    client_data;
};

class poller {
public;    
    enum event_type {
        READ,
        WRITE
    };
    // TODO, redefine deadline
    poll_result poll(int d=-1); 
    
    poll_tag wait_timer(int deadline); // wait only for timeout
    poll_tag wait_timer(int deadline, callback<poll_result>); // wait only for timeout
    poll_tag wait_fd(enum event_type et, int fd, int d=1);
    poll_tag wait_fd(enum event_type et, int fd, int d=1, callback<poll_result> c, void* data);
    
    void     remove(poll_tag pt);
    void     remove(enum event_type et, int fd)
private:
    // noncopyable
    poller(poller const&);
    poller& operator=(poller const&);
    
    struct impl;
    std::auto_ptr<impl> imp;
};

struct async_read_result {
    int    fd;
    char*  buf;
    size_t requested_size;
    size_t readed_size;
    
    poll_result status;
}

struct async_read_data {
    int    fd;
    char*  buf;
    size_t requested_size;
    size_t readed_size;
    
    callback<async_read_result> callback;    
};

//
// async_read attempt
// 
// in general, very clean idea besides one ugly fact
// current tinfra-sandbox callback.h cannot carry
// any additional data (like std::function in functor instance or in
// std::bind bound arguments
//
// i.e it's not easily composable

void async_read_callback(poll_result const& pr)
{
    async_read_data* ard = (async_read_data*)pr.client_data;
    async_read_result arr;
    if( pr.is_ok(poller::READ) ) {
        int r = = ::read(ard->fd, ard->buf, ard->requested_size);
        if( r == -1 ) {
            arr->tyupe = ERROR;
            arr->status.error_code = errno;
        } else if ( r == 0 ) {
            arr->readed_size = 0;
            arr->status = OK;
        } else {
            arr->readed_size = r;
            arr->status = OK;
        } 
    } else {
        arr->status = pr;
    }
    arr.buf = ard->buf;
    arr.requested_size = ard->readed_size;
    arr.fd = ard->fd;
    callback<async_read_result> callback = ard->callback;
    delete ard;
    
    callback(arr);
}

poll_tag async_read(poller P, int fd, char* buf, int size, int deadline, callback<async_read_result> cbk)
{
    async_read_data* ard = new async_read_data();
    ard->fd = fd;
    ard->buf = buf;
    ard->requested_size = size;
    ard->callback = cbk;
    P.wait(poller::READ, fd, deadline, make_callback(&async_read_callback), ard);
}

void async_write(poller, int fd, char* buf, int size, callback<async_read_result>)
{
}
