/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/
#include <iostream>
#include <numeric>

#include <ctime>
#include <cmath>
#include <set>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include "spotIt.hpp"
#include "kgUtils.hpp"
#include "kMeansClustering.hpp"
#include "kgGeometricHash.hpp"

using namespace std;
using namespace cv;
using namespace std::placeholders;


//wrapper for OGLAugmentedScene::_handleKey
void keyboardCallback(SpotIt *p, char keyChar) {
    bool bVal = p->handleKey(keyChar);
    if(bVal) {
        p->pKeyboardCallbackRegistry->clear();
    }
}




void mouseCallback(SpotIt *p, int event, int x, int y, int flags, void * userdata) {
    p->handleMouse( event, x, y, flags, userdata);
}

template<>
const float Quantizer<float>::kEpsilon_ = 0.000001f;

template<>
const double Quantizer<double>::kEpsilon_ = 0.00000000001;




SpotIt::SpotIt( std::vector< std::function<void(char)>> *pKeyboardCallbackRegistry, std::vector<std::function<void(int, int, int, int, void*)>> *pMouseCallbackRegistry):
statsMakerMaxX("maxX"),
statsMakerMaxY("maxY"),
statsMakerMinX("minX"),
statsMakerMinY("minY"),
lsHash(1.0, -80, 80),
pKeyboardCallbackRegistry(pKeyboardCallbackRegistry),
pMouseCallbackRegistry(pMouseCallbackRegistry){
    //register keyboard function
    if( pKeyboardCallbackRegistry ) {
        std::function<void(char)> boundKeyboardFunBody = std::bind(keyboardCallback, this, _1 );
        pKeyboardCallbackRegistry->push_back(boundKeyboardFunBody);
    }
    
    if( pMouseCallbackRegistry ) {
        std::function<void(int, int, int, int, void *)> boundMouseFunBody = std::bind(mouseCallback, this, _1, _2, _3, _4, _5 );
        pMouseCallbackRegistry->push_back(boundMouseFunBody);
    }
    
    
    colorSchemesForDrawing.reserve(11);
    colorSchemesForDrawing.push_back( ColorScheme("blue", Scalar(255, 0, 0)) );
    colorSchemesForDrawing.push_back( ColorScheme("red", Scalar(0, 0, 255)) );
    colorSchemesForDrawing.push_back( ColorScheme("green", Scalar(0, 255, 0)) );
    colorSchemesForDrawing.push_back( ColorScheme("yellow", Scalar(0, 255, 255)));
    colorSchemesForDrawing.push_back( ColorScheme("purple", Scalar(255, 0, 255)) );
    colorSchemesForDrawing.push_back( ColorScheme("cyan",  Scalar(255, 255, 0)) );
    colorSchemesForDrawing.push_back( ColorScheme("bisque", Scalar(196, 228, 255)) );
    colorSchemesForDrawing.push_back( ColorScheme("olive", Scalar(0, 128, 128)));
    
    colorSchemesForDrawing.push_back( ColorScheme("skyblue", Scalar(235, 206, 135)) );
    colorSchemesForDrawing.push_back( ColorScheme("orange",  Scalar(0, 165, 255)) );
    colorSchemesForDrawing.push_back( ColorScheme("limegreen", Scalar(47, 255, 173)) );
}


SpotIt::~SpotIt( ) {
    if(  pKeyboardCallbackRegistry ) {
        pKeyboardCallbackRegistry->clear();
    }
    
    if(  pMouseCallbackRegistry ) {
        pMouseCallbackRegistry->clear();
    }
}

