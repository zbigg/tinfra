//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <vector>
#include
#include <poll.h>


template<T>
class array {    
public:
    typedef typename T value_type;
    
    typedef value_type* iterator;
    typedef value_type const* const_iterator;
    typedef std::size_t size_type;

    iterator begin() { return _begin; }
    const_iterator begin() const { return _begin; }
    
    iterator end() { return _end; }
    const_iterator end() const { return _end; }
    
    size_type size() const { return _end - _begin; }
    
    void size(int new_size) {
        if( size() != new_size ) {
            iterator* new_begin = new T[new_size];
            
            if( size() > 0 ) {
                size_type common_size = std::min(size(), new_size);
                std::copy(_begin, _begin + common_size, new_begin);
                delete [] _begin;
            }            
            _begin = new_begin;
            _end = _begin + size;
        }
    }
    
    array(size_type n = 0): _begin(0), _end(0) { size(n); }
    array(array<T> const& other): _begin(0), _end(0) { size(other.size(); copy(other.begin(), other.end(), begin());
    ~array() { delete [] _begin; }
    
    T&       at(size_type n)       { return *(_begin + n); }
    T const& at(size_type n) const { return *(_begin + n); }
    
    T&       operator[] (size_type n)       { return at(n); }
    T const& operator[] (size_type n) const { return at(n); }
    
private:
    iterator* _begin;
    iterator* _end;
};

struct IOChannel {
    int filedesc;    
    
    virtual void failure
};

struct IOContext {
    std::vector<IOChannel*> channels;
    
    int timeout;
};

void make_fds(std::vector<IOChannel> const& channels, array<pollfd>& result)
{    
    result.size(channels.size());
    
    for(std::vector<IOChannel>::const_iterator i = channels.begin(); i != channels.end(); ++i )
    {
        result[i].fd = i->filedesc;
        result[i].events = wanted_state;
        result[i].revents = POLLIN | POLLOUT;
    }
}
int loop(IOContext& r)
{
    array<pollfd> pollfds;
    while( true ) {
        make_fds(pollfds, r.channels);
        
        int r = poll(pollfds.begin(), r.channels.size(), r.timeout);
        if( r == 0 ) {
            continue;
        }
        if( r == -1 ) {
            perror("loop: poll failed, retrying");
            continue;
        }
        
        int i = 0;
        for(pollfd* pfd = pollds.begin(); pfd != pollfds.end(); ++pfd,++i ) {
            if( pfd->revents == 0 ) continue;
            
            if( (pfd->revents & (POLLERR | POLLHUP)) != 0 ) {
                r.channels[i].failure();
                continue;
            }
            if( (pfd->revents && POLLIN  ) == POLLIN ) {
                r.channels[i].data_available();
            }
            if( (pfd->revents && POLLOUT ) == POLLOUT) {
                r.channels[i].write_possible();
            }            
        }
    }
    delete[] pollfds;
}

int main(int argc, char** argv)
{
}