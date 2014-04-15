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
#include <functional>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/nonfree.hpp>


#include "kgUtils.hpp"
#include "kgLocalitySensitiveHash.hpp"
#include "kgGeometricHashing.hpp"

template<>
struct KgCommon_Traits< cv::Point2f >{
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

    static T gen(std::normal_distribution<> &norm, std::mt19937 &gen) {
        T val( norm(gen), norm(gen));
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
    bool operator==(const HashEntry &h)const {
        return h.l == l && h.r == r;
    }
    void write(cv::FileStorage &fs) const {
        fs << "{";
        fs << "left" << l;
        fs << "right" << r;
        fs << "count" << count;
        fs << "}";

    }
    void read(const cv::FileNode &node) {
        node["left"] >> l;
        node["right"] >> r;
        /*
        l = (cv::Point2f)node["left"];
        r = (cv::Point2f)node["right"];
        */
        count = (int)node["count"];
    }
};

//These write and read functions must be defined for the serialization in FileStorage to work
static void write(cv::FileStorage& fs, const std::string&, const HashEntry& x)
{
    x.write(fs);
}
static void read(const cv::FileNode& node, HashEntry& x, const HashEntry& default_value = HashEntry()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

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
    SpotIt( std::vector< std::function<void(char)>> *pKeyboardCallbackRegistry,
           std::vector<std::function<void(int, int, int, int, void*)>> *pMouseCallbackRegistry);
    
    ~SpotIt();
    //returns true if successfully processed the circle
    bool processCircle(
                       cv::Point &circleCenter,
                       float circleRadius,
                       cv::Mat &inputImage,
                       cv::Mat &roiOutputImageCopy);
    
    void readPointClusters(const std::string &filename, std::vector< std::vector< cv::Point2f > > &ptClusters);
    void writePointClusters(const std::string &filename, const std::vector< std::vector< cv::Point2f > > &ptClusters);
    
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
    

    void geometricHashBuilding(std::vector<std::vector<cv::Point2f> >::const_iterator b, std::vector<std::vector<cv::Point2f> >::const_iterator e);
    
    bool handleKey(char keyChar) ;
    bool handleMouse( int event, int x, int y, int flags, void * userdata);
    
    Kg::StatsMaker statsMakerMaxX;
    Kg::StatsMaker statsMakerMaxY;
    Kg::StatsMaker statsMakerMinX;
    Kg::StatsMaker statsMakerMinY;
    std::vector< std::function<void(char)>> *pKeyboardCallbackRegistry;
    std::vector<std::function<void(int, int, int, int, void*)>> *pMouseCallbackRegistry;
    std::vector< std::vector<cv::Point2f> > approximatedContours;
    Hash< cv::Point2f, HashEntry, 3, KgGeometricHash_Traits< cv::Point2f > > hash;
    std::vector<std::vector<cv::Point2f>> pointClusters;
    std::vector<cv::Point2f> pointClusterCentroids;
    struct  ColorScheme {
        std::string name;
        cv::Scalar  value;
        ColorScheme(const std::string &name, const cv::Scalar &col):
        name(name),value(col){}
        ColorScheme(){}
    };
    std::vector<ColorScheme> colorSchemesForDrawing;
    friend void keyboardCallback(SpotIt *p, char keyChar);
    friend void mouseCallback(SpotIt *p, int event, int x, int y, int flags, void * userdata);
};


#endif //SPOTIT_H_