bool SpotIt::processCircle(
                           cv::Point &circleCenter,
                           float circleRadius,
                           Mat &inputImage,
                           Mat &roiOutputImage
                           ) {
    KG_DBGOUT( cout << "  inputImage type " << inputImage.type() << "  inputImage depth " << inputImage.depth() << endl );
    if(pKeyboardCallbackRegistry && pKeyboardCallbackRegistry->size() <= 0) {
        
        std::function<void(char)> boundKeyboardFunBody = std::bind(keyboardCallback, this, _1 );
        pKeyboardCallbackRegistry->push_back(boundKeyboardFunBody);
    }
    vector<Vec4i> hierarchy;
    
    //find an roi in input image
    
    //make roi slightly bigger than the circle specified
    int roiDim = circleRadius * 2.2;
    //roiOrigin
    Point roiOrigin(Kg::max(circleCenter.x - roiDim/2, 0) , Kg::max(circleCenter.y - roiDim/2, 0));
    //roi width, height
    int adjustedRoiWidth = Kg::min( (int)roiDim  , static_cast<int>(inputImage.cols - roiOrigin.x));
    int adjustedRoiHeight = Kg::min( (int)roiDim  , static_cast<int>(inputImage.rows - roiOrigin.y));
    Point circleCenterInRoi = circleCenter - roiOrigin;
    //reference to part of inputImage that covers roi
    Mat tempImage = inputImage(Rect(roiOrigin.x, roiOrigin.y, adjustedRoiWidth, adjustedRoiHeight));
    
    
    //input image processing
    
    Mat  roiGrayImage, roiHsvImage;
    blur(tempImage, tempImage, cv::Size(3,3));
    cvtColor(tempImage, roiGrayImage, CV_BGR2GRAY);
    blur(tempImage, tempImage, cv::Size(3,3));
    cvtColor(tempImage, roiHsvImage, CV_BGR2HSV);
    vector<Mat> roiHsvComponents;
    split(roiHsvImage, roiHsvComponents);
    
    //Canny
    double hysterisisRatio =  3.0;
    int lowThreshold = 70;
    int kernelSize = 3;
    Canny(roiGrayImage, roiGrayImage, lowThreshold, hysterisisRatio * lowThreshold, kernelSize );
    
    
    //findContours
    vector<vector<Point> > contours;
    findContours(
                 roiGrayImage,
                 contours,
                 hierarchy,
                 CV_RETR_EXTERNAL,
                 CV_CHAIN_APPROX_TC89_L1
                 //,roiOrigin
                 );
    
    //only process contours that fall within specified circle
    const double circleRadiusSq = circleRadius * circleRadius;
    vector<bool> shouldProcessContour(contours.size(), false);
    int nContoursGood = 0;
    for(int i=0; i < contours.size(); ++i) {
        vector<Point> &contour = contours[i];
        //number of points in the contour that falls within the circle
        int numPointsInCircle = 0;
        
        int sumDistSqFromCircle = 0;
        int sumSqDistSqFromCircle =  0;
        for(int j=0; j < contour.size(); ++j) {
            Point pRelativeToCircleCenter = contour[j] - circleCenterInRoi;
            double distSqOfPointToCenter = pRelativeToCircleCenter.x * pRelativeToCircleCenter.x  + pRelativeToCircleCenter.y  * pRelativeToCircleCenter.y;
            sumDistSqFromCircle += (circleRadiusSq - distSqOfPointToCenter);
            sumSqDistSqFromCircle  += (circleRadiusSq - distSqOfPointToCenter) * (circleRadiusSq - distSqOfPointToCenter);
            if( ( circleRadiusSq - distSqOfPointToCenter )  > 2000 ) {
                ++numPointsInCircle;
            }
        }
        double avgDistSqFromCircle = sumDistSqFromCircle /contour.size();
        double varianceDistSqFromCircle = sumSqDistSqFromCircle /contour.size() - avgDistSqFromCircle * avgDistSqFromCircle;
        double percentageNumPointsInCircle = (double)numPointsInCircle * 100.0/ contour.size();
        //if 90% of pointsof contour falls within the circle,
        //then this is a contour that need be further processed
        if( percentageNumPointsInCircle > 90.0 ) {
            //if( avgDistSqFromCircle > 1000.0 ) {
            assert(contour.size() > 0);
            shouldProcessContour[i] = true;
            
            KG_DBGOUT( cout << "~~~~~~~~~~~~~~~~" << endl);
            KG_DBGOUT( cout << "mu= " << avgDistSqFromCircle << ", rho=" << varianceDistSqFromCircle <<  endl);
            nContoursGood++;
        }
    }
    if(nContoursGood <= 0) {
        //if no contours were produced
        //return false
        KG_DBGOUT( cout << "dropped frame: " << endl );
        return false;
    }
    
    //cluster contours based on KMeans
    int numClusters=0;
    vector<int> clusterAssignments;
    float sse = numeric_limits<float>::max();
    
    
    vector<ContourRepresentativePoint> clusterCenters;
    
    cluster(
            roiHsvComponents[0], //hue component of roi
            contours,           //output from findContours
            shouldProcessContour, //flag whether each contour need be processed
            clusterAssignments, //output cluster assignments,
            clusterCenters, //output cluster centers , used for drawing
            numClusters,  //number of clusters,
            sse //sum of squared error
            );
    
    
    
    vector<vector<Point2f>> approximatedContours(contours.size());
    approximateCurves( contours, shouldProcessContour,  approximatedContours  );
    
    
    
    map<int, int> numPointsForEachCluster;
    for(int i=0; i < numClusters; ++i) {
        numPointsForEachCluster[i] =0;
    }
    for(int i=0; i < approximatedContours.size(); ++i ) {
        if( shouldProcessContour[i]) {
            int clusterId = clusterAssignments[i];
            assert(clusterId >= 0);
            numPointsForEachCluster[clusterId] += approximatedContours[i].size();
        }
    }
    
    //draw the output onto the output images
    roiOutputImage.create(roiGrayImage.size(), inputImage.type() );
    roiOutputImage = Scalar(0,0,0);
    
    vector<KeyPoint> keyPoints;
    //extractFeaturesFromRoiOutput (roiGrayImage, keyPoints );
    
    pointClusters.resize(numClusters);
    for(int i=0; i < numClusters; ++i) {
        if(numPointsForEachCluster[i] > 0) {
            pointClusters[i].reserve(numPointsForEachCluster[i]);
            pointClusters[i].clear();
        }
    }
    vector< vector<Point> > outContours(contours.size());
    for(int i=0; i< contours.size(); ++i) {
        if( shouldProcessContour[i] ) {
            vector<Point2f> &approxContour = approximatedContours[i];
            outContours[i].resize( approxContour.size());
            int clusterId = clusterAssignments[i];
            for(int j=0; j < approxContour.size(); ++j) {
                outContours[i][j] = Point( round(approxContour[j].x), round(approxContour[j].y));
                pointClusters[clusterId].push_back( approxContour[j] );
            }
        }
    }
    pointClusterCentroids.resize(pointClusters.size());
    for(int i=0; i < pointClusters.size(); ++i) {
        const vector<Point2f> &pointCluster = pointClusters[i];
        Point2f pointClusterCentroid(0,0);
        for(int j=0; j < pointCluster.size(); ++j) {
            pointClusterCentroid += pointCluster[j];
        }
        if(pointCluster.size() > 0) {
            pointClusterCentroid *= 1.0f / pointCluster.size();
        }
        pointClusterCentroids[i] = pointClusterCentroid;
    }
    
    drawOutput(
               outContours,
               shouldProcessContour,
               clusterAssignments,
               clusterCenters,
               keyPoints,
               inputImage,
               roiOutputImage,
               sse);
    
    return true;
}

