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


#include "kgeorge_cv/kgUtils.hpp"
#include "kgeorge_cv/kgLocalitySensitiveHash.hpp"
#include "kgeorge_cv/kgGeometricHash.hpp"

template<>
struct KgCommon_Traits< cv::Point2f >{
    typedef cv::Point2f T;
    typedef AppropriateNonIntegralType<T::value_type>::value_type K;

    static const int numElementsInT = 2;
    static constexpr int numElementsInTPlus1 = 3;
    typedef Eigen::Matrix<K, Eigen::Dynamic,  numElementsInTPlus1> TMatrix;
    
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


    static void fill( const T &arg, int idx,  TMatrix  &target) {
        target(idx, 0)  = arg.x;
        target(idx, 1)  = arg.y;
        target(idx, 2)  = 1.0f;
    }

    friend T & operator * ( T &l, K s) {
        l *= s;
        return l;
    }
    
};

template<>
struct KgGeometricHash_Traits<cv::Point2f> : public KgCommon_Traits<cv::Point2f>{

    template< typename LSHEntry>
    static void foo( const LSHEntry &a) {
        std::cout << "KgGeometricHash_Traits::foo" << std::endl;
    }

    template<typename LSHEntry>
    static const T &left( const LSHEntry &entry) {
        //throw std::runtime_error( "not implemented" );
        return entry.l;
    }

    template<typename LSHEntry>
    static const T &right( const LSHEntry &entry) {
        //throw std::runtime_error( "not implemented" );
        return entry.r;
    }

    template<typename LSHEntry>
    static T &left( LSHEntry &entry) {
        //throw std::runtime_error( "not implemented" );
        return entry.l;
    }


    template<typename LSHEntry>
    static T &right( LSHEntry &entry) {
        //throw std::runtime_error( "not implemented" );
        return entry.r;
    }

};


//order the element
template<>
struct std::less<cv::Point2f> : std::binary_function< cv::Point2f, cv::Point2f, bool> {
    typedef cv::Point2f::value_type I;
    EpsilonEq<I> epsilonEq;
    bool operator()(const cv::Point2f &l, const cv::Point2f &r) {
        bool b =
        ( l.x != r.x )?
        (l.x < r.x) :
        ( l.y != r.y ) ?
         (l.y < r.y):
         false;
        return b;
    }
};



struct LSHashEntryForGeometricHash {
    cv::Point2f l;
    cv::Point2f r;
    int templateId;
    int count;
    LSHashEntryForGeometricHash(const cv::Point2f &l, const cv::Point2f &r, int templateId=-1):
    l(l), r(r), count(0),templateId(templateId){}
    LSHashEntryForGeometricHash():count(0){}
    bool operator==(const LSHashEntryForGeometricHash &h)const {
        return h.l == l && h.r == r && h.templateId == templateId;
    }

    void write(cv::FileStorage &fs) const {
        fs << "{";
        fs << "left" << l;
        fs << "right" << r;
        fs << "count" << count;
        fs << "templateId" << templateId;
        fs << "}";

    }
    void read(const cv::FileNode &node) {
        node["left"] >> l;
        node["right"] >> r;
        templateId = (int)node["templateId"];
        count = (int)node["count"];
    }
};

template<>
struct StatsForLSHashEntry<LSHashEntryForGeometricHash> {
    typedef LSHashEntryForGeometricHash LSHashEntry;

    void operator()(const LSHashEntry &rhs) {
        auto mit = totalNumEntriesPerTemplate.find(rhs.templateId);
        if(mit == totalNumEntriesPerTemplate.end()) {
            totalNumEntriesPerTemplate.insert( std::pair<int, int>(rhs.templateId, rhs.count));
        } else {
            mit->second += rhs.count;
        }
    }
    std::map< int, int> totalNumEntriesPerTemplate;

    std::ostream & output( std::ostream &o) {
        o << "!!!!!!!!!!!!!!!!!!!" << std::endl;
        o << "Locality sensitive hash output" << std::endl;
        for(auto mit =totalNumEntriesPerTemplate.begin(); mit != totalNumEntriesPerTemplate.end(); ++mit ) {
            o << mit->first << ", " << mit->second << std::endl;
        }
        return o;
    }
};



