#include <gtest/gtest.h>
#include "../spotIt.hpp"


using namespace std;
using namespace cv;



TEST( LocalitySensitiveHashing, PStable) {
    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    int hashOutput1[3];
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> udist(1.0f, 2.0f);
    EXPECT_EQ(1, 1);
}

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

    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    LocalitySensitiveHash<Point2f, TestHashEntry, kLSHashNumFields, kLSHashNumBuckets,  KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -2, 2);

    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    TestHashEntry he1(67);
    TestHashEntry he2(54);

    int dummyHashTableIndices[kLSHashNumBuckets];

    hashEngine.index(pt1, he1, dummyHashTableIndices);
    hashEngine.index(pt1, he1, dummyHashTableIndices);
    hashEngine.index(pt2, he2, dummyHashTableIndices);

    const std::string fname("data/test/dump/testhash1.xml");
    {
        cv::FileStorage fs(fname, cv::FileStorage::WRITE);
        fs << "HashTable";
        hashEngine.serialize(fs);
        fs.release();
    }
    LocalitySensitiveHash<Point2f, TestHashEntry, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;
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
    EXPECT_EQ(hashEngine.nSizePerField, hashEngine2.nSizePerField);
    EXPECT_EQ(hashEngine.nSizeAllFields, hashEngine2.nSizeAllFields);
    EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}



TEST( LocalitySensitiveHashingFirGeometricHash, Serialization2) {
    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;

    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    
    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    LSHashEntryForGeometricHash he1(Point2f(0,0), Point2f(0,1));
    LSHashEntryForGeometricHash he2(Point2f(1,0), Point2f(1,1));


    int dummyHashTableIndices[kLSHashNumBuckets];


    hashEngine.index(pt1, he1, dummyHashTableIndices);
    hashEngine.index(pt1, he1, dummyHashTableIndices);
    hashEngine.index(pt2, he2, dummyHashTableIndices);


    const std::string fname("data/test/dump/testhash2.xml");
    {
        cv::FileStorage fs(fname, cv::FileStorage::WRITE);
        fs << "HashTable";
        hashEngine.serialize(fs);
        fs.release();
    }
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;

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
    EXPECT_EQ(hashEngine.nSizePerField, hashEngine2.nSizePerField);
    EXPECT_EQ(hashEngine.nSizeAllFields, hashEngine2.nSizeAllFields);
    EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}




TEST( LocalitySensitiveHashingForGeometricHash, Serialization3) {
    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    const std::string fname("data/test/merged.xml");
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine2;

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
    EXPECT_EQ(160, hashEngine2.nSizePerField);
    EXPECT_EQ(4096000, hashEngine2.nSizeAllFields);
    //EXPECT_EQ(hashEngine.numEntries, hashEngine2.numEntries);
}



TEST( LocalitySensitiveHashingForGeometricHash, ConstancyOfHash) {

    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

    Point2f pt1(0.5f, 0.5f), pt2(-1.0f, 0.0f), pt3(1.0f, 0.0f);
    LSHashEntryForGeometricHash he1(pt1, pt2);
    LSHashEntryForGeometricHash he2(pt2, pt3);
    LSHashEntryForGeometricHash he3(pt3, pt1);

    int hashTableIndicesCalculated[kLSHashNumBuckets];
    hashEngine.index(pt3, he1, hashTableIndicesCalculated);

    for(int k=0; k < 5; ++k) {
        int hashTableIndicesCalculated2[kLSHashNumBuckets];
        hashEngine.index(pt3, he1, hashTableIndicesCalculated2);
        for(int i=0; i < kLSHashNumBuckets; ++ i) {
            EXPECT_EQ(hashTableIndicesCalculated[i], hashTableIndicesCalculated2[i]);
        }
    }

}



TEST( LocalitySensitiveHashing, AdvanceDiff) {

    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

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

    const int kLSHashNumFields=3;
    const int kLSHashNumBuckets=1;
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, kLSHashNumFields, kLSHashNumBuckets,  KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -4, 4);

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
    /*
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



    */
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



TEST( SpotIt, Write) {
    vector< vector< Point2f > > ptClusters(2);
    ptClusters[0].reserve(2);
    ptClusters[0].push_back(Point2f(100, 100));
    ptClusters[0].push_back(Point2f(200, 200));
    
    
    ptClusters[1].reserve(1);
    ptClusters[1].push_back(Point2f(100, 100));
    SpotIt sp(NULL, NULL);

    sp.writePointClusters("data/test/dump/foo.xml", "blue", ptClusters[0], Point2f());
    
    vector< vector< Point2f > > ptClusters2;
    ptClusters2.resize(2);
    std::string readName;
    sp.readPointClusters("data/test/dump/foo.xml", readName, ptClusters2[0]);

    EXPECT_EQ(ptClusters[0][0], ptClusters2[0][0]);
    
}

int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    
}