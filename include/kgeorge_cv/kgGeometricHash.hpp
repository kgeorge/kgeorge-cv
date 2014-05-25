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

#if !defined(KG_GEOMETRIC_HASH_H_)
#define KG_GEOMETRIC_HASH_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>
#include <random>

#include <Eigen/Core>
#include <Eigen/Dense>




#include "kgCommonTraits.hpp"
#include "kgUtils.hpp"
#include "kgLocalitySensitiveHash.hpp"
#include "kgLeastSquaresTransform.hpp"

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

template< typename TTemplate>
struct PointCorrespondence {
    typedef typename TTemplate::value_type T;
    typedef int PointIdInQueryModel;
    typedef  KgGeometricHash_Traits<T> TTraits;
    typedef typename TTraits::K K;
    struct Record {
        Record(const T& templatePoint, int count):templatePoint(templatePoint),count(count){}
        Record & operator=(const Record &rhs) {
            templatePoint = rhs.templatePoint;
            count = rhs.count;
            return *this;
        }
        //Record(const Record &rhs):templatePoint(rhs.templatePoint),count(rhs.count){}
        const T &templatePoint;
        int count;
     };
    typedef typename std::map<PointIdInQueryModel, Record>::value_type value_type;
    PointCorrespondence(const TTemplate &refData, const T &centroid):refData(refData),templateCentroid(centroid){}
    const TTemplate &refData;
    T templateCentroid;
    std::map<PointIdInQueryModel, Record>  data;
    K foo(Eigen::MatrixXf &transform) {
        auto cit = data.begin();
        int idx=0;
        Eigen::Matrix<K, Eigen::Dynamic, 3>A(data.size(), 3), B(data.size(), 3);
        for( ; cit != data.end(); ++cit, ++idx) {
            int pointId = cit->first;
            T pt = refData[pointId];
            pt -=  templateCentroid;
            Record &rec = cit->second;
            TTraits::fill(pt, idx, A );
            //std::cout << rec.templatePoint << std::endl;
            TTraits::fill(rec.templatePoint, idx, B );
        }
        LeastSquaresTransform<T> lst;
        /*
        std::cout << "A" << std::endl;
        std::cout << A;
        std::cout << "~~~~~~~~~~~~~" << std::endl;
        std::cout << B;
        std::cout << std::endl;
         */
        lst(A, B, transform);
        std::cout << "transform: " << transform << std::endl;
        Eigen::Matrix<K, Eigen::Dynamic, 3> C = A * transform;
        return (C - B).norm();
    }


};

template< typename TTemplate, typename Q  >
class KgGeometricHash {
public:
    //Eg: if the template type is a vector of Point-s
    //T is Point
    
    typedef typename TTemplate::value_type T;
    typedef  KgGeometricHash_Traits<T> TTraits;
    typedef typename TTraits::K K;
    typedef PointCorrespondence<TTemplate> TPointCorrespondence;
    typedef typename TPointCorrespondence::Record TPointCorrespondenceRecord;
    static const int kLSHashNumFields=3;
    static const int kLSHashNumBuckets=1;
    
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
    void processTemplateSet(K w, LocalitySensitiveHash<T, LSHashEntry, kLSHashNumFields, kLSHashNumBuckets, LSHashTraits> &lsHash);

    template<typename LSHashEntry, typename LSHashTraits>
    void queryTemplateSet(
        LocalitySensitiveHash<T, LSHashEntry, kLSHashNumFields, kLSHashNumBuckets, LSHashTraits> &lsHash,
        std::vector<std::map<int,int>> &templateMatches,
        std::map<int, std::vector<cv::Point2f> > &pointClusterMap);

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
void KgGeometricHash<TTemplate, Q >::processTemplateSet(K w, LocalitySensitiveHash<T, LSHashEntry, kLSHashNumFields, kLSHashNumBuckets, LSHashTraits> &lsHash) {
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
                    T lMinusCentroid = l - templateCentroid;
                    T rMinusCentroid = r - templateCentroid;
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
                        LSHashEntry he(lMinusCentroid, rMinusCentroid);
                        int hashTableIndicesCalculated[kLSHashNumBuckets ];
                        lsHash.index(projection, he, hashTableIndicesCalculated);
                    }
                }
            }
        }
    }
}



