/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/
#include <iostream>
#include <strstream>

#include <ctime>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include "spotIt.hpp"
#include "kgUtils.hpp"
#include "kMeansClustering.hpp"

using namespace std;
using namespace cv;

#if 0

void doIt(
          const Mat &hsvImage,
          const vector< vector< Point > > &contours,
          const  vector<bool> &shouldProcessContour,
          vector<int> &clusterAssignments,
          int &numClusters ) {
    vector<ContourRepPoint> contourRepPoint(contours.size());
    for(int i=0; i < contours.size(); ++i) {
        const vector< Point> &curContour = contours[i];
        if(!shouldProcessContour[i]) {
            continue;
        }
        assert(curContour.size() > 0);
        Point meanPoint(0,0);
        Scalar colorAvg(0,0,0);
        for(int j=0; j < curContour.size(); ++j ) {
            meanPoint += curContour[j];
            int f = static_cast<int>(hsvImage.at<uchar>(curContour[j].y, curContour[j].x));
            colorAvg += Scalar(f, 0, 0);
        }
        meanPoint = meanPoint * (1.0/curContour.size());
        colorAvg = colorAvg * (1.0/curContour.size());
        Point variance(0,0);
        for(int j=0; j < curContour.size(); ++j ) {
            Point meanDiff = curContour[j] - meanPoint;
            variance += Point(meanDiff.x * meanDiff.x, meanDiff.y * meanDiff.y);
        }
        variance = variance * (1.0/curContour.size());
        ContourRepPoint &cstats = contourRepPoint[i];
        cstats.mean = meanPoint;
        cstats.variance = variance;
        cstats.nPointsInContour = curContour.size();
        cstats.hsvColor = colorAvg;
    }


    int optimalK = -1;
    int k=9;
    float minErrorSoFar = std::numeric_limits<float>::max();
    KMeansClustering<ContourRepPoint> cluster(k, contourRepPoint, shouldProcessContour);
    cluster.doIt();
    if(cluster.lastTotalError < minErrorSoFar) {
        optimalK = k;
        minErrorSoFar = cluster.lastTotalError;
    }
    clusterAssignments.swap(cluster.clusterAssignments);
    for(int j =0; j < contourRepPoint.size(); ++j) {
        assert(!shouldProcessContour[j] || clusterAssignments[j] >= 0);
    }
    numClusters = k;
}



 void spotItProcessCircle( Point &circleCenter, double circleRadius, Mat & image, Mat &roiImageCopy) {
    //make random colors for displaying
    static RNG rng1(12345), rng2(54321);
    static const Scalar colorMark[] = {
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) )

    };
    KG_DBGOUT( cout << "  image type " << image.type() << "  image depth " << image.depth() << endl );
    vector<Vec4i> hierarchy;
    int roiDim = circleRadius * 2.2;
    Point roiOrigin(Kg::max(circleCenter.x - roiDim/2, 0) , Kg::max(circleCenter.y - roiDim/2, 0));
    int adjustedRoiWidth = Kg::min( (int)roiDim  , image.cols - roiOrigin.x);
    int adjustedRoiHeight = Kg::min( (int)roiDim  , image.rows - roiOrigin.y);
    Point circleCenterInRoi = circleCenter - roiOrigin;

    Mat tempImage = image(Rect(roiOrigin.x, roiOrigin.y, adjustedRoiWidth, adjustedRoiHeight));
    Mat roiImage2, roiImage, hsvImage;
    tempImage.copyTo(roiImageCopy);
    roiImage.create(roiImageCopy.size(), roiImageCopy.type());
    cvtColor(roiImageCopy, roiImage, CV_BGR2GRAY);
    cvtColor(roiImageCopy, hsvImage, CV_BGR2HSV);
    vector<Mat> hsvComponents;
    split(hsvImage, hsvComponents);
    double hysterisisRatio =  3.0;
    int lowThreshold = 70;
    int kernelSize = 3;
    //equalizeHist( roiImage2,  roiImage );
    Canny(roiImage, roiImage, lowThreshold, hysterisisRatio * lowThreshold, kernelSize );
    Mat visualGroups;
    int numGroups = 0;
    vector<vector<Point> > contours;

    findContours(
        roiImage,
        contours,
        hierarchy,
        //CV_RETR_TREE,
        CV_RETR_EXTERNAL,
        //CV_CHAIN_APPROX_SIMPLE,
        CV_CHAIN_APPROX_TC89_L1
        //,roiOrigin
    );
    const double circleRadiusSq = circleRadius * circleRadius;
    vector<bool> shouldProcessContour(contours.size(), false);
    int nContoursGood = 0;
    for(int i=0; i < contours.size(); ++i) {
        vector<Point> &contour = contours[i];
        int numPointsInCircle = 0;
        for(int j=0; j < contour.size(); ++j) {
            Point pRelativeToCircleCenter = contour[j] - circleCenterInRoi;
            double distSqOfPointToCenter = pRelativeToCircleCenter.x * pRelativeToCircleCenter.x  + pRelativeToCircleCenter.y  * pRelativeToCircleCenter.y;
            if( ( circleRadiusSq - distSqOfPointToCenter )  > 10 ) {
                ++numPointsInCircle;
            }
        }
        double percentageNumPointsInCircle = (double)numPointsInCircle * 100.0/ contour.size();
        if( percentageNumPointsInCircle > 90.0 ) {
            assert(contour.size() > 0);
            shouldProcessContour[i] = true;
            /*
            drawContours(
                image,
                contours,
                i,
                colorMark[iColorIdx]
            );*/
            nContoursGood++;
        }
    }
    if(nContoursGood <= 0) {
        return;
    }
     int numClusters=0;
     vector<int> clusterAssignments;
     doIt(     hsvComponents[0],
               contours,
               shouldProcessContour,
               clusterAssignments,
          numClusters );




     //cvtColor(roiImageCopy, roiImage, CV_BGR2GRAY);
     roiImageCopy = Scalar(0,0,0);
     //assert(sizeof(colorMark)/sizeof(Scalar(0,0,0)) > numClusters);
    for(int i=0; i < contours.size(); ++i) {
        if(!shouldProcessContour[i]) {
            continue;
        }

        int iColorIdx = clusterAssignments[i];
        drawContours(
                     roiImageCopy,
                     contours,
                     i,
                     colorMark[iColorIdx]
                     );
        drawContours(
                     image,
                     contours,
                     i,
                     colorMark[iColorIdx]
                     );

    }
    imshow("roi", roiImage);

 }

