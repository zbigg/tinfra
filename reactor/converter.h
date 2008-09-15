#ifndef __theg_converter_h__
#define __theg_converter_h__

template <typename WF>
struct Converter {
    template <typename FROM, typename TO>
    static void convert(FROM& source, TO& dest);
    
    template <typename T>
    static void convert(T& source, TO& dest) { dest = source; }
};

#endif
