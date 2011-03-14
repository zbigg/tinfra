


struct SM: public state_machine<SM> {
    struct start_state {
    };

    struct collecting_state {
        std::string collected;
    };

    struct finished_state {
        std::string result;
    };
    
    void operator() (start_state& d, char c)
    {
        collecting_state n(d);
        
        n.collected.append(1,c);
        
        next(n);
    }

    void operator() (collecting_state& d, char c)
    {
        if( c == '!' ) {
            finished_state ff;
            ff.result = d.collected;
            next(ff);
        } else {
            d.collected.append(d);
        }
        next(d);
    }
};

int main()
{
    SM.process(c);
}