void SpotIt::geometricHashBuilding( int pointClusterIndex) {
    auto whichClusterIter =  pointClusters.begin() + pointClusterIndex;
    geometricHashBuilding(whichClusterIter, whichClusterIter+1 );
    lsHash.colorName = colorSchemesForDrawing[pointClusterIndex].name;
}

void SpotIt::geometricHashBuilding(vector<vector<Point2f> >::const_iterator b, vector<vector<Point2f> >::const_iterator e) {
    lsHash.clear();
    //KgGeometricHash<vector<Point2f>,  Quantizer<float>, KgGeometricHash_Traits< vector<Point2f> > > geomHash(contours.begin(), contours.end());
    KgGeometricHash<vector<Point2f>,  Quantizer<float> >  geomHash(b, e);
    geomHash.processTemplateSet(1.0f, lsHash);
    statsMakerMaxX.addSample(geomHash.maxValX);
    statsMakerMaxY.addSample(geomHash.maxValY);
    statsMakerMinX.addSample(geomHash.minValX);
    statsMakerMinY.addSample(geomHash.minValY);
    std::cout << "////////////////////" << std::endl;
    std::cout << statsMakerMaxX;
    std::cout << statsMakerMaxY;
    std::cout << statsMakerMinX;
    std::cout << statsMakerMinY;
    
}


void SpotIt::extractFeaturesFromRoiOutput (Mat &roiImage, vector<KeyPoint> &keypoints ) {
    
    Ptr<FeatureDetector> featureDetector = FeatureDetector::create("SIFT");
    
    featureDetector->detect(roiImage, keypoints);
    
}


void SpotIt::writePointClusters(const std::string &filename, const std::vector< std::vector< cv::Point2f > > &ptClusters) {
    FileStorage fs(filename, FileStorage::WRITE);
    fs << "clusters";
    fs << "[";
    for(int i=0; i < ptClusters.size(); ++i) {
        const vector<Point2f> &ptCluster = ptClusters[i];
        fs << "{";
        fs << "id" << i;
        fs << "comment" << "no comments";
        fs << "score" << "{";
        fs << "outlineOnly" << 80;
        fs << "noNoise" << 100;
        fs << "bigSize" << 50;
        fs << "color" << colorSchemesForDrawing[i].name;
        fs << "connectedness" << 100;
        fs << "}";
        fs << "name" << colorSchemesForDrawing[i].name;
        fs <<"cluster";
        fs << "[";
        for (int j=0; j < ptCluster.size(); ++j) {
            fs << ptCluster[j];
        }
        fs << "]";
        fs << "}";
    }
    fs << "]";
}


void SpotIt::readPointClusters(const std::string &filename, std::vector< std::vector< cv::Point2f > > &ptClusters) {
    FileStorage fs(filename, FileStorage::READ);
    if (!fs.isOpened())
    {
        cerr << "Failed to open " << filename << endl;
        throw runtime_error("failed to open file");
    }
    FileNode n = fs["clusters"];                         // Read string sequence - Get node
    if (n.type() != FileNode::SEQ)
    {
        cerr << "clusters is not a sequence! FAIL" << endl;
        throw runtime_error("clusters is not a sequence! FAIL");
    }
    
    FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
    for (; it != it_end; ++it) {
        int id = (*it)["id"];
        string name = (*it)["name"];
        FileNode ncl = (*it)["cluster"];
        string comment = (*it)["comment"];
        FileNode sc = (*it) ["score"];
        int outlineOnly =0;
        outlineOnly << (int)sc["outlineOnly"];
        int noise =0;
        noise << (int)sc["noNoise"];
        int connectedness =0;
        connectedness << (int)sc["connectedness"];
        int bigSize =0;
        bigSize << (int)sc["bigSize"];
        string color;
        color  = (string)sc["color"];
        if (ncl.type() != FileNode::SEQ)
        {
            cerr << "strings is not a sequence! FAIL" << endl;
            throw runtime_error("strings is not a sequence! FAIL");
        }
        FileNodeIterator it2 = ncl.begin(), it2_end = ncl.end(); // Go through the node
        vector<Point2f> ptCluster;
        for (; it2 != it2_end; ++it2) {
            Point2f pt;
            *it2 >> pt;
            ptCluster.push_back(pt);
            //Point2f pt (*it2);
        }
        ptClusters.push_back(ptCluster);
    }
}