inline std::ostream & operator << ( std::ostream &o, const LSHashEntryForGeometricHash & rhs) {
    o << rhs.templateId << ", (" << rhs.l.x << ", " << rhs.l.y << "), (" << rhs.r.x << ", " << rhs.r.y << ")";
    return o;
}
//order the element
template<>
struct std::less<LSHashEntryForGeometricHash> : std::binary_function< LSHashEntryForGeometricHash, LSHashEntryForGeometricHash, bool> {
    bool operator()(const LSHashEntryForGeometricHash &left, const LSHashEntryForGeometricHash &right) const {
        std::less<cv::Point2f> lessPoint2f;
        return (left.templateId != right.templateId) ? (left.templateId < right.templateId):
            (left.l != right.l) ? lessPoint2f(left.l, right.l) :
             (left.r != right.r) ? lessPoint2f(left.r, right.r): false;
    }
};


//order the element
template<>
struct std::less<LSHashEntryForGeometricHash*> : std::binary_function< LSHashEntryForGeometricHash*, LSHashEntryForGeometricHash*, bool> {
    bool operator()(const LSHashEntryForGeometricHash *left, const LSHashEntryForGeometricHash *right) const {
        std::less<cv::Point2f> lessPoint2f;
        return (left->templateId != right->templateId) ? (left->templateId < right->templateId):
            (left->l != right->l) ? lessPoint2f(left->l, right->l) :
             (left->r != right->r) ? lessPoint2f(left->r, right->r): false;
    }
};




//These write and read functions must be defined for the serialization in FileStorage to work
static void write(cv::FileStorage& fs, const std::string&, const LSHashEntryForGeometricHash& x)
{
    x.write(fs);
}
static void read(const cv::FileNode& node, LSHashEntryForGeometricHash& x, const LSHashEntryForGeometricHash& default_value = LSHashEntryForGeometricHash()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}
/*
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
*/

struct ClusterItem;
struct ContourRepresentativePoint;
const int kLSHashNumFields=3;
const int kLSHashNumBuckets=1;

typedef LocalitySensitiveHash<
    cv::Point2f,
    LSHashEntryForGeometricHash,
    kLSHashNumFields,
    kLSHashNumBuckets,
    KgLocalitySensitiveHash_Traits< cv::Point2f >
    > SpotItLocalitySensitiveHashBase;

//use a derived class for proper serialization of the hashtable
struct SpotItLocalitySensitiveHash : public SpotItLocalitySensitiveHashBase {
    typedef float K;
    SpotItLocalitySensitiveHash(K w, K minRange, K maxRange):SpotItLocalitySensitiveHashBase(w,minRange, maxRange){}
    SpotItLocalitySensitiveHash():SpotItLocalitySensitiveHashBase(){}

    void serialize(const std::string &filename) const{
        cv::FileStorage fs(filename, cv::FileStorage::WRITE);
        if (!fs.isOpened())
        {
            std::cerr << "Failed to open " << filename << std::endl;
            throw std::runtime_error("failed to open file");
        }
        fs << "SpotItHash";
        fs << "{";

        fs << "score" << "{";
        fs << "outlineOnly" << 100;
        fs << "noNoise" << 100;
        fs << "bigSize" << 50;
        fs << "color" << colorName;
        fs << "connectedness" << 100;
        fs << "}";
        fs << "name" << colorName;
        fs << "hashTable";

        SpotItLocalitySensitiveHashBase::serialize(fs);
        fs << "}";
        fs.release();
    }
    void unSerialize(const std::string &filename) {
        cv::FileStorage fs(filename, cv::FileStorage::READ);
        if (!fs.isOpened())
        {
            std::cerr << "Failed to open " << filename << std::endl;
            throw std::runtime_error("failed to open file");
        }
        cv::FileNode fn = fs["SpotItHash"];
        cv::FileNode score = fn["score"];
        std::string name = (std::string)fn["name"];
        cv::FileNode fn_hashtable = fn["hashTable"];
        SpotItLocalitySensitiveHashBase::unSerialize(fn_hashtable);
        fs.release();
    }
    std::string colorName;
};

class SpotIt {
public:
typedef enum { eCollect=0, eQuery} EMode;
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
    
    static void readPointClusters(const std::string &filename,  std::string &name, std::vector< cv::Point2f >  &ptCluster);
    static void writePointClusters(
        const std::string &filename,
        const  std::string &name,
        const std::vector< cv::Point2f >  &ptCluster,
        const cv::Point2f &clusterCentroid);
    
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

    void geometricHashQuerying( int pointClusterIndex);
    void geometricHashBuilding( int pointClusterIndex);
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
    SpotItLocalitySensitiveHash lsHash;
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
    void initTemplateNameLookup();
    std::map<std::string, int> templateNameLookup;
    std::map<int, std::vector<cv::Point2f> > pointClusterMap;
    EMode mode;
    int lastSelectedClusterIdx;
};


#endif //SPOTIT_H_