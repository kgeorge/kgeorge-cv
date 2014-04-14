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
#include <string>


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
    
    //A utility class that collects float samples
    //and keeps track of current, mean, variance and numSamples
    struct StatsMaker {
        float mean;
        int n;
        float variance;
        StatsMaker(const std::string &name):name(name) {
            mean = 0.0f, variance = 0.0f, n=0;
        }
        void addSample( float sample ) {
            float reciprocal = 1.0f /(n + 1);
            float newMean = (mean * n + sample) * reciprocal;
            float sumSq = (variance +  mean * mean) * n + sample * sample;
            variance = sumSq * reciprocal - (newMean * newMean);
            mean = newMean;
            n +=1;
        }
        std::string name;
        
        friend std::ostream &operator<< (std::ostream &o, const StatsMaker &s) {
            o << s.name << ": mu=" << s.mean << ": variance=" << s.variance <<  std::endl;
            return o;
            
        }
    };
}


#endif //KG_UTILS_H_