bool SpotIt::handleKey(char keyChar) {
    bool retVal = false;
    switch( keyChar ) {
        case 'c':
        case 'C':
            std::cout << "SpotIt::handling key char: " << keyChar <<  std::endl;
            lsHash.serialize("geomHash.xml" );
            //writePointClusters("pointCluster.xml", pointClusters);
            //write the pointCluster to a file
            //save the output image in current directory
            retVal = true;
            break;
        default:
            break;
    }
    return retVal;
}



bool SpotIt::handleMouse( int event, int x, int y, int flags, void * userdata) {
    bool retVal = true;
    
    
    if  ( event == EVENT_LBUTTONDOWN )
    {
        //select the nearest pointClusterCentroid
        Point2f mousePoint(x, y);
        float minDistSq = numeric_limits<float>::max();
        int whichClusterIdx=-1;
        for(int i=0; i < pointClusterCentroids.size(); ++i) {
            Point2f distanceFromCluster =  pointClusterCentroids[i] - mousePoint;
            float distSq = distanceFromCluster.x * distanceFromCluster.x + distanceFromCluster.y * distanceFromCluster.y;
            if( distSq < minDistSq ) {
                minDistSq = distSq;
                whichClusterIdx = i;
            }
        }
        if(whichClusterIdx >= 0 && minDistSq < 15000.0f) {
            cout << "minDistSq: " << minDistSq << endl;
            geometricHashBuilding(whichClusterIdx);
        }

    }
    return retVal;
}

void SpotIt::approximateCurves(
                               const vector<vector<Point> > & inContours,
                               const std::vector<bool> &shouldProcessContour,
                               vector<vector<Point2f> > & outContours ) {
    int numPointsBeforeSimplification = 0;
    vector<Point> outContourInt;
    for(int i=0; i < inContours.size(); ++i) {
        numPointsBeforeSimplification += ( shouldProcessContour[i]) ? inContours[i].size() : 0;
    }
    for(int i=0; i< inContours.size(); ++i) {
        
        if( shouldProcessContour[i]) {
            vector<Point2f> &outContour = outContours[i];
            outContourInt.clear();
            approxPolyDP(Mat(inContours[i]), outContourInt, arcLength(inContours[i], false) * 0.003, false);
            outContour.resize(outContourInt.size());
            for(int j=0; j < outContourInt.size(); ++j) {
                outContour[j] = Point2f( outContourInt[j].x, outContourInt[j].y );
            }
        }
    }
    int    numPointsAfterSimplification = 0;
    for(int i=0; i < outContours.size(); ++i) {
        numPointsAfterSimplification += ( shouldProcessContour[i]) ? outContours[i].size() : 0;
    }
    cout << "num points before: " << numPointsBeforeSimplification << ", after " << numPointsAfterSimplification << endl;
}

/********************************************************************************/
//  Clustering of contour points
//
/********************************************************************************/




struct ClusterItem {
    float data_[4];
    
    ClusterItem(){
        data_[0] = data_[1] = data_[2] = data_[3] = 0;
    }
    ClusterItem(const ClusterItem &rhs) {
        data_[0] = rhs.data_[0];
        data_[1] = rhs.data_[1];
        data_[2] = rhs.data_[2];
        data_[3] = rhs.data_[3];
    }
    ClusterItem &operator=(const ClusterItem &rhs){
        data_[0] = rhs.data_[0];
        data_[1] = rhs.data_[1];
        data_[2] = rhs.data_[2];
        data_[3] = rhs.data_[3];
        return *this;
    }
    ClusterItem &operator+(const ClusterItem &rhs) {
        
        data_[0] += rhs.data_[0];
        data_[1] += rhs.data_[1];
        data_[2] += rhs.data_[2];
        data_[3] += rhs.data_[3];
        return *this;
    }
    
    ClusterItem &operator*(float scale) {
        data_[0] *= scale;
        data_[1] *= scale;
        data_[2] += scale;
        data_[3] += scale;
        return *this;
    }
    
    ClusterItem &operator-(const ClusterItem &rhs) {
        data_[0] -= rhs.data_[0];
        data_[1] -= rhs.data_[1];
        data_[2] -= rhs.data_[2];
        data_[3] -= rhs.data_[3];
        return *this;
    }
    