#endif




bool SpotIt::processCircle(
        cv::Point &circleCenter,
        float circleRadius,
        Mat &inputImage,
        Mat &roiOutputImage
) {
    KG_DBGOUT( cout << "  inputImage type " << inputImage.type() << "  inputImage depth " << inputImage.depth() << endl );

    vector<Vec4i> hierarchy;

    //find an roi in input image

    //make roi slightly bigger than the circle specified
    int roiDim = circleRadius * 2.2;
    //roiOrigin
    Point roiOrigin(Kg::max(circleCenter.x - roiDim/2, 0) , Kg::max(circleCenter.y - roiDim/2, 0));
    //roi width, height
    int adjustedRoiWidth = Kg::min( (int)roiDim  , inputImage.cols - roiOrigin.x);
    int adjustedRoiHeight = Kg::min( (int)roiDim  , inputImage.rows - roiOrigin.y);
    Point circleCenterInRoi = circleCenter - roiOrigin;
    //reference to part of inputImage that covers roi
    Mat tempImage = inputImage(Rect(roiOrigin.x, roiOrigin.y, adjustedRoiWidth, adjustedRoiHeight));


    //inout image processing

    Mat  roiGrayImage, roiHsvImage;
    //copy to
    cvtColor(tempImage, roiGrayImage, CV_BGR2GRAY);
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
        for(int j=0; j < contour.size(); ++j) {
            Point pRelativeToCircleCenter = contour[j] - circleCenterInRoi;
            double distSqOfPointToCenter = pRelativeToCircleCenter.x * pRelativeToCircleCenter.x  + pRelativeToCircleCenter.y  * pRelativeToCircleCenter.y;
            if( ( circleRadiusSq - distSqOfPointToCenter )  > 10 ) {
                ++numPointsInCircle;
            }
        }
        double percentageNumPointsInCircle = (double)numPointsInCircle * 100.0/ contour.size();
        //if 90% of pointsof contour falls within the circle,
        //then this is a contour that need be further processed
        if( percentageNumPointsInCircle > 90.0 ) {
            assert(contour.size() > 0);
            shouldProcessContour[i] = true;
            nContoursGood++;
        }
    }
    if(nContoursGood <= 0) {
        //if no contours were produced
        //return false
        return false;
    }

     //cluster contours based on KMeans
     int numClusters=0;
     vector<int> clusterAssignments;
     float sse = numeric_limits<float>::max();
     cluster(
        roiHsvComponents[0], //hue component of roi
        contours,           //output from findContours
        shouldProcessContour, //flag whether each contour need be processed
        clusterAssignments, //output cluster assignments
        numClusters,  //number of clusters,
        sse //sum of squared error
     );

     //draw the output onto the output images
     roiOutputImage.create(roiGrayImage.size(), inputImage.type() );
     roiOutputImage = Scalar(0,0,0);
     drawOutput(
        contours,
        shouldProcessContour,
        clusterAssignments,
        inputImage,
        roiOutputImage);

    return true;
}





