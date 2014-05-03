/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided

/*
Copyright (c) 2014, kgeorge
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the {organization} nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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

    template<int i, int n> struct Pow{
        enum { value = i * Pow<i, n-1>::value};
    };


    template<int i> struct Pow<i,1>{
        enum {value = i};
    };

}


#endif //KG_UTILS_H_