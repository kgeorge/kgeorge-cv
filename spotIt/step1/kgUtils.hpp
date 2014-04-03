/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/
#if !defined(KG_UTILS_H_)
#define KG_UTILS_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <unordered_set>
#include <set>

#include <opencv2/core/core.hpp>



#if defined(DEBUG)

#define KG_DBGOUT( x ) x

#else

#define KG_DBGOUT( x )

#endif

namespace Kg {
    
    
    inline float uRand0To1() {
        return static_cast<float>(rand())/RAND_MAX;
    }
    
    template<typename T >
    T min( T a, T b ) {
        return a <= b ? a: b;
    }
    
    
    template<typename T >
    T max( T a, T b ) {
        return a >= b ? a: b;
    }
}


#endif //KG_UTILS_H_