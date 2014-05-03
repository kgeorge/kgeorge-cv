#include <gtest/gtest.h>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include "kgeorge_cv/kgLocalitySensitiveHash.hpp"
#include "kgeorge_cv/kgUtils.hpp"
#include "kgeorge_cv/kgKernel.hpp"
#include "kgeorge_cv/kgGeometricHash.hpp"
#include "kgeorge_cv/kgLeastSquaresTransform.hpp"


using namespace std;
using namespace cv;



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

/*

TEST( LocalitySensitiveHashing, PStable) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    int hashOutput1[3];
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> udist(1.0f, 2.0f);
    EXPECT_EQ(1, 1);
}
*/
struct TestHashEntry {
    int i;
    int count;
    TestHashEntry():i(0), count(0){}

    TestHashEntry(int i):i(i),count(0){}

    void write(cv::FileStorage &fs) const {
        fs << "{";
        fs << "i" << i;
        fs << "count" << count;
        fs << "}";
    }
    
    
    void read(const cv::FileNode &node)  {
        i = (int)node["i"];
        count = (int)node["count"];
    }


    bool operator==(const TestHashEntry &h)const {
        return h.i == i;;
    }
};


std::ostream & operator << ( std::ostream &o, const TestHashEntry & rhs) {
    o << rhs.i ;
    return o;
}

static void write(FileStorage& fs, const std::string&, const TestHashEntry& x)
{
    x.write(fs);
}

static void read(const FileNode& node, TestHashEntry& x, const TestHashEntry& default_value = TestHashEntry()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

TEST( LocalitySensitiveHashing, Serialization1) {
    LocalitySensitiveHash<Point2f, TestHashEntry, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -2, 2);

    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    TestHashEntry he1(67);
    TestHashEntry he2(54);

    hashEngine.index(pt1, he1);
    hashEngine.index(pt1, he1);
    hashEngine.index(pt2, he2);

    const std::string fname("data/dump/testhash1.xml");
    {
        cv::FileStorage fs(fname, cv::FileStorage::WRITE);
        fs << "HashTable";
        hashEngine.serialize(fs);
        fs.release();
    }
    LocalitySensitiveHash<Point2f, TestHashEntry, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;
    {
        cv::FileStorage fs(fname, cv::FileStorage::READ);
        FileNode fn = fs["HashTable"];
        hashEngine2.unSerialize(fn);
        fs.release();
    }
    EXPECT_EQ(hashEngine.w, hashEngine2.w);
    EXPECT_EQ(hashEngine.oneByW, hashEngine2.oneByW);
    EXPECT_EQ(hashEngine.minRange, hashEngine2.minRange);
    EXPECT_EQ(hashEngine.maxRange, hashEngine2.maxRange);
    EXPECT_EQ(hashEngine.nSizePerBucket, hashEngine2.nSizePerBucket);
    EXPECT_EQ(hashEngine.nSizeAllBuckets, hashEngine2.nSizeAllBuckets);
    EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}

