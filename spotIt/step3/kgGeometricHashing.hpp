/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/

#if !defined(KG_GEOMETRIC_HASHING_H_)
#define KG_GEOMETRIC_HASHING_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>
#include <random>



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


template< typename TTemplate >
struct KgGeometricHashing_Traits;

template<typename T, typename HashEntry, int n >
struct Hash {
    typedef KgGeometricHashing_Traits<T> TTraits;
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    Hash(K w):w(w),gen(42),dist(std::normal_distribution<>(0.0, 1)) {
        throw std::runtime_error( "not implemented" );
    }
    void index( const T &arg, const HashEntry & entry) {
        throw std::runtime_error( "not implemented" );
    }
    std::normal_distribution<> dist;
    std::mt19937 gen;
    T a[n];
    K w;
};

template< typename T >
struct KgGeometricHashing_Traits {
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
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

template< typename TTemplate, typename TTraits=KgGeometricHashing_Traits<TTemplate> >
struct TemplateExtra_ {
    typedef typename TTraits::T T;
    typedef typename TTraits::K K;
    K minDistSqrdBetweenPoints_;
    K maxDistSqrdBetweenPoints_;
    K avgDistSqrdBetweenPoints_;
    K varianceDistSqrd_;

    TemplateExtra_():
    minDistSqrdBetweenPoints_(std::numeric_limits<K>::max()),
    maxDistSqrdBetweenPoints_(-std::numeric_limits<K>::max()),
        avgDistSqrdBetweenPoints_(0),
        varianceDistSqrd_(0){}

    template<typename ForwardIter>
    void compute(ForwardIter b, ForwardIter e);

};


template<typename TTemplate, typename TTraits>
template<typename ForwardIter>
void TemplateExtra_<TTemplate, TTraits>::compute(ForwardIter b, ForwardIter e) {
    ForwardIter it1, it2;
    avgDistSqrdBetweenPoints_ =0;
    K sumOfSquareOfDistSqrd = 0;
    size_t numItems =0;
    for( it1 = b; it1 != e; ++it1 ) {
        for(it2 = (it1+1); it2 != e; ++it2,  ++numItems) {
            K sqDist = TTraits::distSqrd(*it1, *it2);
            minDistSqrdBetweenPoints_ = (sqDist < minDistSqrdBetweenPoints_) ? sqDist: minDistSqrdBetweenPoints_;
            maxDistSqrdBetweenPoints_ = (sqDist > maxDistSqrdBetweenPoints_) ? sqDist: maxDistSqrdBetweenPoints_;
            avgDistSqrdBetweenPoints_ += sqDist;
            sumOfSquareOfDistSqrd += (sqDist * sqDist);
        }
    }
    if(b != e) {
        K sizeReciprocal = 1.0/numItems;
        avgDistSqrdBetweenPoints_ *= sizeReciprocal;
        varianceDistSqrd_ = (sumOfSquareOfDistSqrd *  sizeReciprocal) - (avgDistSqrdBetweenPoints_  * avgDistSqrdBetweenPoints_ );
    }
}


template< typename TTemplate, typename Q  >
class KgGeometricHashing {
public:
    //Eg: if the template type is a vector of Point-s
    //T is Point

    typedef typename TTemplate::value_type T;
    typedef  KgGeometricHashing_Traits<T> TTraits;
    typedef typename TTraits::K K;

    KgGeometricHashing(){

        maxValX = -std::numeric_limits<K>::max();
        minValX = std::numeric_limits<K>::max();

        maxValY = -std::numeric_limits<K>::max();
        minValY = std::numeric_limits<K>::max();
    }

    //ForwardIter value_type should be TTemplate
    template<typename ForwardIter>
    KgGeometricHashing(ForwardIter b, ForwardIter e){
        
        maxValX = -std::numeric_limits<K>::max();
        minValX = std::numeric_limits<K>::max();
        
        maxValY = -std::numeric_limits<K>::max();
        minValY = std::numeric_limits<K>::max();
        addTemplates(b, e);
    }

    //ForwardIter value_type should be TTemplate
    template<typename ForwardIter>
    void addTemplates( ForwardIter b, ForwardIter e ) {
        while(b != e) {
            //store pointers to the template
            queue_.push_back(&(*b));
            b++;
        }
    }

    ~KgGeometricHashing(){
        queue_.clear();
    }

