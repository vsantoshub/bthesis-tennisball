#ifndef NEW_CONTOUR_METHOD_H_
#define NEW_CONTOUR_METHOD_H_

/**
* Author:   Victor Santos (viic.santos@gmail.com)
* Simple shape detector program.
* It loads an image and tries to find simple shapes (rectangle, triangle, circle, etc) in it.
* This program is a modified version of `squares.cpp` found in the OpenCV sample dir.
*/
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour);



#ifdef __cplusplus
}
#endif


#endif