void SpotIt::drawOutput(
    const vector<vector<Point>> &contours,
    const vector<bool> &shouldProcessContour,
    const vector<int> &clusterAssignments,
    Mat &inputImage,
    Mat &roiOutputImage){


    //make random colors for displaying
    static RNG rng1(12345), rng2(54321);
    static const Scalar colorMark[] = {
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) ),
        Scalar( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) )

    };

     for(int i=0; i < contours.size(); ++i) {
            if(!shouldProcessContour[i]) {
                continue;
            }

            int iColorIdx = clusterAssignments[i];
            drawContours(
                         roiOutputImage,
                         contours,
                         i,
                         colorMark[iColorIdx]
                         );
            drawContours(
                         inputImage,
                         contours,
                         i,
                         colorMark[iColorIdx]
                         );

        }
}


/********************************************************************************/
//  Clustering of contour points
//
/********************************************************************************/

//We represent each contour by a representative point (ContourRepPoint) and
//cluster the representative points of all the valid clusters.
//We choose the centroid of all the points in a contour as the representative point.


struct ContourRepresentativePoint {
    Point2f meanPoint;
    Point2f variance;
    int nPointsInContour;
    cv::Scalar hsvColor;
    ContourRepresentativePoint():nPointsInContour(0){}
    ContourRepresentativePoint & operator +(const ContourRepresentativePoint & rhs) {
        //common reciprocal term
        float reciprocal = 1.0f/( nPointsInContour + rhs. nPointsInContour);

        Point2f newMeanPoint =  (meanPoint * (float)nPointsInContour + rhs.meanPoint * (float)rhs.nPointsInContour )*  reciprocal;

        //
        hsvColor *=  (float)nPointsInContour;
        Scalar temp = rhs.hsvColor;
        temp *=  rhs.nPointsInContour;
        hsvColor += temp;
        hsvColor *= reciprocal;

        //calculating  new variance
        Point2f tempPoint = variance + Point2f(meanPoint.x* meanPoint.x, meanPoint.y * meanPoint.y);
        tempPoint *= (float)(nPointsInContour);
        Point2f tempPointRhs = rhs.variance + Point2f( rhs.meanPoint.x * rhs.meanPoint.x, rhs.meanPoint.y * rhs.meanPoint.y);
        tempPointRhs *= (float)(rhs.nPointsInContour);
        variance = (tempPoint + tempPointRhs) * reciprocal - Point2f(newMeanPoint.x * newMeanPoint.x, newMeanPoint.y * newMeanPoint.y);

        meanPoint = newMeanPoint;
        nPointsInContour += rhs.nPointsInContour;
        return *this;
    }
    
