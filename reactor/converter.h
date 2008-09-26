//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