/*

TEST( LocalitySensitiveHashingFirGeometricHash, Serialization2) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    
    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    LSHashEntryForGeometricHash he1(Point2f(0,0), Point2f(0,1));
    LSHashEntryForGeometricHash he2(Point2f(1,0), Point2f(1,1));
    
    hashEngine.index(pt1, he1);
    hashEngine.index(pt1, he1);
    hashEngine.index(pt2, he2);


    const std::string fname("data/test/dump/testhash2.xml");
    {
        cv::FileStorage fs(fname, cv::FileStorage::WRITE);
        fs << "HashTable";
        hashEngine.serialize(fs);
        fs.release();
    }
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;

    {
        cv::FileStorage fs(fname, cv::FileStorage::READ);
        cv::FileNode fn = fs["HashTable"];
        hashEngine2.unSerialize(fn);
        fs.release();
    }
    EXPECT_EQ(hashEngine.w, hashEngine2.w);
    EXPECT_EQ(hashEngine.oneByW, hashEngine2.oneByW);
    EXPECT_EQ(hashEngine.minRange, hashEngine2.minRange);
    EXPECT_EQ(hashEngine.maxRange, hashEngine2.maxRange);
    EXPECT_EQ(hashEngine.nSizePerBucket, hashEngine2.nSizePerBucket);
    EXPECT_EQ(hashEngine.nSizeAllBuckets, hashEngine2.nSizeAllBuckets);
    EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}




TEST( LocalitySensitiveHashingForGeometricHash, Serialization3) {
    const std::string fname("data/test/merged.xml");
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;

    {
        cv::FileStorage fs(fname, cv::FileStorage::READ);
        cv::FileNode fn = fs["SpotItHash"];
        cv::FileNode fn2 = fn["hashTable"];
        hashEngine2.unSerialize(fn2);
        fs.release();
    }
    EXPECT_EQ(1.0, hashEngine2.w);
    EXPECT_EQ(1.0, hashEngine2.oneByW);
    EXPECT_EQ(-80, hashEngine2.minRange);
    EXPECT_EQ(80, hashEngine2.maxRange);
    EXPECT_EQ(160, hashEngine2.nSizePerBucket);
    EXPECT_EQ(4096000, hashEngine2.nSizeAllBuckets);
    //EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}



TEST( LocalitySensitiveHashingForGeometricHash, ConstancyOfHash) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

    Point2f pt1(0.5f, 0.5f), pt2(-1.0f, 0.0f), pt3(1.0f, 0.0f);
    LSHashEntryForGeometricHash he1(pt1, pt2);
    LSHashEntryForGeometricHash he2(pt2, pt3);
    LSHashEntryForGeometricHash he3(pt3, pt1);

    int index = hashEngine.index(pt3, he1);
    for(int i=0; i < 5; ++i) {
        int index2 =  hashEngine.index(pt3, he1);
        EXPECT_EQ(index, index2);
    }

    for(int i=0; i < 5; ++i) {
        int index2 =  hashEngine.index(pt3, he2);
        EXPECT_EQ(index, index2);
    }
}



TEST( LocalitySensitiveHashing, AdvanceDiff) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

        int output[3];
        for(int j=0; j < 3; ++j) {
            output[j] = -1;
        }
    for(int i=0; i < 100; ++i) {
        hashEngine.advanceDiffIndex(output);
        std::cout << output[0] << ", " << output[1] << ", " << output[2] << std::endl;
    }
}

TEST( LocalitySensitiveHashingForGeometricHash, LSHashEntryForGeometricHashOrder) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

    Point2f pt1(0.5f, 0.5f), pt2(-1.0f, 0.0f), pt3(1.0f, 0.0f);
    LSHashEntryForGeometricHash he1(pt1, pt2, 1);
    LSHashEntryForGeometricHash he2(pt2, pt3, 1);

    LSHashEntryForGeometricHash he3(pt3, pt1, 2);
    LSHashEntryForGeometricHash he4(pt3, pt1, 2);

    less<LSHashEntryForGeometricHash> lessHE;
    EXPECT_EQ(lessHE(he1, he2), false);
    EXPECT_EQ(lessHE(he1, he3), true);
    EXPECT_EQ(he3 , he4);
}



TEST(LeastSquaresTransform, Transform) {
    Eigen::MatrixXf A(7, 3), B(7,3);

    A:

   4:
    |\      A
    |'\
    | '\,
    |   \,
....|____\.............
    :    2
    :
    :
                   _,-, 2
            _,--'-'   |    B
    :  ,-/~'          |
....:/'...............|.......
    :                 4




    A << 0, 4, 1,
        0, 3, 1,
        0, 2, 1,
        0, 1, 1,
        0, 0, 1,
        1, 0, 1,
        2, 0, 1;

    B << 0, 0, 1,
        1, 0, 1,
        2, 0, 1,
        3, 0, 1,
        4, 0, 1,
        4, 1, 1,
        4, 2, 1;

    Eigen::MatrixXf T;
    LeastSquaresTransform<cv::Point2f> lst;

    lst(A, B, T);
    EXPECT_EQ(T.cols(), 3);
    EXPECT_EQ(T.rows(), 3);
    Eigen::MatrixXf TExpected(3,3);
    TExpected << 0, 1, 0,
                -1, 0, 0,
                4, 0, 1;
    EXPECT_EQ(TExpected.isApprox(T, 0.0001f), true);
}


TEST(LeastSquaresTransform, PointCorrespondence) {

    std::vector<cv::Point2f> rvec(7);
    rvec[0] = Point2f(0, 4);
    rvec[1] = Point2f(0, 3);
    rvec[2] = Point2f(0, 2);
    rvec[3] = Point2f(0, 1);
    rvec[4] = Point2f(0, 0);
    rvec[5] = Point2f(1, 0);
    rvec[6] = Point2f(2, 0);

    PointCorrespondence< std::vector< cv::Point2f >> ptCorresp(rvec, Point2f());
    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(0, 0), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 0, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(1, 0), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 1, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(2, 0), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 2, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(3, 0), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 3, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(4, 0), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 4, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(4, 1), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 5, rec));
    }

    {
        PointCorrespondence< std::vector< cv::Point2f > >::Record rec(Point2f(4, 2), 1);
        ptCorresp.data.insert(pair<int, PointCorrespondence< std::vector< cv::Point2f > >::Record >( 6, rec));
    }
    Eigen::MatrixXf T;
    float f = ptCorresp.foo(T);
    f = f;
}

*/
int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    
}