    //last weight item is the sum of all other weights
    friend float dot(const ClusterItem &lhs, const ClusterItem &rhs , const float weights[]) {
        //return (weights[0] * lhs.data_[0] * rhs.data_[0] +  weights[1] * lhs.data_[1] * rhs.data_[1] +  weights[2] * lhs.data_[2] * rhs.data_[2] + weights[3] * lhs.data_[3] * rhs.data_[3])/weights[4];
        return (weights[0] * lhs.data_[0] * rhs.data_[0] +  weights[1] * lhs.data_[1] * rhs.data_[1] +  weights[2] * lhs.data_[2] * rhs.data_[2] )/weights[4];
        
    }
    
    
};

//We represent each contour by a representative point (ContourRepPoint) and
//cluster the representative points of all the valid clusters.
//We choose the centroid of all the points in a contour as the representative point.
//ContourRepresentativePoint is an intermediate representation of a contour which
//captures the above statement.

struct ContourRepresentativePoint {
    Point2f meanPoint;
    Point2f variance;
    int nPointsInContour;
    cv::Scalar hsvColor;
    cv::Scalar hsvVariance;
    ContourRepresentativePoint():nPointsInContour(0),hsvVariance(0){}
    //when you add two ContourRepresentativePoint,
    //change the mean and variance of the result accordingly
    ContourRepresentativePoint & operator +(const ContourRepresentativePoint & rhs) {
        //common reciprocal term
        float reciprocal = 1.0f/( nPointsInContour + rhs. nPointsInContour);
        
        Point2f newMeanPoint =  (meanPoint * (float)nPointsInContour + rhs.meanPoint * (float)rhs.nPointsInContour )*  reciprocal;
        
        //
        Scalar newHsvColor = hsvColor;
        newHsvColor *=  (float)nPointsInContour;
        Scalar temp = rhs.hsvColor;
        temp *=  rhs.nPointsInContour;
        newHsvColor += temp;
        newHsvColor *= reciprocal;
        
        
        //calculating  new variance
        Point2f tempPoint = variance + Point2f(meanPoint.x* meanPoint.x, meanPoint.y * meanPoint.y);
        tempPoint *= (float)(nPointsInContour);
        Point2f tempPointRhs = rhs.variance + Point2f( rhs.meanPoint.x * rhs.meanPoint.x, rhs.meanPoint.y * rhs.meanPoint.y);
        tempPointRhs *= (float)(rhs.nPointsInContour);
        variance = (tempPoint + tempPointRhs) * reciprocal - Point2f(newMeanPoint.x * newMeanPoint.x, newMeanPoint.y * newMeanPoint.y);
        
        Scalar tempVar = hsvVariance + Scalar(hsvColor[0] * hsvColor[0],  hsvColor[1] * hsvColor[1], hsvColor[2] * hsvColor[2], hsvColor[3] * hsvColor[3] );
        tempVar *= (float)(nPointsInContour);
        Scalar tempVarRhs = rhs.hsvVariance + Scalar(rhs.hsvColor[0] * rhs.hsvColor[0],  rhs.hsvColor[1] * rhs.hsvColor[1], rhs.hsvColor[2] * rhs.hsvColor[2], rhs.hsvColor[3] * rhs.hsvColor[3] );
        tempVarRhs *= (float)(rhs.nPointsInContour);
        hsvVariance = (tempVar + tempVarRhs);
        hsvVariance *= reciprocal;
        hsvVariance -= Scalar( newHsvColor[0] * newHsvColor[0], newHsvColor[1] * newHsvColor[1], newHsvColor[2] * newHsvColor[2], newHsvColor[3] * newHsvColor[3]);
        
        hsvColor = newHsvColor;
        meanPoint = newMeanPoint;
        nPointsInContour += rhs.nPointsInContour;
        return *this;
    }
    
    void initializeFromContour(const vector<Point> &contour, const Mat &roiHueComponentImage) {
        
        assert(contour.size() > 0);
        Point2f sumContourPoints, sumSqContourPoints;
        Scalar sumColor(0,0,0), sumSqColor(0,0,0);
        for(int j=0; j < contour.size(); ++j ) {
            sumContourPoints += Point2f(contour[j].x, contour[j].y);
            sumSqContourPoints += Point2f(contour[j].x*contour[j].x, contour[j].y*contour[j].y);
            
            int f = static_cast<int>(roiHueComponentImage.at<uchar>(contour[j].y, contour[j].x));
            sumColor += Scalar(f, 0,0);
            sumSqColor += Scalar(f*f, 0, 0);
        }
        //mu = sigma(x)/n
        meanPoint = sumContourPoints * (1.0f/contour.size());
        //variance = sigma(x**2)/n - mu**2
        variance = sumSqContourPoints * (1.0f/contour.size()) - Point2f(meanPoint.x * meanPoint.x, meanPoint.y * meanPoint.y);
        hsvColor =  sumColor;
        hsvColor *= (1.0f/contour.size());
        hsvVariance = sumSqColor;
        hsvVariance *= (1.0f/contour.size());
        hsvVariance -= Scalar(
                              hsvColor[0] * hsvColor[0],
                              hsvColor[1] * hsvColor[1],
                              hsvColor[2] * hsvColor[2],
                              hsvColor[3] * hsvColor[3]);
        hsvVariance[1] = hsvVariance[2] = hsvVariance[3] = 1.0f;
        nPointsInContour = contour.size();
    }
    