    void initializeFromContour(const vector<Point> &contour, const Mat &roiHueComponentImage) {
    
    assert(contour.size() > 0);
        Point2f sumContourPoints, sumSqContourPoints;
        Scalar sumColor(0,0,0);
    for(int j=0; j < contour.size(); ++j ) {
        sumContourPoints += Point2f(contour[j].x, contour[j].y);
        sumSqContourPoints += Point2f(contour[j].x*contour[j].x, contour[j].y*contour[j].y);

        int f = static_cast<int>(roiHueComponentImage.at<uchar>(contour[j].y, contour[j].x));
        sumColor += Scalar(f, 0,0);
    }
    //mu = sigma(x)/n
    meanPoint = sumContourPoints * (1.0f/contour.size());
    //variance = sigma(x**2)/n - mu**2
    variance = sumSqContourPoints * (1.0f/contour.size()) - Point2f(meanPoint.x * meanPoint.x, meanPoint.y * meanPoint.y);
    hsvColor =  sumColor;
    hsvColor *= (1.0f/contour.size());
    nPointsInContour = contour.size();
    }

};

//order the element according to the hue color
template<>
struct std::less<ContourRepresentativePoint> : std::binary_function<ContourRepresentativePoint, ContourRepresentativePoint, bool> {
    bool operator()(const ContourRepresentativePoint &l, const ContourRepresentativePoint &r) {
        return (l.hsvColor[0]< r.hsvColor[0] );
    }
};

template<>
KMeansClustering<ContourRepresentativePoint>::~KMeansClustering(){}



template<>
struct KMeansDataElementTraits<ContourRepresentativePoint>{
    typedef ContourRepresentativePoint T;

    static float getMinDist(float minDist, const T &c) {
        return minDist;
    }

    static ContourRepresentativePoint& getContribution( ContourRepresentativePoint &c) {
        return c;
    }

    static const ContourRepresentativePoint& getContribution( const ContourRepresentativePoint &c) {
        return c;
    }

    static int getContributingNumber( const T &c) {
         return 1;
    }


    static  bool validate( const ContourRepresentativePoint &c) {
        return c.nPointsInContour > 0;
    }

    static float dist(const ContourRepresentativePoint &lhs, const ContourRepresentativePoint &rhs) {
        Scalar pdiff = lhs.hsvColor - rhs.hsvColor;
        return pdiff[0] * pdiff[0] ;
    }
};


void SpotIt::cluster(
          const Mat &roiHueComponentImage,
          const vector< vector< Point > > &contours,
          const  vector<bool> &shouldProcessContour,
          vector<int> &clusterAssignments,
          int &numClusters,
          float &sse) {

    //init
    vector<ContourRepresentativePoint> contourRepPoints(contours.size());

    for(int i=0; i < contours.size(); ++i) {
        const vector< Point> &curContour = contours[i];
        if(!shouldProcessContour[i]) {
            continue;
        }
        ContourRepresentativePoint &cRepPoint = contourRepPoints[i];
        assert(curContour.size() > 0);
        cRepPoint.initializeFromContour(curContour, roiHueComponentImage);
    }
    //desired number of clusters
    int desiredK=9;
    KMeansClustering<ContourRepresentativePoint> cluster(desiredK, contourRepPoints, shouldProcessContour);
    cluster.doIt();

    sse = cluster.getOutputLastSSEForAllItems();
    vector<int> &outputClusterAssignments = cluster.getOutputClusterAssignments();

    clusterAssignments.swap(outputClusterAssignments);
    numClusters = desiredK;


    //verification that each valid contour is assigned to a cluster
    for(int j =0; j < contourRepPoints.size(); ++j) {
        assert(!shouldProcessContour[j] || clusterAssignments[j] >= 0);
    }


}
