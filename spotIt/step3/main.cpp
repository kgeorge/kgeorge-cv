/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/

#include <iostream>
#include <fstream>
#include <strstream>

#include <ctime>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include "spotIt.hpp"
#include "kgUtils.hpp"


using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    
    //initialize the random number generator
    srand (time(NULL));
    
    VideoCapture cap(0);
    //video shows the current scene
    namedWindow( "video");
    
    
    Mat roiOutputImageLast;
    SpotIt spotIt;
    bool continueProcessing = true;
    for(;  ;) {
        //for each frame
        Mat frame;
        Mat roiOutputImage;
        cap >> frame;
        Mat image, tempImage, grayImage;
        if( frame.empty() )
            break;
        frame.copyTo(image);
        
        //first detect the single circle
        cvtColor(frame, tempImage, CV_BGR2GRAY);
        pyrDown(tempImage, grayImage, Size(frame.cols/2, frame.rows/2));
        vector<Vec3f> circles;
        HoughCircles(
                     grayImage,        //input image
                     circles,               //output circles
                     CV_HOUGH_GRADIENT,
                     1,                      //
                     grayImage.rows/8,  //
                     250,                        //upperThresholdForInternalCannyEdgeDetect
                     50,                          //thresholdForCenterDetect
                     40,                          //minRadius
                     80                           //maxRadius,
                     );
        vector<int> numItemsInCluster;
        if(circles.size()>0 && continueProcessing ) {
            double startTime = getTickCount();

            //draw only the first circle
            //todo: this need be extended to detect two circular regions
            Point circleCenter(  round(circles[0][0])*2, round(circles[0][1])*2);
            double circleRadius = circles[0][2]*2;
            spotIt.processCircle(
                                 circleCenter, //center of circle detected
                                 circleRadius,  //radius of circle detected
                                 image,  //input BGR image
                                 roiOutputImage//output BGR image of processed region
                                 );
            roiOutputImage.copyTo(roiOutputImageLast);
            double endTime = getTickCount();

            cout << "frameRate: " <<  getTickFrequency()/(endTime - startTime) << endl;
            continueProcessing = false;
        }
        if(!roiOutputImageLast.empty()) {
            Mat dst;
            //Size sz = roiOutputImageLast.size();
            Mat image_reg = image(Rect(0,0,roiOutputImageLast.cols, roiOutputImageLast.rows));
            Size s1 = image_reg.size();
            Size s2 = roiOutputImageLast.size();
            addWeighted(image_reg, 0, roiOutputImageLast, 1, 0, dst);
            dst.copyTo(image(Rect(0,0,roiOutputImageLast.cols, roiOutputImageLast.rows)));
        }
        imshow("video", image);
        
        strstream ss;
        
        ofstream myfile;
        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch( c ) {
            case 's':
            case 'S':
                //save the output image in current directory
                imwrite("roiOutputImageLast.jpg", roiOutputImageLast );
                break;


            case 'n':
            case 'N':
                continueProcessing = true;
                break;
            default:
                break;
        }
    }
    cap.release();
    destroyAllWindows();
    
}

