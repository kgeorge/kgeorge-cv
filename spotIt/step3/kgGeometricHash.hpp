/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/

#if !defined(KG_GEOMETRIC_HASH_H_)
#define KG_GEOMETRIC_HASH_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>
#include <random>



#include "kgUtils.hpp"
#include "kgKernel.hpp"
#include "kgLocalitySensitiveHash.hpp"

template< typename TTemplate, typename TTraits=KgGeometricHash_Traits<TTemplate> >
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
class KgGeometricHash {
public:
    //Eg: if the template type is a vector of Point-s
    //T is Point
    
    typedef typename TTemplate::value_type T;
    typedef  KgGeometricHash_Traits<T> TTraits;
    typedef typename TTraits::K K;
    
    KgGeometricHash(){
        
        maxValX = -std::numeric_limits<K>::max();
        minValX = std::numeric_limits<K>::max();
        
        maxValY = -std::numeric_limits<K>::max();
        minValY = std::numeric_limits<K>::max();
    }
    
    //ForwardIter value_type should be TTemplate
    template<typename ForwardIter>
    KgGeometricHash(ForwardIter b, ForwardIter e){
        
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
    
    ~KgGeometricHash(){
        queue_.clear();
    }
    
    template<typename LSHashEntry, typename LSHashTraits>
    void processTemplateSet(K w, LocalitySensitiveHash<T, LSHashEntry, 3, LSHashTraits> &lsHash);

    template<typename LSHashEntry, typename LSHashTraits>
    void queryTemplateSet(
        LocalitySensitiveHash<T, LSHashEntry, 3, LSHashTraits> &lsHash,
        std::vector<std::map<int,int>> &templateMatches);

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
template<typename LSHashEntry, typename LSHashTraits>
void KgGeometricHash<TTemplate, Q >::processTemplateSet(K w, LocalitySensitiveHash<T, LSHashEntry, 3, LSHashTraits> &lsHash) {
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
                        maxValX = (quantizer(projection.x) > maxValX) ? quantizer(projection.x) : maxValX;
                        maxValY = (quantizer(projection.y) > maxValY) ? quantizer(projection.y) : maxValY;
                        minValX = (quantizer(projection.x) < minValX) ? quantizer(projection.x) : minValX;
                        minValY = (quantizer(projection.y) < minValY) ? quantizer(projection.y) : minValY;
                        LSHashEntry he(l, r);
                        lsHash.index(projection, he);
                    }
                }
                
            }
        }
    }
    //std::cout << "///////////////////////////////////////////////////" << std::endl;
    //std::cout << "maximum:  x: " << maxValX <<  ", " << maxValY << ", minimum: " << minValX <<   ",  " << minValY << std::endl;
    
}



template< typename TTemplate, typename Q>
template<typename LSHashEntry, typename LSHashTraits>
void KgGeometricHash<TTemplate, Q >::queryTemplateSet(
    LocalitySensitiveHash<T, LSHashEntry, 3, LSHashTraits> &lsHash,
    std::vector<std::map<int,int>> &templateMatches) {
    std::cout << "querying " << std::endl;
    templateMatches.resize(queue_.size());
    int queueIdx =0;
    for(auto qit= queue_.begin(); qit != queue_.end(); ++qit, ++queueIdx) {
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
                    std::less<LSHashEntry> le;
                    std::map<LSHashEntry, int, std::less<LSHashEntry> > templateMatch(le);
                    for( allPointsIt = t->begin(); allPointsIt != t->end(); ++allPointsIt ) {
                        const T &currentPoint = *allPointsIt;
                        T currentDiff = currentPoint - centroidOfBase;
                        T projection(TTraits::dot(currentDiff, xAxis) * oneByLngth, TTraits::dot(currentDiff, yAxis) * oneByLngth);
                        maxValX = (quantizer(projection.x) > maxValX) ? quantizer(projection.x) : maxValX;
                        maxValY = (quantizer(projection.y) > maxValY) ? quantizer(projection.y) : maxValY;
                        minValX = (quantizer(projection.x) < minValX) ? quantizer(projection.x) : minValX;
                        minValY = (quantizer(projection.y) < minValY) ? quantizer(projection.y) : minValY;
                        LSHashEntry he(l, r);
                        lsHash.query(projection, he, templateMatch);
                    }
                    int maxCount =0;
                    auto titWithMaxCount = templateMatch.begin();
                    for(auto tit = templateMatch.begin(); tit != templateMatch.end(); ++tit) {
                        if( maxCount < tit->second ) {
                            maxCount = Kg::max<int>(tit->second, maxCount);
                            titWithMaxCount = tit;
                        }
                    }

                    std::cout << titWithMaxCount->first << ", " << titWithMaxCount->second << std::endl;
                }

            }
        }
    }
    //std::cout << "///////////////////////////////////////////////////" << std::endl;
    //std::cout << "maximum:  x: " << maxValX <<  ", " << maxValY << ", minimum: " << minValX <<   ",  " << minValY << std::endl;

}

#endif //KG_GEOMETRIC_HASH_H_