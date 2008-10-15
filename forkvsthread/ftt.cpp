#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <iterator>
#include <vector>
#include <algorithm>


#include "tinfra/thread.h"
#include "tinfra/runner.h"
#include "tinfra/cmd.h"
#include "tinfra/fmt.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/time.h>

class Timer {
    std::string _message;
    size_t   _start;
    size_t   _end;
    bool _running;
public:
    Timer(const char* message): 
        _message(message),
        _start(0),
        _end(0),
        _running(false)
    {
        start();
    }
    ~Timer()
    {
        if( !stopped()) {
            stop();
            report();
        }
    }
    
    void start()
    {
        _start = now();
        _running = true;
    }
    void stop()
    {
        _end = now();
        _running = false;
    }
    
    void report()
    {
        size_t end = _end;
        if( _running )
            end = now();
        int r = (end-_start);
        int s = r/1000000;
        int us = r % 1000000;
        std::cout << "time benchmark: " << _message << ": " <<  s << "." << std::setfill('0') << std::setw(6) << us << std::endl; 
    }
    
    bool stopped() const { return ! _running; }
    
private:
    size_t now()
    {
        timeval tv;
        if( gettimeofday(&tv,0) == -1 ) 
            throw std::runtime_error("gettimeofday failed");
        
        size_t r = tv.tv_sec * 1000000 + tv.tv_usec;
        return r;
    }
};


struct SearchJobParams {
    // in
    int job;
    
    const char* pattern;
    const char* start;
    const char* end;
    
    // out
    int result;
};

class SearchJob: public tinfra::Runnable {
    SearchJobParams& params;    
public:
    SearchJob(SearchJobParams& params): params(params) {}
    
    void run()
    {
        // TODO: implement
        Timer tt(tinfra::fmt("job[%i]") % params.job);
        size_t result = 0;
        for( const char* s = params.start; s < params.end; ++s ) {
            result += *s;
            if( rand() % 1000 == 321 ) {
                char* p = 0;
                *p = 2;
            }
        }
        params.result = result;
    }
    
    int get_result()
    {
        return params.result;
    }
};

class JobRunner: public tinfra::Runner {
public:
    virtual void wait(std::vector<int>& results) = 0;
};

int getJobResult(tinfra::Runnable* r)
{
    SearchJob* sj = dynamic_cast<SearchJob*>(r);
    if( sj ) {
        return sj->get_result();
    } else {
        throw std::logic_error("bad type, expected SearchJob");
    }
}

class SubprocessRunner: public JobRunner {
    std::vector<pid_t> children;
public:
    virtual void run(tinfra::Runnable* runnable)
    {
        int r = ::fork();
        if( r > 0 ) {
            children.push_back(r);
            return;
        }
        if( r == 0 ) {
            runnable->run();
            int r = getJobResult(runnable);
            exit(r);
        }
        throw std::runtime_error("fork failed");
    }
    
    void wait(std::vector<int>& results)
    {
        for( int i = children.size()-1; i >= 0 ; --i ) {
            int exit_code;
            int r = ::waitpid(children[i], &exit_code, 0);
            if( r == -1 )
                throw std::runtime_error("waitpid failed");
            if( WIFSIGNALED(exit_code) ) {
               results.push_back(- WTERMSIG(exit_code) );
            } else {
                results.push_back(WEXITSTATUS(exit_code));
            }
        } 
        children.clear();
    }
};

class ThreadRunner: public JobRunner {
    tinfra::ThreadSet ts;
    std::vector<tinfra::Runnable*> jobs;
public:
    virtual void run(tinfra::Runnable* job)
    {
        ts.start(*job);
        jobs.push_back(job);
    }
    
    void wait(std::vector<int>& results)
    {
        ts.join();
        for( size_t i = 0; i < jobs.size(); ++i ) {
            results.push_back(getJobResult(jobs[i]));
        }
    }
};

std::string test_string;

void test(JobRunner& r, const int job_count)
{
    const size_t tasks_size = test_string.size();
    const size_t job_size = tasks_size / job_count;
    if( job_size * job_count != tasks_size )
        throw std::logic_error("bad jobs count");
    
    SearchJobParams jobs[job_count];
     
    for( int i = 0; i < job_count; ++i ) {
        jobs[i].job     = i;
        jobs[i].pattern = "ABC";
        jobs[i].start = test_string.data();
        jobs[i].end   = test_string.data() + job_size;
        
        jobs[i].result = -1;
        
        r.run(new SearchJob(jobs[i]));
    }
    
    std::vector<int> result;
    
    r.wait(result);
    
    std::cout << "result: ";
    std::copy(result.begin(), result.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";
}

int ftt_main(int argc, char** argv)
{
    Timer ta("program");
    srand(10);
    
    const int size=1024*1024*64; // 512MB
    {
        Timer tt("init");
        test_string.reserve(size);
        for( int i = 0; i < size; ++i ) {
            char c = (i % 'Z'-'A') + 'A';
            test_string.append(1, c);
        }
    }
    
    
    
    if( false) {
        Timer tt("threads_all");
        ThreadRunner tr;
        test(tr, 16);
    }
    {
        Timer tt("process_all");
        SubprocessRunner tr;
        test(tr, 16);
    }
    return 0;
}

TINFRA_MAIN(ftt_main);