    //meanNormalization, x = (x - mu)/sd, where mu = mean, sd = standard deviation
    template<typename Iter>
    static ContourRepresentativePoint meanNormalization(Iter b, Iter e) {
        ContourRepresentativePoint cpAvg;
        cpAvg = std::accumulate(b, e, cpAvg);
        while(b != e) {
            ContourRepresentativePoint &cRepPoint = *b;
            cRepPoint.meanPoint -= cpAvg.meanPoint;
            cRepPoint.meanPoint.x /= (sqrt(cpAvg.variance.x) + 0.000001);
            cRepPoint.meanPoint.y /= (sqrt(cpAvg.variance.y) + 0.000001);
            cRepPoint.hsvColor -= cpAvg.hsvColor;
            cRepPoint.hsvColor[0] /= sqrt(cpAvg.hsvVariance[0]);
            cRepPoint.hsvColor[1] /= sqrt(cpAvg.hsvVariance[1]);
            cRepPoint.hsvColor[2] /= sqrt(cpAvg.hsvVariance[2]);
            cRepPoint.hsvColor[3] /= sqrt(cpAvg.hsvVariance[3]);
            cRepPoint.variance = Point2f(1,1);
            cRepPoint.hsvVariance = Scalar(1,1,1,1);
            cRepPoint.nPointsInContour = 1;
            ++b;
        }
        return cpAvg;
    }
    
    
    static ContourRepresentativePoint translateBack( const ClusterItem &citem, const ContourRepresentativePoint &cpAvg) {
        ContourRepresentativePoint cpSample(cpAvg);
        cpSample.meanPoint.x = citem.data_[0] * sqrt(cpAvg.variance.x) + cpAvg.meanPoint.x;
        cpSample.meanPoint.y = citem.data_[1] * sqrt(cpAvg.variance.y) + cpAvg.meanPoint.y;
        cpSample.hsvColor[0] = citem.data_[1] * sqrt(cpAvg.hsvVariance[0]) + cpAvg.hsvColor[0];
        cpSample.variance = Point2f(1,1);
        cpSample.hsvVariance = Scalar(1,1,1,1);
        cpSample.nPointsInContour = 1;
        return cpSample;
    }
    
};



template<>
struct KMeansDataElementTraits<ClusterItem>{
    typedef ClusterItem T;
    const static float weights[];// = {1,1,9,0,11};
    
    static float getMinDist(float minDist, const T &c) {
        return minDist;
    }
    
    static T& getContribution( T &c) {
        return c;
    }
    
    static const T& getContribution( const T &c) {
        return c;
    }
    
    static int getContributingNumber( const T &c) {
        return 1;
    }
    
    
    static  bool validate( const T &c) {
        return true;
    }
    
    static float dist(const T &lhs, const T &rhs) {
        T tDiff(lhs);
        tDiff - rhs;
        return dot(tDiff, tDiff, weights);
    }
};

//weights for calculating distance between clusterItem components.
//x position weight = 3
//y position weignt = 3
//hue value weight = 5
//unused component weight = 0
//sum of all weights = 11
const float KMeansDataElementTraits<ClusterItem>::weights[] = {3,3,5,0,11};


//order the element
template<>
struct std::less<ClusterItem> : std::binary_function<ClusterItem, ClusterItem, bool> {
    bool operator()(const ClusterItem &l, const ClusterItem &r) {
        bool b = (l.data_[0]< r.data_[0] ) ? true: ( (l.data_[0] > r.data_[0] ) ? false: (
                                                                                          (l.data_[1]< r.data_[1] ) ? true: ( (l.data_[1] > r.data_[1] ) ? false: (
                                                                                                                                                                   (l.data_[2]< r.data_[2] ) ? true: ( (l.data_[2] > r.data_[2] ) ? false: (
                                                                                                                                                                                                                                            (l.data_[3]< r.data_[3]) ? true : ( (l.data_[3] > r.data_[3] ) ? false: false
                                                                                                                                                                                                                                                                               )
                                                                                                                                                                                                                                            )
                                                                                                                                                                                                      )
                                                                                                                                                                   )
                                                                                                                             )
                                                                                          )
                                                    );
        return b;
        
    }
};



//order the element for custom initialization
template<typename T, typename TTraits = KMeansDataElementTraits<T> >
struct InitComp : std::binary_function<T, T, bool> {
    
    InitComp( const T &refElement):refElement(refElement){}
    
