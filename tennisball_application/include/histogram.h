#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

/**
* Author:   Victor Santos (viic.santos@gmail.com)
* Code sample for displaying image histogram in OpenCV.
*/
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

void showHistogram(Mat& img);
void showHistogram_rgb(Mat& img, char * r, char * g, char * b);

#ifdef __cplusplus
}
#endif


#endif
