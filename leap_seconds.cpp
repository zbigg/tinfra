#include <time.h>
#include <stdio.h>

void print_time(const char* syn, time_t t)
{
    struct tm exploded_time;
    gmtime_r(&t, &exploded_time);
    char strtime_buf[256];
    strftime(strtime_buf, sizeof(strtime_buf), "%Y.%m.%d %H:%M:%S", &exploded_time);
    
    printf("%s -> %s, %ld\n", syn, strtime_buf, (long)t);
}

int main()
{
    //tzset();
    
    struct tm t;
    t.tm_year = 2012-1900;
    t.tm_mon  = 5;
    t.tm_mday  = 30;
    t.tm_hour = 23;
    t.tm_min  = 59;
    t.tm_sec  = 58;
    
    time_t t0 = timegm(&t);
    time_t t1 = t0+1;
    time_t t2 = t0+2;
    time_t t3 = t0+3;
    
    print_time("t0       ", t0);
    print_time("t1 = t0+1", t1);
    print_time("t2 = t0+2", t2);
    print_time("t3 = t0+3", t3);
}