    bool operator()(const T &l, const T &r) {
        float lmin = TTraits::dist(l, refElement);
        float rmin = TTraits::dist(r, refElement);
        return lmin < rmin;
    }
    
    const T &refElement;
};



bool SpotIt::customInitialClusterAssignment( int k,
                                            const vector<ClusterItem> &items,
                                            const vector< vector< Point > > &contours,
                                            const map<int, int> &indexReMapping,
                                            vector<ClusterItem> &clusterCenters,
                                            vector<int> &itemsSelected
                                            ) {
    clusterCenters.assign(k, ClusterItem());
    
    static RNG rng1(12345);
    
    //first let us get some statistics about the items
    //get component-wise minimum, maximum and average
    float minComponents[4];
    float maxComponents[4];
    for(int i=0; i < 4; ++i ) {
        minComponents[i] = numeric_limits<float>::max();
        maxComponents[i] = -numeric_limits<float>::max();
    }
    
    for(int i=0; i < items.size(); ++i) {
        const ClusterItem & it = items[i];
        for(int j=0; j < 4; ++j ) {
            minComponents[j] = Kg::min(it.data_[j], minComponents[j]);
            maxComponents[j] = Kg::max(it.data_[j], maxComponents[j]);
        }
    }
    
    //container for storing already selected clusterCenters
    set<ClusterItem, std::less<ClusterItem> > itemsSoFarSelectedForInitialization;
    //select up to k non-unique items
    for(int m=0; m < k; ++m) {
        int maxIterations = 100;
        ClusterItem prospectiveItem;
        //our initialization should ideally produce clusterCenters which have a pairwise distance of 'minimumDistanceBetweenSelectedClusterCenters'
        float minimumDistanceBetweenSelectedClusterCenters = 0.5f;
        bool foundMthClusterCenter = false;
        while ( !foundMthClusterCenter && (minimumDistanceBetweenSelectedClusterCenters /= 2.0f) > 0.0001) {
            //if we cant find good initializations in 100 iterations
            //try reducing minimumDistanceBetweenSelectedClusterCenters
            while(--maxIterations > 0) {
                //for the first 25 iterations try clusterCenters from the input items
                if(maxIterations > 75 ) {
                    float r = 1;
                    while(r >= 1.0f) {
                        r = Kg::uRand0To1();
                    }
                    
                    int prospectiveItemIndex =  floor(r * items.size());
                    assert( prospectiveItemIndex < items.size());
                    map<int,int>::const_iterator mit = indexReMapping.find(prospectiveItemIndex);
                    int contourIndex = mit->second;
                    double contourLength = arcLength(contours[contourIndex], false);
                    if(contourLength < 100) {
                        continue;
                    }
                    prospectiveItem = items[prospectiveItemIndex];
                    itemsSelected.push_back(prospectiveItemIndex);
                    
                } else {
                    //if the first 25 iterations doesnt yield k clusterCenters which are seperated by 'minimumDistanceBetweenSelectedClusterCenters'
                    //try randomization of the range
                    for(int j=0; j < 4; ++j) {
                        prospectiveItem.data_[j] =   rng1.uniform(minComponents[j], maxComponents[j]);
                    }
                }
                //compare already selected clusterCenters with prospectiveItem
                InitComp<ClusterItem, KMeansDataElementTraits<ClusterItem> > comp( prospectiveItem );
                auto min_elem_iter = std::min_element( itemsSoFarSelectedForInitialization.begin(), itemsSoFarSelectedForInitialization.end(), comp);
                float dist_from_other_elements_so_far = 0.0f;
                if(min_elem_iter != itemsSoFarSelectedForInitialization.end() ) {
                    dist_from_other_elements_so_far = KMeansDataElementTraits<ClusterItem>::dist(*min_elem_iter, prospectiveItem );
                }
                if( (min_elem_iter == itemsSoFarSelectedForInitialization.end())  || dist_from_other_elements_so_far > minimumDistanceBetweenSelectedClusterCenters) {
                    //if the prospectiveItem has a pairwise distance of atleast minimumDistanceBetweenSelectedClusterCenters,
                    //the go ahead and move to the next 'm'th clusterItem
                    itemsSoFarSelectedForInitialization.insert(prospectiveItem);
                    foundMthClusterCenter = true;
                    break;
                }
            }
        }
        //hangup on failure
        if( minimumDistanceBetweenSelectedClusterCenters <= 0.0001) {
            KG_DBGOUT ( std::cout << "Custom Initialization:  cant find initialization"  << std::endl );
            clusterCenters.clear();
            return false;
        }
        
        clusterCenters[m] = prospectiveItem;
    }
    return true;
}


