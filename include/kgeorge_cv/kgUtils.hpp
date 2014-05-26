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
#include <sstream>


#include "kgKernel.hpp"


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
    template<typename T>
    T clamp(T val, T mni, T mxa) {
        return (val < mni)? mni : (val > mxa) ? mxa: val;
    }
    
    //A utility class that collects float samples
    //and keeps track of current, mean, variance and numSamples
    struct StatsMaker {
        float mean;
        int nSamples;
        float variance;
        float minVal, maxVal;
        StatsMaker(const std::string &name):name(name) {
            mean = 0.0f, variance = 0.0f, nSamples=0;
            minVal=std::numeric_limits<float>::max();
            maxVal=-std::numeric_limits<float>::max();
        }
        void addSample( float sample ) {
            float reciprocal = 1.0f /(nSamples + 1);
            float newMean = (mean * nSamples + sample) * reciprocal;
            float sumSq = (variance +  mean * mean) * nSamples + sample * sample;
            minVal = (sample < minVal) ? sample : minVal;
            maxVal = (sample > maxVal) ? sample : maxVal;
            variance = sumSq * reciprocal - (newMean * newMean);
            mean = newMean;
            nSamples +=1;
        }
        std::string name;

        void describe_(std::string &desc) const {
            std::stringstream ss;
            ss << name << ": mu=" << mean << ": variance=" << variance;
            ss << ": minVal=" << minVal <<": maxVal=" << maxVal << std::endl;
            desc = ss.str();
        }

        friend std::ostream &operator<< (std::ostream &o, const StatsMaker &s) {
            std::string desc;
            s.describe_(desc);
            o << desc;
            return o;
            
        }
    };



    template<typename T>
    struct Point2 {
        T r_;
        T c_;
        Point2():r_(0),c_(0){}
        Point2(T r, T c):r_(r),c_(c){}
        T get_r()const { return r_;}
        T get_c()const {return c_;}
        void set_r(T r){ r_ = r;}
        void set_c(T c){ c_ = c;}
    };

    typedef Point2<int> Point2i;
    typedef Point2<float> Point2f;

    template<typename T, typename C>
    struct Histogram1 {
        typedef C CountType;
        typedef T ValueType;
        typedef typename AppropriateNonIntegralType<T>::value_type R;
        typedef std::vector<C> THist;
        Histogram1(int nBins, T minSample, T maxSample):
            nSamples(0),
            nBins(nBins),
            minSample(minSample),
            maxSample(maxSample){
            assert(maxSample > minSample);
            binWidth_ = static_cast<R>(maxSample - minSample)/nBins;
            hist.assign(nBins, static_cast<C>(0));
        }
        virtual ~Histogram1(){
            hist.clear();
        }



        virtual void addSample(T sample) {
            assert(sample <= maxSample);
            assert(sample >= minSample);

            int whichBin = floor(static_cast<R>(sample - minSample)/binWidth_);
            if(whichBin >= nBins) {
                assert(sample == maxSample);
                whichBin -=1;
            }
            hist[whichBin] += static_cast<C>(1);
            nSamples++;
        }

        void describe( std::string &desc, bool verbose) const {
            std::stringstream ss;
            if(verbose) {
                ss << "[,";
            }
            T leftLimit, rightLimit;

            for(int i=0; i < nBins; ++i ) {
                leftLimit = minSample + i * binWidth_;
                rightLimit = minSample + (i +1) * binWidth_;
                if(verbose) {
                    ss << i << ":" << leftLimit << "<=" << hist[i] << "<" << rightLimit;
                    ss << ",  ";
                    if (i > 0 && i %9 ==0) {
                        ss << std::endl;
                    }
                } else {
                    ss << hist[i];
                    if( i < nBins -1) {
                        ss << ",";
                    }
                }
            }
            if(verbose) {
                ss << "  ]";
            }
            desc = ss.str();
        }

        R binRepresentative(int whichBin) {
            assert(whichBin < nBins);
            return (minSample + static_cast<R>(whichBin + 0.5) * binWidth_);
        }

        THist &get_hist(){ return hist;}


        THist hist;
        int nBins;
        T minSample;
        T maxSample;
        T binWidth_;
        int nSamples;
    };

    typedef Histogram1<float, int> Histogram1fi;

    template<int i, int n> struct Pow{
        enum { value = i * Pow<i, n-1>::value};
    };


    template<int i> struct Pow<i,1>{
        enum {value = i};
    };

}


#endif //KG_UTILS_H_