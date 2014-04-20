#include <gtest/gtest.h>
#include "../spotIt.hpp"


using namespace std;
using namespace cv;



TEST( LocalitySensitiveHashing, PStable) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    int hashOutput1[3];
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> udist(1.0f, 2.0f);
    /*
     Point2f p1(-40.0f, 20.0f);
     hashEngine(p1, hashOutput1);
     cout << hashOutput1[0] << ", " << hashOutput1[1] << ", " << hashOutput1[2] << endl;
     
     for(int i=0; i < 1000; ++i) {
     int hashOutput[3];
     int hashDiff[3];
     Point2f delta  = Point2f(-2.0f + 2.0f * udist(gen), -2.0f + 2.0f * udist(gen));
     Point2f p = delta + p1;
     hashEngine(p, hashOutput);
     hashDiff[0] = hashOutput[0] - hashOutput1[0];
     hashDiff[1] = hashOutput[1] - hashOutput1[1];
     hashDiff[2] = hashOutput[2] - hashOutput1[2];
     if( abs(hashDiff[0]) > 1 && abs(hashDiff[1]) > 1 && abs(hashDiff[2]) > 1  ) {
     cout << p.x << ", " << p.y << endl;
     cout << hashOutput[0] << ", " << hashOutput[1] << ", " << hashOutput[2] << endl;
     }
     }
     */
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
    LocalitySensitiveHash<Point2f, TestHashEntry, 3, KgLocalitySensitiveHash_Traits< cv::Point2f >> hashEngine(1.0f, -2, 2);

    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    TestHashEntry he1(67);
    TestHashEntry he2(54);

    hashEngine.index(pt1, he1);
    hashEngine.index(pt1, he1);
    hashEngine.index(pt2, he2);

    const std::string fname("data/testhash1.xml");
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



TEST( LocalitySensitiveHashingFirGeometricHash, Serialization2) {
    LocalitySensitiveHash<Point2f, LSHashEntryForGeometricHash, 3, KgLocalitySensitiveHash_Traits< cv::Point2f > > hashEngine(1.0f, -2, 2);
    
    Point2f pt1(0.5f, 0.5f), pt2(0.0f, -0.7f);
    LSHashEntryForGeometricHash he1(Point2f(0,0), Point2f(0,1));
    LSHashEntryForGeometricHash he2(Point2f(1,0), Point2f(1,1));
    
    hashEngine.index(pt1, he1);
    hashEngine.index(pt1, he1);
    hashEngine.index(pt2, he2);


    const std::string fname("data/testhash2.xml");
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
    const std::string fname("data/merged.xml");
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


TEST( SpotIt, Write) {
    vector< vector< Point2f > > ptClusters(2);
    ptClusters[0].reserve(2);
    ptClusters[0].push_back(Point2f(100, 100));
    ptClusters[0].push_back(Point2f(200, 200));
    
    
    ptClusters[1].reserve(1);
    ptClusters[1].push_back(Point2f(100, 100));
    SpotIt sp(NULL, NULL);

    sp.writePointClusters("foo.xml", "blue", ptClusters[0], Point2f());
    
    vector< vector< Point2f > > ptClusters2;
    ptClusters2.resize(2);
    std::string readName;
    sp.readPointClusters("foo.xml", readName, ptClusters2[0]);

    EXPECT_EQ(ptClusters[0][0], ptClusters2[0][0]);
    
}

int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    
}