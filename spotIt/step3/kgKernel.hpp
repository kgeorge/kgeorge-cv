/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
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
#include <Eigen/Core>
#include <Eigen/Dense>



#include "kgUtils.hpp"

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

template <typename I>
struct AppropriateNonIntegralType {
    typedef float value_type;
};


template <>
struct AppropriateNonIntegralType<double> {
    typedef double value_type;
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