    template<typename HashEntry>
    void processTemplateSet(K w, Hash<T, HashEntry, 3> &hash);

    K maxValX;
    K minValX;

    K maxValY;
    K minValY;

protected:
    //note that we keep pointers to the point set
    //Eg: if you keep your Template as a vector of Points,
    //we keep a pointer to the template.
    std::deque<const TTemplate *> queue_;

    //computed during processTemplate
    std::deque< TemplateExtra_<TTemplate,TTraits> > templateExtras_;
};


template< typename TTemplate, typename Q>
template<typename HashEntry>
void KgGeometricHashing<TTemplate, Q >::processTemplateSet(K w, Hash<T, HashEntry, 3> &hash) {
    for(auto qit= queue_.begin(); qit != queue_.end(); ++qit) {
        const TTemplate *t = *qit;
        TemplateExtra_<TTemplate, TTraits> extra;
        extra.compute(t->begin(), t->end());
        std::less< T > lessT;
        Q quantizer;
        K limitDistanceBetweenPoints =  4;
        T templateCentroid;
        if(t->begin() != t->end()) {
            int numPoints = t->end() - t->begin();
            K reciprocal = static_cast<K> (1.0)/numPoints;
            for(auto pointIt =  t->begin(); pointIt != t->end(); ++pointIt ) {
                templateCentroid = templateCentroid +  *pointIt;
            }
            templateCentroid = templateCentroid  * reciprocal;
        }

        for(auto pointIt =  t->begin(); pointIt != t->end(); ++pointIt ) {
            auto nextPointIt = pointIt + 1;
            for( ; nextPointIt != t->end(); ++nextPointIt) {
                if( TTraits::distSqrd(*pointIt, *nextPointIt) >= limitDistanceBetweenPoints ) {
                    T l(*pointIt), r(*nextPointIt);
                    int isLeftRightOrOn = TTraits::leftRightOrOn(l, r, templateCentroid);
                    if( isLeftRightOrOn == 0) {
                        continue;
                    } else if( isLeftRightOrOn > 1) {
                         std::swap(l, r);
                    }
                    T diff = r-l;
                    T centroidOfBase = (r + l) * static_cast<K>(0.5);
                    K d = TTraits::distSqrd(r, l);
                    assert(d > EpsilonEq<K>::epsilon_);
                    K oneByLngth = 1.0/sqrt(d);
                    T xAxis  =  diff * oneByLngth;
                    T yAxis = TTraits::orthogonal(diff) * oneByLngth;
                    T xAxisNeg(xAxis);
                    T yAxisNeg(yAxis);

                    TTraits::negate(xAxisNeg);
                    TTraits::negate(yAxisNeg);

                    auto allPointsIt = t->begin();
                    for( allPointsIt = t->begin(); allPointsIt != t->end(); ++allPointsIt ) {
                        const T &currentPoint = *allPointsIt;
                        T currentDiff = currentPoint - centroidOfBase;
                        T projection(TTraits::dot(currentDiff, xAxis) * oneByLngth, TTraits::dot(currentDiff, yAxis) * oneByLngth);
                        //T projection2( TTraits::dot(currentDiff, xAxisNeg)*oneByLngth, TTraits::dot(currentDiff, yAxisNeg) *oneByLngth);
                        //std::cout << "x: " << projection.x <<  ", " << quantizer(  projection.x  ) << ", xNeg: " << projection2.x <<   ", y: " << projection.y <<  ", " << quantizer(  projection.y  ) << ", yNeg:" << projection2.y  << std::endl;
                        maxValX = (quantizer(projection.x) > maxValX) ? quantizer(projection.x) : maxValX;
                        maxValY = (quantizer(projection.y) > maxValY) ? quantizer(projection.y) : maxValY;
                        minValX = (quantizer(projection.x) < minValX) ? quantizer(projection.x) : minValX;
                        minValY = (quantizer(projection.y) < minValY) ? quantizer(projection.y) : minValY;
                        HashEntry he(*pointIt, *nextPointIt);
                        //hash.index(projection, he);

                    }
                }
                
            }
        }
    }
    //std::cout << "///////////////////////////////////////////////////" << std::endl;
    //std::cout << "maximum:  x: " << maxValX <<  ", " << maxValY << ", minimum: " << minValX <<   ",  " << minValY << std::endl;

}

#endif //KG_GEOMETRIC_HASHING_H_