void SpotIt::cluster(
                     const Mat &roiHueComponentImage,
                     const vector< vector< Point > > &contours,
                     const  vector<bool> &shouldProcessContour,
                     vector<int> &clusterAssignments,
                     vector<ContourRepresentativePoint> &clusterCenters,
                     int &numClusters,
                     float &sse) {
    
    //init
    map<int, int> indexReMapping;
    int numGoodContours=0;
    for(int i=0; i < shouldProcessContour.size(); ++i) {
        if(shouldProcessContour[i] && contours[i].size() > 0) {
            indexReMapping[numGoodContours++] = i;
        }
    }
    
    //get  an intermediate representation
    vector<ContourRepresentativePoint> contourRepPoints(numGoodContours);
    
    for(int j=0; j < contourRepPoints.size(); ++j) {
        map<int, int>::const_iterator mit = indexReMapping.find(j);
        assert(mit != indexReMapping.end());
        int indexInContours = mit->second;
        ContourRepresentativePoint &cRepPoint = contourRepPoints[j];
        cRepPoint.initializeFromContour(contours[indexInContours], roiHueComponentImage);
    }
    //do meanNormalization
    ContourRepresentativePoint cpAvg = ContourRepresentativePoint::meanNormalization< vector<ContourRepresentativePoint>::iterator >(contourRepPoints.begin(), contourRepPoints.end());
    
    //marshall the intermediate representation to clusterItems
    vector<ClusterItem> clusterItems(numGoodContours);
    for(int i=0; i < numGoodContours; ++i) {
        ClusterItem &ci = clusterItems[i];
        const ContourRepresentativePoint &cp = contourRepPoints[i];
        ci.data_[0] = cp.meanPoint.x;
        ci.data_[1] = cp.meanPoint.y;
        ci.data_[2] = cp.hsvColor[0];
    }
    
    
    //desired number of clusters
    int desiredK=8;
    
    
    //randomly initialize points from the image
    vector<ClusterItem> clusterCentersInitialization;
    vector<int> itemsSelected;
    bool wasSuccessful = customInitialClusterAssignment(
                                                        desiredK,
                                                        clusterItems,
                                                        contours,
                                                        indexReMapping,
                                                        clusterCentersInitialization,
                                                        itemsSelected);
    KMeansClustering<ClusterItem> cluster(desiredK, clusterItems, clusterCentersInitialization);
    KG_DBGOUT(cout << "#################" << endl);
    for(int j=0; j < itemsSelected.size(); ++j) {
        map<int, int>::const_iterator mit = indexReMapping.find(itemsSelected[j]);
        int contourIdx = mit->second;
        KG_DBGOUT(cout << contourIdx << ": " << arcLength(contours[contourIdx], false) << ", ");
    }
    KG_DBGOUT(cout << endl);
    //clustering
    cluster.doIt();
    
    //gather output
    sse = cluster.getOutputLastSSEForAllItems();
    const vector<int> &outputClusterAssignments = cluster.getOutputClusterAssignments();
    clusterAssignments.assign(contours.size(), -1);
    
    const vector<ClusterItem> &citems = cluster.getClusterCenters();
    clusterCenters.assign(citems.size(), ContourRepresentativePoint());
    for(int i=0; i < citems.size(); ++i) {
        clusterCenters[i] =  ContourRepresentativePoint::translateBack( citems[i], cpAvg);
    }
    
    for(int j=0; j < outputClusterAssignments.size(); ++j) {
        map<int, int>::const_iterator mit = indexReMapping.find(j);
        assert(mit != indexReMapping.end());
        int indexInContours = mit->second;
        clusterAssignments[indexInContours] = outputClusterAssignments[j];
    }
    
    numClusters = desiredK;
}


void SpotIt::drawOutput(
                        const vector<vector<Point>> &contours,
                        const vector<bool> &shouldProcessContour,
                        const vector<int> &clusterAssignments,
                        const vector<ContourRepresentativePoint> &clusterCenters,
                        const vector<KeyPoint> &keyPoints,
                        Mat &inputImage,
                        Mat &roiOutputImage,
                        float sse){
    static char sbuffer[1024];
    
    for(int i=0; i < contours.size(); ++i) {
        if(!shouldProcessContour[i]) {
            continue;
        }
        
        int iColorIdx = clusterAssignments[i];
        drawContours(
                     roiOutputImage,
                     contours,
                     i,
                     colorSchemesForDrawing[iColorIdx].value
                     );
        drawContours(
                     inputImage,
                     contours,
                     i,
                     colorSchemesForDrawing[iColorIdx].value
                     );
        
    }
    for(int i=0; i < clusterCenters.size(); ++i) {
        
        Point clusterCenterImagePoint = Point(round(clusterCenters[i].meanPoint.x), round(clusterCenters[i].meanPoint.y));
        circle(roiOutputImage, clusterCenterImagePoint , 4, colorSchemesForDrawing[i].value, -1, 8);
        
    }
    
    Scalar featureColor(255, 255, 255);
    drawKeypoints(roiOutputImage, keyPoints, roiOutputImage, featureColor, DrawMatchesFlags::DEFAULT);
    
    
    
    sprintf(sbuffer, "sse. %f", sse);
    putText(roiOutputImage, sbuffer, cvPoint(20,20),
            FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(200,200,250), 1, CV_AA);
}