template< typename TTemplate, typename Q>
template<typename LSHashEntry, typename LSHashTraits>
void KgGeometricHash<TTemplate, Q >::queryTemplateSet(
    LocalitySensitiveHash<T, LSHashEntry, kLSHashNumFields, kLSHashNumBuckets, LSHashTraits> &lsHash,
    std::vector<std::map<int,int>> &templateMatches,
    std::map<int, std::vector<cv::Point2f>> &pointClusterMap ) {
    std::cout << "querying " << std::endl;
    templateMatches.resize(queue_.size());
    int queueIdx =0;
    for(auto qit= queue_.begin(); qit != queue_.end(); ++qit, ++queueIdx) {
        const TTemplate *t = *qit;
        std::map<int, TPointCorrespondence > pointCorrespondenceMap;
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
        } else {
            continue;
        }

        int lPointIdx =0;
        for(auto pointIt =  t->begin(); pointIt != t->end(); ++pointIt, ++lPointIdx) {
            auto nextPointIt = pointIt + 1;
            int rPointIdx = lPointIdx + 1;
            for( ; nextPointIt != t->end(); ++nextPointIt, ++rPointIdx) {
                if( TTraits::distSqrd(*pointIt, *nextPointIt) >= limitDistanceBetweenPoints ) {
                    pointCorrespondenceMap.clear();
                    T l(*pointIt), r(*nextPointIt);
                    int isLeftRightOrOn = TTraits::leftRightOrOn(l, r, templateCentroid);
                    if( isLeftRightOrOn == 0) {
                        continue;
                    } else if( isLeftRightOrOn > 1) {
                        std::swap(l, r);
                        std::swap(lPointIdx, rPointIdx);
                    }

                    T lMinusCentroid = l - templateCentroid;
                    T rMinusCentroid = r - templateCentroid;
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
                    std::less<LSHashEntry*> le;
                    std::map<const LSHashEntry *, int, std::less<LSHashEntry*> > templateMatch(le);
                    for( allPointsIt = t->begin(); allPointsIt != t->end(); ++allPointsIt ) {
                        const T &currentPoint = *allPointsIt;
                        T currentDiff = currentPoint - centroidOfBase;
                        T projection(TTraits::dot(currentDiff, xAxis) * oneByLngth, TTraits::dot(currentDiff, yAxis) * oneByLngth);
                        maxValX = (quantizer(projection.x) > maxValX) ? quantizer(projection.x) : maxValX;
                        maxValY = (quantizer(projection.y) > maxValY) ? quantizer(projection.y) : maxValY;
                        minValX = (quantizer(projection.x) < minValX) ? quantizer(projection.x) : minValX;
                        minValY = (quantizer(projection.y) < minValY) ? quantizer(projection.y) : minValY;
                        LSHashEntry he(lMinusCentroid, rMinusCentroid);
                        std::deque< std::pair< const T *, const LSHashEntry *> > queryResults;
                        lsHash.query(projection,  queryResults);
                        for(auto qit = queryResults.begin(); qit != queryResults.end(); ++qit) {
                            auto tmatch = templateMatch.find(qit->second);
                            if(tmatch == templateMatch.end()) {
                                templateMatch.insert(std::pair<const LSHashEntry*, int>(qit->second, qit->second->count));
                            } else {
                                tmatch->second += qit->second->count;
                            }
                        }
                    }
                    int maxCount =0;
                    std::map< int, std::pair<const LSHashEntry*, int> > templateResults;
                    auto titWithMaxCount = templateMatch.begin();
                    for(auto tit = templateMatch.begin(); tit != templateMatch.end(); ++tit) {
                        auto tResultIt = templateResults.find(tit->first->templateId);
                        if ( tResultIt == templateResults.end()) {
                            const LSHashEntry *e = tit->first;
                            std::pair<int, std::pair<const LSHashEntry*, int> > insertData( tit->first->templateId, std::pair<const LSHashEntry*, int>(e, tit->second));
                            templateResults.insert( insertData );
                            //std::cout << tit->first.templateId << ", " <<   e.l << ", " << e.r << std::endl;
                        } else {
                            std::pair<const LSHashEntry*, int> & tResultItSecond = tResultIt->second;
                            if( tResultItSecond.second < tit->second ) {
                                const LSHashEntry *e = tit->first;
                                tResultIt->second = std::pair<const LSHashEntry*, int>(e, tit->second);
                                //std::cout << tit->first.templateId << ", " <<   e.l << ", " << e.r << std::endl;
                            }
                        }
                    }
                    /*
                    for(auto tit = templateMatch.begin(); tit != templateMatch.end(); ++tit) {
                        std::cout << *(tit-> first) << ", " << tit-> second << std::endl;
                    }
                    */
                    std::cout << "~~~~~~~~~~~~" << std::endl;
                    
                    for(auto tResultIt = templateResults.begin(); tResultIt != templateResults.end(); ++tResultIt) {
                        std::cout << tResultIt->first << ", " << *(tResultIt->second.first) << ", " << tResultIt->second.second << std::endl;
                    }
                    /*
                    for(auto tResultIt = templateResults.begin(); tResultIt != templateResults.end(); ++tResultIt) {
                        int possibleTemplateId = tResultIt->first;
                        //std::cout << tResultIt->first << ", " << tResultIt->second.first << ", " << tResultIt->second.second << std::endl;
                        auto ptCorrespIt = pointCorrespondenceMap.find( possibleTemplateId  );
                        if(ptCorrespIt == pointCorrespondenceMap.end()) {
                            pointCorrespondenceMap.insert( std::pair<int, TPointCorrespondence >(possibleTemplateId,  TPointCorrespondence(*t, templateCentroid)));
                        }
                        auto ptCorrespIt2 = pointCorrespondenceMap.find(possibleTemplateId);
                        TPointCorrespondence & ptCorresp = ptCorrespIt2->second;

                        const std::pair<const LSHashEntry*, int> &matchResult = tResultIt->second;

                        const T & tleft = TTraits::left(*(matchResult.first));
                        const T & tright = TTraits::right(*(matchResult.first));

                        TPointCorrespondenceRecord leftRecord (tleft, matchResult.second);
                        TPointCorrespondenceRecord rightRecord(tright, matchResult.second);

                        typename TPointCorrespondence::value_type ptCorresp_left(lPointIdx, leftRecord);
                        typename TPointCorrespondence::value_type ptCorresp_right(rPointIdx, rightRecord);



                        auto pit = ptCorresp.data.find( lPointIdx );
                        if(pit == ptCorresp.data.end() ) {
                            ptCorresp.data.insert( ptCorresp_left );
                            //std::cout << possibleTemplateId << "adding " << ptCorresp_left.second.templatePoint << std::endl;
                        } else {
                            const TPointCorrespondenceRecord &existingRecord = pit->second;
                            if( existingRecord.count < ptCorresp_left.second.count ) {
                                ptCorresp.data.erase(pit);
                                //std::cout << possibleTemplateId << "adding " << ptCorresp_left.second.templatePoint << std::endl;
                                ptCorresp.data.insert( ptCorresp_left );
                            }
                        }

                        pit = ptCorresp.data.find( rPointIdx );
                        if(pit == ptCorresp.data.end() ) {
                            ptCorresp.data.insert( ptCorresp_right );
                            //std::cout << possibleTemplateId << "adding " << ptCorresp_right.second.templatePoint << std::endl;
                        } else {
                            const TPointCorrespondenceRecord &existingRecord = pit->second;
                            if( existingRecord.count < ptCorresp_right.second.count ) {
                                ptCorresp.data.erase(pit);
                                //std::cout << possibleTemplateId << "adding " << ptCorresp_right.second.templatePoint << std::endl;
                                ptCorresp.data.insert( ptCorresp_right );
                            }
                        }

                    }
                    */
        /*
        //std::cout << "///////////////////////////////////////////////////" << std::endl;
        //std::cout << "maximum:  x: " << maxValX <<  ", " << maxValY << ", minimum: " << minValX <<   ",  " << minValY << std::endl;
        auto mit = pointCorrespondenceMap.begin();
        for(; mit != pointCorrespondenceMap.end(); ++mit) {
            int templateId = mit->first;
            TPointCorrespondence &corresp = mit->second;
            for(auto p = corresp.data.begin(); p != corresp.data.end(); ++p) {
                std::cout << templateId << ", " <<  p->first << ": " << p->second.templatePoint << std::endl;
            }
            Eigen::MatrixXf mat;
            std::cout << "!!!!!!!!!: " << templateId << std::endl;
            float fnorm = corresp.foo(mat);
            int i = 90;
            std::cout << templateId << ": " << fnorm << std::endl;
            std::cout << "@@@@@@@@@@@@@@@@@@@@" << std::endl;
        }
        */



                }
            }
        }

        /*

        //std::cout << "///////////////////////////////////////////////////" << std::endl;
        //std::cout << "maximum:  x: " << maxValX <<  ", " << maxValY << ", minimum: " << minValX <<   ",  " << minValY << std::endl;
        auto mit = pointCorrespondenceMap.begin();
        for(; mit != pointCorrespondenceMap.end(); ++mit) {
            int templateId = mit->first;
            TPointCorrespondence &corresp = mit->second;
            for(auto p = corresp.data.begin(); p != corresp.data.end(); ++p) {
                std::cout << templateId << ", " <<  p->first << ": " << p->second.templatePoint << std::endl;
            }
            Eigen::MatrixXf mat;
            std::cout << "!!!!!!!!!: " << templateId << std::endl;
            float fnorm = corresp.foo(mat);
            int i = 90;
            std::cout << templateId << ": " << fnorm << std::endl;
            std::cout << "@@@@@@@@@@@@@@@@@@@@" << std::endl;
        }
        */
    }
}

#endif //KG_GEOMETRIC_HASH_H_