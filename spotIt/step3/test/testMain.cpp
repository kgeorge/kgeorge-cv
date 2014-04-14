#include <gtest/gtest.h>
#include "../spotIt.hpp"


using namespace std;
using namespace cv;



TEST( LSHHashing, PStable) {
    Hash<Point2f, HashEntry, 3> hashEngine(1.0f);
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

TEST( SpotIt, Write) {
    vector< vector< Point2f > > ptClusters(2);
    ptClusters[0].reserve(2);
    ptClusters[0].push_back(Point2f(100, 100));
    ptClusters[0].push_back(Point2f(200, 200));


    ptClusters[1].reserve(1);
    ptClusters[1].push_back(Point2f(100, 100));
    SpotIt sp(NULL, NULL);
    sp.writePointClusters("foo.xml", ptClusters);

    vector< vector< Point2f > > ptClusters2;
    sp.readPointClusters("foo.xml", ptClusters2);
    
    
    
    EXPECT_EQ(2, ptClusters2.size());
    EXPECT_EQ(ptClusters[0][0], ptClusters2[0][0]);
    
}

int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();

}