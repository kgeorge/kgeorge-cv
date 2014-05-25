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

#if !defined(KG_COMMON_TRAITS_H_)
#define KG_COMMON_TRAITS_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>
#include <random>
#include <Eigen/Core>
#include <Eigen/Dense>

#include "kgKernel.hpp"

template<typename K>
struct Quantizer {
    static const K kEpsilon_;
    Quantizer(K eps=kEpsilon_):
    m_fEps(eps)
    {}
    K operator()( K valIn ) const
    {
        return round(valIn);
    }
    const K m_fEps;
};




template< typename T >
struct KgCommon_Traits {
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    static const int numElementsInT = -1;
    static constexpr int numElementsInTPlus1 = 3;
    typedef Eigen::Matrix<K, Eigen::Dynamic,  numElementsInTPlus1> TMatrix;
    static K distSqrd(const T &l, const T &r) {
        throw std::runtime_error( "not implemented" );
    }
    
    static T orthogonal(const T &) {
        throw std::runtime_error( "not implemented" );
    }
    
    static K dot(const T &l, const T &t) {
        throw std::runtime_error( "not implemented" );
    }
    static T &negate(T) {
        throw std::runtime_error( "not implemented" );
    }
    
    //returns  -1 for left, 0 for on , +1 for right
    static int leftRightOrOn(const T &a, const T &b, const T &c) {
        throw std::runtime_error( "not implemented" );
    }
    
    friend T & operator * ( T &l, K s) {
        throw std::runtime_error( "not implemented" );
    }

    static void fill( const T &arg, int idx,  TMatrix &target) {
        throw std::runtime_error( "not implemented" );
    }
};

template<typename T>
struct KgGeometricHash_Traits : public KgCommon_Traits<T>{

    template< typename LSHEntry>
    static void foo( const LSHEntry &a) {
        std::cout << "KgGeometricHash_Traits::foo" << std::endl;
    }

    template<typename LSHEntry>
    static const T &left( const LSHEntry &entry) {
        throw std::runtime_error( "not implemented" );
        return T();
    }


    template<typename LSHEntry>
    static const T &right( const LSHEntry &entry) {
        throw std::runtime_error( "not implemented" );
        return T();
    }


    template<typename LSHEntry>
    static T &left( LSHEntry &entry) {
        throw std::runtime_error( "not implemented" );
        return T();
    }


    template<typename LSHEntry>
    static T &right( LSHEntry &entry) {
        throw std::runtime_error( "not implemented" );
        return T();
    }
};

template<typename T>
struct KgLocalitySensitiveHash_Traits : public KgCommon_Traits<T>{};

template<typename T>
struct KgLeastSquaresTransform_Traits : public KgCommon_Traits<T>{};



#endif //KG_COMMON_TRAITS_H_