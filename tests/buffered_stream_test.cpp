#include "tinfra/test.h" // for test infra
#include "tinfra/buffered_stream.h" // we test this

#include <deque>

SUITE(tinfra) {

struct simple_mock_info {
    
};    
struct mock_input_stream: public tinfra::input_stream {
    int read(char* ptr, int size)
    {
        // record what was called
        calls.push_back(size);
        
        // and return prepared 
        CHECK(!results.empty());
        const char* res = results.front();
        results.pop_front();
        if( res == 0 ) {
            return 0;
        } else {
            const size_t len = strlen(res);
            CHECK((size_t)size >= len);
            memcpy(ptr, res, len);
            return len;
        }
    }
    
    void close() {}
    
    void will_return(const char* x) { results.push_back(x); }
    int  called_with() { 
        int result = calls.front(); 
        calls.pop_front(); 
        return result; 
    }
    bool no_more_calls() { return calls.empty(); }
    
    std::deque<const char*> results;
    std::deque<int> calls;
};

TEST(buffered_stream)
{
    
    mock_input_stream mock;
    mock.will_return("ab");
    
    
    tinfra::buffered_input_stream bs(mock, 4);
    {
        char buf[16] = {0};
        CHECK_EQUAL(2, bs.read(buf, 4));
        CHECK_EQUAL("ab", buf);
        
        CHECK_EQUAL(4, mock.called_with());
        CHECK(mock.no_more_calls());
    }
    
    mock.will_return("d");
    {
        char buf[16] = {0};
        CHECK_EQUAL(1, bs.read(buf, 4));
        CHECK_EQUAL("d", buf);
        
        CHECK_EQUAL(4, mock.called_with());
        CHECK(mock.no_more_calls());
    }
    
    // check that subsequent
    // small read will provoke one read
    mock.will_return("123x");
    {
        char buf[16] = {0};
        CHECK_EQUAL(1, bs.read(buf, 1));
        CHECK_EQUAL("1", buf);
        CHECK_EQUAL(1, bs.read(buf, 1));
        CHECK_EQUAL("2", buf);
        
        CHECK_EQUAL(1, bs.read(buf, 1));
        CHECK_EQUAL("3", buf);
        
        CHECK_EQUAL(4, mock.called_with());
        CHECK(mock.no_more_calls());
    }
    // now test that buffer will be reused
    // and remaining "bigger" than buffer part
    // will be readed directly
    mock.will_return("2345abcde");
    {
        char buf[16] = {0};
        CHECK_EQUAL(10, bs.read(buf, 10));
        CHECK_EQUAL("x2345abcde", buf);
        
        CHECK_EQUAL(9, mock.called_with());
        CHECK(mock.no_more_calls());
    }
    
    // now check if partial buffer
    // will be correctly handler
    mock.will_return("abcd");
    mock.will_return("1234");
    {
        char buf[16] = {0};
        CHECK_EQUAL(2, bs.read(buf, 2));
        CHECK_EQUAL(4, mock.called_with());
        CHECK_EQUAL("ab", buf);
        
        CHECK_EQUAL(4, bs.read(buf, 4));
        CHECK_EQUAL("cd12", buf);
        CHECK_EQUAL(4, mock.called_with());
        
        CHECK_EQUAL(2, bs.read(buf, 2));
        CHECK_EQUAL("3412", buf);
        
        CHECK(mock.no_more_calls());
    }
}

}
