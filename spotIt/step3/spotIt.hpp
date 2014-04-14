/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/
#if !defined(SPOTIT_H_)
#define SPOTIT_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <unordered_set>
#include <set>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/nonfree.hpp>


#include "kgUtils.hpp"
#include "kgGeometricHashing.hpp"

template<>
struct KgGeometricHashing_Traits< cv::Point2f >{
    typedef cv::Point2f T;
    typedef AppropriateNonIntegralType<T::value_type>::value_type K;

    static K distSqrd(const T &l, const T &r) {
        T diff = r - l;
        return static_cast< K > ( diff.x * diff.x + diff.y * diff.y);
    }

    static T orthogonal(const T &p) {
        return T(-p.y, p.x);
    }

    static K dot( const T & l, const T &r) {
        return l.x * r.x + l.y * r.y;
    }

    static T &negate(T &val) {
        val.x = - val.x;
        val.y = -val.y;
        return val;
    }

    //returns  -1 for left, 0 for on , +1 for right
    static int leftRightOrOn(const T &a, const T &b, const T &c) {
        static const K zero = static_cast<K>(0.0);
        T cMinusA = c  - a;
        T bMinusA = b -  a;
        K determinant = bMinusA.x * cMinusA.y - bMinusA.y * cMinusA.x;
        EpsilonEq<K> eps;
        return (eps(determinant, zero)) ?  0 : (( determinant < zero) ? -1 : 1 );
    }


    friend T & operator * ( T &l, K s) {
        l *= s;
        return l;
    }

};


struct HashEntry {
    cv::Point2f l;
    cv::Point2f r;
    int count;
    HashEntry(const cv::Point2f &l, const cv::Point2f &r):
        l(l), r(r), count(0){}
    HashEntry():count(0){}
    bool operator==(const HashEntry &h) {
        return h.l == l && h.r == r;
    }
};

struct HashTableValue {
    std::deque< HashEntry > data;
};


template<typename HashEntry, int n>
struct Hash< cv::Point2f,HashEntry, n  > {
    typedef cv::Point2f T;
    typedef KgGeometricHashing_Traits< cv::Point2f > TTraits;
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    static constexpr K zero = static_cast<K>(0);
    static constexpr K one = static_cast<K>(1);
    Hash(K w):w(w),numEntries(0),oneByW(one/w),gen(42),ndist(std::normal_distribution<>(zero, one)),udist(zero, w),hashTable(160*160*160) {
        for(int i=0; i < n; ++i) {
            a[i] = cv::Point2f(ndist(gen), ndist(gen));
            b[i] = udist(gen);
        }
    }

    int findIndex( int ret[n]) {
        return ret[2] * 160 * 160 + ret[1] * 160 + ret[0];
    }

    void index( const T &arg, const HashEntry & entry) {
        K temp;
        for(int i=0; i < n; ++i) {
            temp = TTraits::dot(a[i], arg) + b[i];
            ret[i] = floor(temp * oneByW);
            assert(ret[i] >= -80 && ret[i] <= 80);
            ret[i] += 80;
            assert(ret[i] > 0);
        }
        int hashTableIndex = findIndex(ret);
        assert(hashTableIndex < hashTable.size());
        HashTableValue &hashTableVal = hashTable[hashTableIndex];
        auto hit = hashTableVal.data.begin();
        int numEntriesOfSameValue =0;
        bool found = false;
        for(hit = hashTableVal.data.begin(); hit != hashTableVal.data.end(); ++hit) {
            if( *hit == entry ) {
                hit->count++;
                numEntriesOfSameValue++;
                found = true;
            }
        }
        if(!found) {
            hashTableVal.data.push_back(entry);
            ++numEntries;
            if(((numEntries % 100) == 0) && (numEntries >= 100)) {
                //stats();
            }
        }
        assert( numEntriesOfSameValue <= 1);
    }


