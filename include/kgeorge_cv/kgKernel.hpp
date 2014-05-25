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

#if !defined(KG_KERNEL_H_)
#define KG_KERNEL_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>
#include <random>




template <typename I>
struct AppropriateNonIntegralType {
    typedef float value_type;
};



template <>
struct AppropriateNonIntegralType<double> {
    typedef double value_type;
};

template<typename K>
struct EpsilonEq {
    static const K epsilon_;
    bool operator()(const K &l, const K &r) {
        return abs(l-r) < epsilon_;
    }
};

template<>
struct EpsilonEq<int> {
    typedef int K;
    static const int epsilon_=1;
    bool operator()(const int &l, const int &r) {
        return l == r;
    }
};

template<>
struct EpsilonEq<float> {
    typedef float K;
    static constexpr float epsilon_ =  0.000001f;
    bool operator()(const K &l, const K &r) {
        return abs(l-r) < epsilon_;
    }
};


template<>
struct EpsilonEq<double> {
    typedef double K;
    static constexpr double epsilon_ =  0.00000000001;
    bool operator()(const K &l, const K &r) {
        return abs(l-r) < epsilon_;
    }
};


#endif //KG_KERNEL_H_