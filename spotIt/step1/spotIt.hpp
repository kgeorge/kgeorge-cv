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

#include <opencv2/core/core.hpp>

void spotItProcessCircle(
                         cv::Point &circleCenter,  //center of circular region
                         double circleRadius,  //radius of circular region
                         cv::Mat & image,   //input image
                         cv::Mat &roiOutputImageCopy //output image of circular region

);


class SpotIt {
public:
    SpotIt(){}
    
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
                 int &numClusters, //out
                 float &sse //out, sum of squared error
    );
    
    
    void drawOutput(
                    const std::vector<std::vector<cv::Point>> &contours,
                    const std::vector<bool> &shouldProcessContour,
                    const std::vector<int> &clusterAssignments,
                    cv::Mat &inputImage,
                    cv::Mat &roiOutputImage);
    
};


#endif //SPOTIT_H_