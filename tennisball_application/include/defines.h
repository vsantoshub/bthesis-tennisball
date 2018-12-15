/*
 * defines.h
 *
 *  Created on: Sep 14, 2014
 *  Author:   Victor Santos (viic.santos@gmail.com)
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CAMERA_DEVICE "/dev/video0"
#define CAMERA_COLS 320   /* 320 */
#define CAMERA_ROWS 240   /* 240 */
#define CAMERA_FPS 30     /* 30 */
#define MID_SCREEN_Y CAMERA_ROWS/2
#define MID_SCREEN_X CAMERA_COLS/2

#define ACPT_ERROR 6 /* 10 */ /* acceptable error inn servo movement through coordinates*/

//uncomment the #undefs to turn ON the desired feature
/************************************/
/* CAPTURE MODE */

/*original opencv functions to access camera*/
#define CLASSIC_OPENCV
#undef CLASSIC_OPENCV

/*beldenfox mjpeg functions to access the camera, using directly v4l2*/
#define OCVCAPTURE_MJPEG
#undef OCVCAPTURE_MJPEG

/*beldenfox yuyv functions to access the camera, using directly v4l2*/
#define OCVCAPTURE_YUYV
//#undef OCVCAPTURE_YUYV

/*another yuyv kind of v4l2 library to access camera functions, just for comparison: best than the others above*/
#define LIBCAM
#undef LIBCAM
/************************************/

/************************************/

#define SERVO_ON
//#undef SERVO_ON

/************************************/
/* DIP RELATED */
//split hsv
#define SPLIT_HSV_PLANE //used in manual threshold, just disable this if INRANGE_THRESHOLD IS USED
//#undef SPLIT_HSV_PLANE

#define DEBUG_SPLIT_HSV
#undef DEBUG_SPLIT_HSV


/************************************/
//turns on/off hsv cannonical histogram equalization
#define EQ_HSV_HIST
#undef EQ_HSV_HIST

#ifdef EQ_HSV_HIST
#ifndef SPLIT_HSV_PLANE
#define SPLIT_HSV_PLANE
#endif //SPLIT_HSV_PLANE
#endif //EQ_HSV_HIST

#define DEBUG_HSV_HIST
#undef DEBUG_HSV_HIST

/************************************/
//turns on/off manual hsv threshold
#define MANUAL_THRESHOLD
//#undef MANUAL_THRESHOLD

#ifdef MANUAL_THRESHOLD
#ifndef SPLIT_HSV_PLANE
#define SPLIT_HSV_PLANE
#endif //SPLIT_HSV_PLANE
#endif //MANUAL_THRESHOLD

#define MANUAL_THRESHOLD_CFG
#undef MANUAL_THRESHOLD_CFG

#define DEBUG_MANUAL_THRESHOLD
#undef DEBUG_MANUAL_THRESHOLD

/************************************/
//turns on/off morphologic transformation
#define MORPH_TRANSFORM
//#undef MORPH_TRANSFORM
#define DEBUG_MORPH
#undef DEBUG_MORPH

/************************************/
//turns on/off contour drawing and filter
#define DRAW_CONTOUR
//#undef  DRAW_CONTOUR
#ifdef DRAW_CONTOUR
/*NEEDS MORPH TRANSFORM*/
#ifndef MORPH_TRANSFORM
#define MORPH_TRANSFORM
#endif //MORPH_TRANSFORM
#endif //DRAW_CONTOUR

#define DEBUG_CONTOUR
#undef DEBUG_CONTOUR
/************************************/
//turns on/off hough transform after contour
#define HOUGH_TRANSFORM
#undef  HOUGH_TRANSFORM
/*NEEDS MORPH TRANSFORM*/
#ifdef HOUGH_TRANSFORM
#ifndef MORPH_TRANSFORM
#define MORPH_TRANSFORM
#endif //MORPH_TRANSFORM
#endif //HOUGH_TRANSFORM

#define DEBUG_HOUGH
#undef DEBUG_HOUGH
/************************************/
//turns on/off screen printed debug
#define SCREEN_DEBUG
#undef SCREEN_DEBUG
    
/************************************/
#define DEBUG_ORIGINAL
#undef DEBUG_ORIGINAL
#define DEBUG_HSV
#undef DEBUG_HSV

#define ORIGINAL_HISTOGRAM
#undef ORIGINAL_HISTOGRAM

/************************************/
/*Contrast Limited Adaptive Histogram Equalization*/
#define EQ_CLAHE
//#undef EQ_CLAHE
#define DEBUG_EQ_CLAHE
#undef DEBUG_EQ_CLAHE
#define DEBUG_CLAHE_HIST
#undef DEBUG_CLAHE_HIST

/************************************/
/*new contour method used to find circles based on trigonometric properties*/    
#define NEW_CONTOUR_METHOD
//#undef NEW_CONTOUR_METHOD
#define DEBUG_NEW_CONTOUR
#undef DEBUG_NEW_CONTOUR

/*just some printfs to debug when I cant use X server*/
#define MINIMAL_DEBUG
#undef MINIMAL_DEBUG

/*used when debug is needed when machine in out from home spot*/
#define DEBUG_IN_LOCO
#undef DEBUG_IN_LOCO
#ifdef DEBUG_IN_LOCO
#define MANUAL_THRESHOLD_CFG
#define SCREEN_DEBUG
//#define DEBUG_ORIGINAL
//#define DEBUG_EQ_CLAHE
#define DEBUG_MORPH
#define DEBUG_CONTOUR
#endif

/************************************/
/************************************/
#ifdef __cplusplus
}
#endif

#endif /* DEFINES_H_ */