    void stats() const {
        std::map<int, int> histogram;
        for(auto hashTableValIt = hashTable.begin(); hashTableValIt != hashTable.end(); ++hashTableValIt ) {
            int n2 = 0;
            for(auto hoit = hashTableValIt->data.begin(); hoit!= hashTableValIt->data.end(); ++hoit ) {
                n2 += hoit->count;
            }
            n2 /= 10;
            auto histIt = histogram.find(n2);
            if (histIt == histogram.end()) {
                histogram.insert( std::pair<int, int>(n2, 1));
            } else {
                histIt->second += 1;
            }
        }
        std::cout << "histogram of hashTbale" << std::endl;
        for(auto mit = histogram.begin(); mit != histogram.end(); ++mit) {
            std::cout << mit->first << ", " << mit->second << std::endl;
        }
    }

    std::normal_distribution<> ndist;
    std::uniform_real_distribution<K> udist;
    std::mt19937 gen;
    T a[n];
    K b[n];
    K w;
    K oneByW;
    int ret[n];
    std::deque<HashTableValue> hashTable;
    int numEntries;
};


//order the element
template<>
struct std::less<cv::Point2f> : std::binary_function< cv::Point2f, cv::Point2f, bool> {
    typedef cv::Point2f::value_type I;
    EpsilonEq<I> epsilonEq;
    bool operator()(const cv::Point2f &l, const cv::Point2f &r) {
        bool b = ( !epsilonEq(l.x, r.x ) )?
                    (l.x < r.x) :
                    ( !epsilonEq(l.y, r.y ) ?
                        (l.y < r.y):
                        false
                    );
        return b;
    }
};


struct ClusterItem;
struct ContourRepresentativePoint;

class SpotIt {
public:
    SpotIt():
    statsMakerMaxX("maxX"),
    statsMakerMaxY("maxY"),
    statsMakerMinX("minX"),
    statsMakerMinY("minY"),
    hash(1.0){}
    
    ~SpotIt(){}
    //returns true if successfully processed the circle
    bool processCircle(
                       cv::Point &circleCenter,
                       float circleRadius,
                       cv::Mat &inputImage,
                       cv::Mat &roiOutputImageCopy);
    
protected:
    
    void cluster(
                 const cv::Mat &roiHueComponentImage, //in
                 const std::vector< std::vector< cv::Point > > &contours,//in
                 const  std::vector<bool> &shouldProcessContour, //in
                 std::vector<int> &clusterAssignments, //out
                 std::vector<ContourRepresentativePoint> &clusterCenters, //out
                 int &numClusters, //out
                 float &sse //out, sum of squared error
    );


    void drawOutput(
                        const std::vector< std::vector<cv::Point>> &contours,
                        const std::vector<bool> &shouldProcessContour,
                        const std::vector<int> &clusterAssignments,
                        const std::vector<ContourRepresentativePoint> &clusterCenters,
                        const std::vector<cv::KeyPoint> &keyPoints,
                        cv::Mat &inputImage,
                        cv::Mat &roiOutputImage,
                        float sse);

    bool customInitialClusterAssignment( int k,
                                        const std::vector<ClusterItem> &items,

                                            const std::vector< std::vector< cv::Point > > &contours,
                                            const std::map<int, int> &indexRemapping,
                                        std::vector<ClusterItem> &clusterCenters,
                                        std::vector<int> &itemsScelected);

    void extractFeaturesFromRoiOutput (cv::Mat &roiImage, std::vector<cv::KeyPoint> &outKeypoints);

    void approximateCurves(
        const std::vector<std::vector<cv::Point> > & inContours,
        const std::vector<bool> &shouldProcessContour,
        std::vector<std::vector<cv::Point2f> > & outContours ) ;

    void geometricHashBuilding(
        const std::vector< std::vector< cv::Point2f> > & contours
    );

    Kg::StatsMaker statsMakerMaxX;
    Kg::StatsMaker statsMakerMaxY;
    Kg::StatsMaker statsMakerMinX;
    Kg::StatsMaker statsMakerMinY;

    std::vector< std::vector<cv::Point2f> > approximatedContours;
    Hash< cv::Point2f, HashEntry, 3> hash;
};


#endif //SPOTIT_H_