/*
* File: tasks.c
* Author:   Victor Santos (viic.santos@gmail.com)
* Comment: Tarefas genéricas
*
*/
/*original includes*/
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iomanip>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

#include "tasks.h"
#include "debug.h"

#include "defines.h"
#include "relay.h"
#include "monster_shield.h"
#include "servo.h"

/* OpenCV app includes */
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <iostream>
#include <stdio.h>
#include <string>

/* beldenfox capture methods*/
#include "OCVCapture_MJPEG.h"
#include "OCVCapture_YUYV.h"

/* libcam capture method*/
#include "libcam.h"

#include "histogram.h"
#include "new_contour_method.h"

using namespace std;
using namespace cv;

/*servo*/
//Variables for keeping track of the current servo positions.
static unsigned char servoTiltPosition = 60;
const int stepUP = 1;
const int stepDOWN = 3;

/* threads */
static pthread_t taskOpenCV_thread;
static pthread_t taskCollector_thread;

/* mutex e condition */
static pthread_mutex_t taskOpenCV_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t taskOpenCV_cond = PTHREAD_COND_INITIALIZER;
static int taskOpenCV_running=0;
static int taskOpenCV_exit;

static pthread_mutex_t taskCollector_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t taskCollector_cond = PTHREAD_COND_INITIALIZER;
static int taskCollector_action;
static int taskCollector_delay;

static unsigned int searching_balls = 0;

static int taskCollector(void) {
    sigset_t mask;
    
    /* block signals */
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGABRT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    DBG(DBG_TRACE, "PID: %d, PPID: %d", getpid(), getppid());
    
    for (;;) {
        
        /*
        waiting...
        */
        pthread_mutex_lock(&taskCollector_mtx);
        
        DBG(DBG_TRACE, "taskCollector stopped, waiting...");
        
        pthread_cond_wait(&taskCollector_cond, &taskCollector_mtx);
        pthread_mutex_unlock(&taskCollector_mtx);
        
        /*
        fazendo algo...
        */
        if (taskCollector_action == COLLECTOR_RUN) {
            
            DBG(DBG_TRACE, "RUN");
            coletor_on();
            
        } else if (taskCollector_action == COLLECTOR_STOP) {
            
            DBG(DBG_TRACE, "STOP");
            coletor_off();
            
        } else if (taskCollector_action == COLLECTOR_TIMER) {
            
            DBG(DBG_TRACE, "TIMER");
            coletor_on();
            while ((taskCollector_action == COLLECTOR_TIMER) && (taskCollector_delay > 0)) {
                
                sleep(1);
                
                pthread_mutex_lock(&taskCollector_mtx);
                taskCollector_delay--; //time while collector stay turned ON
                pthread_mutex_unlock(&taskCollector_mtx);
                
                DBG(DBG_TRACE, "taskCollector_delay(%d)", taskCollector_delay);
            }
            
            /* parar o motor */
            DBG(DBG_TRACE, "COLLECTOR STOP");
            coletor_off();
        }
        DBG(DBG_TRACE, "taskCollector voltando a dormir.");
        fflush(stdout);
    }
    
    return 0;
}

static int taskOpenCV(void) {
    sigset_t mask;
    printf("entrou task OpenCV\n");
    /* block signals */
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGABRT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    DBG(DBG_TRACE, "PID: %d, PPID: %d", getpid(), getppid());
    
    taskOpenCV_exit = 0;
    
    #if 1 //se quiser matar a task inteira para testar
        /* ----------------- init opencv variables --------------------------------*/
    #ifdef CLASSIC_OPENCV
    CvCapture* capture = cvCaptureFromCAM(0);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAMERA_COLS);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAMERA_ROWS);
    #endif
    
    #ifdef OCVCAPTURE_MJPEG
    OCVCapture_MJPEG capture;
    capture.deviceID((char *)CAMERA_DEVICE);
    capture.setDesiredSize(CAMERA_COLS, CAMERA_ROWS);
    capture.setFramerate(CAMERA_FPS);
    capture.setPixelFormat((char *)"MJPEG");
    capture.setVerbose(true);
    capture.open();
    
    if (!capture.isOpen())
    {
        cerr << "Failed to open camera" << endl;
        return -1;
    }
    
    cout << "Capture " << capture.width() << " x " << capture.height() << endl;
    cout << " pixels at " << capture.frameRate() << " fps" << endl;
    
    // At the beginning, several frames tend to come out black.
    for (int i = 0; i < 20; ++i)
    {
        capture.grab();
        usleep(1000);
    }
    #endif
    
    #ifdef OCVCAPTURE_YUYV
    OCVCapture_YUYV capture;
    capture.deviceID((char *) CAMERA_DEVICE);
    capture.setDesiredSize(CAMERA_COLS, CAMERA_ROWS);
    capture.setDesiredFrameRate(CAMERA_FPS);
    capture.setVerbose(true);
    capture.open();
    
    if (!capture.isOpen()) {
        cerr << "Failed to open camera" << endl;
        return -1;
    }
    
    cout << "Capture " << capture.width() << " x " << capture.height();
    cout << " pixels at " << capture.frameRate() << " fps" << endl;
    
    // The first several frames tend to come out black.
    for (int i = 0; i < 20; ++i) {
        capture.grab();
        usleep(1000);
    }
    #endif
    
    #ifdef LIBCAM
    IplImage * ipl_original = cvCreateImage(cvSize(CAMERA_COLS, CAMERA_ROWS), 8, 3);
    Camera cam(CAMERA_DEVICE, CAMERA_COLS, CAMERA_ROWS, CAMERA_FPS);// width=CAMERA_COLS , height=CAMERA_ROWS, fps=CAMERA_FPS
    #endif
    
    #ifdef DEBUG_ORIGINAL
    namedWindow("original");
    #endif
    
    #ifdef DEBUG_HSV
    namedWindow("hsv");
    #endif
    
    #ifdef CLASSIC_OPENCV
    IplImage * ipl_original;
    #endif
    
    Mat m_cameraFrame;
    Mat m_camFrameCopy;
    Mat m_original_frame;
    
    #ifdef SPLIT_HSV_PLANE
    Mat m_h, m_s, m_v;
    vector<Mat> channels; //store H,S and V channels when splitted
    
    #ifdef DEBUG_SPLIT_HSV
    namedWindow("h");
    namedWindow("s");
    namedWindow("v");
    #endif
    
    #endif
    
    
    #ifdef EQ_HSV_HIST
    
    #ifdef DEBUG_HSV_HIST
    Mat m_hist_dbg = m_camFrameCopy.clone(); //stores the equalized HSV frame
    namedWindow("debug_hsv_histogram");
    #endif
    
    #endif
    
    //manual threshold variables
    #ifdef MANUAL_THRESHOLD
    Mat m_h1, m_h2, m_s1, m_s2, m_v1, m_v2;
    Mat m_hsv_thr;
    
    /* Variables for trackbar */
    int h1 = 0, h2 = 0, s1 = 0, s2 = 0, v1 = 0, v2 = 0, m_open = 0, m_close = 0;
    #ifdef MANUAL_THRESHOLD_CFG
    namedWindow("manual_thr");
    //Creating the trackbars (debug interface)
    cv::createTrackbar("H", "manual_thr", &h1, 180, 0);
    cv::createTrackbar("H2", "manual_thr", &h2, 180, 0);
    cv::createTrackbar("S", "manual_thr", &s1, 255, 0);
    cv::createTrackbar("S2", "manual_thr", &s2, 255, 0);
    cv::createTrackbar("V", "manual_thr", &v1, 255, 0);
    cv::createTrackbar("V2", "manual_thr", &v2, 255, 0);
    cv::createTrackbar("Open", "manual_thr", &m_open, 30, 0);
    cv::createTrackbar("Close", "manual_thr", &m_close, 30, 0);
    //initializing with some defaults
    cv::setTrackbarPos("H", "manual_thr", h1 + 39);
    cv::setTrackbarPos("H2", "manual_thr", h2 + 58);
    cv::setTrackbarPos("S", "manual_thr", s1 + 100);
    cv::setTrackbarPos("S2", "manual_thr", s2 + 200);
    cv::setTrackbarPos("V", "manual_thr", v1 + 60);
    cv::setTrackbarPos("V2", "manual_thr", v2 + 255);
    cv::setTrackbarPos("Open", "manual_thr", m_open + 3);
    cv::setTrackbarPos("Close", "manual_thr", m_close + 7);
    #else    /* os valores comentados estavam setados para testes indoor*/
        h1 = 39; //default: 39 //32 na digitro
    h2 = 58;  //default: 58
    s1 = 100; //default: 100
    s2 = 255; //default:255
    v1 = 60;    //default: 60
    v2 = 255;   //default: 255
    m_open = 3; //default: 3
    m_close = 7; //default: 7
    #endif
    
    #ifdef DEBUG_MANUAL_THRESHOLD
    namedWindow("h1_filtrado");
    namedWindow("h2_filtrado");
    namedWindow("s1_filtrado");
    namedWindow("s2_filtrado");
    namedWindow("v1_filtrado");
    namedWindow("v2_filtrado");
    namedWindow("hsv_thr");
    #endif
    
    #endif //MANUAL_THRESHOLD
        
    #ifdef MORPH_TRANSFORM
    IplImage ipl_temp;
    CvArr* _mask;
    IplConvKernel *element1 = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_ELLIPSE, 0);
    IplConvKernel *element2 = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, 0);
    Mat m_ret;
    
    #ifdef DEBUG_MORPH
    namedWindow("open");
    namedWindow("close");
    namedWindow("gaussian");
    #endif
    
    #endif
    
    #ifdef DRAW_CONTOUR
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Scalar color(255, 255, 255); //RGB color to fulfill the found largest contour
    int first_largest_area = 0;
    int first_largest_contour_index = 0;
    Rect bounding_rect;
    Point center;
    Mat m_result_contour = m_original_frame.clone();
    
    #ifdef DEBUG_CONTOUR
    namedWindow("result_contour");
    #endif
    
    #endif
    
    #ifdef NEW_CONTOUR_METHOD
    vector<vector<Point> > n_contours;
    vector<Vec4i> n_hierarchy;
    Scalar n_color(255, 255, 255); //RGB color to fulfill the found largest contour
    Rect n_bounding_rect;
    Point n_center;
    Mat m_result_newContour = m_original_frame.clone();
    vector<Point> approx;
    double area;
    int radius;
    int second_largest_area = 0;
    int second_largest_contour_index = 0;
    
    #ifdef DEBUG_NEW_CONTOUR
    namedWindow("result_new_contour");
    #endif
    
    #endif
    
    #ifdef SCREEN_DEBUG
    char text[255];
    double font_scale = 0.5;
    Scalar font_color = cvScalar(255, 0, 0);
    #endif
    
    #ifdef HOUGH_TRANSFORM
    Mat m_tmp(m_camFrameCopy.size(), CV_8U);
    Mat m_result_hough = m_original_frame.clone();
    vector<Vec3f> circles;
    int biggest_radius = 0;
    int biggest_radius_index = 0;
    Point center2;
    
    //Creating the trackbars
    int param1 = 0;
    int param2 = 0;
    int min_rad = 0;
    int max_rad = 0;
    int inDP = 0;
    int minDist = 0;
    namedWindow("hough");
    cv::createTrackbar("param1", "hough", &param1, 1000, 0);
    cv::createTrackbar("param2", "hough", &param2, 1000, 0);
    cv::createTrackbar("min_rad", "hough", &min_rad, 1000, 0);
    cv::createTrackbar("max_rad", "hough", &max_rad, 1000, 0);
    cv::createTrackbar("inDP", "hough", &inDP, 20, 0);
    cv::createTrackbar("minDist", "hough", &minDist, 1000, 0);
    //initial setup
    cv::setTrackbarPos("inDP", "hough", inDP + 9);
    cv::setTrackbarPos("minDist", "hough", minDist + 500);
    cv::setTrackbarPos("param1", "hough", param1 + 450);
    cv::setTrackbarPos("param2", "hough", param2 + 270);
    cv::setTrackbarPos("max_rad", "hough", max_rad + 180);
    
    #ifdef DEBUG_HOUGH
    namedWindow("result_hough");
    #endif
    
    #endif
    
    /*matrix to store CLAHE result*/
    Mat eq_clahe_result;
    
    //camera coordinates
    int coord_x = 0;
    int coord_y = 0;
    
    //variable to count null frames
    long null_counter = 0;
    
    //servo control variables
    char servo_atHome = 0;
    char servo_halfHome = 0;
    char servo_halfHome_counter = 0;
    char _servo_halfHome_counter = 0;
    
    //circle verification
    char this_is_a_circle = 0;
    (void) this_is_a_circle;
    /*end OpenCV variables declaration*/
    /* ----------------- end opencv variables --------------------------------*/
    
    //servoCamera_goHome(); //talvez nao precise
    
    /* INIT TASK taskOpenCV*/
    while (taskOpenCV_exit == 0) {        
        /*
        waiting...
        */
        pthread_mutex_lock(&taskOpenCV_mtx);
        
        DBG(DBG_TRACE, "taskOpenCV stopped, waiting...");
        
        pthread_cond_wait(&taskOpenCV_cond, &taskOpenCV_mtx);
        pthread_mutex_unlock(&taskOpenCV_mtx);
        
        if (taskOpenCV_exit == 1) break;
        
        taskOpenCV_running = 1;
        
        DBG(DBG_TRACE, "taskOpenCV fazendo algo...");
        while (taskOpenCV_running == 1) {
            //printf("entrou task OpenCV running = 1\n");
            
            /* opencv code */       
            #ifdef SERVO_ON
            
            /* rotina de varredura por novas bolinhas pelo servo motor */
            if (null_counter == 10) {
                DBG(DBG_TRACE, "SERVO: So much nulls, going home...");
                if (servo_atHome == 0) {
                    if (servo_halfHome == 0) {
                        servoCamera_halfHome();
                        servo_halfHome = 1;
                        servoTiltPosition = 30;
                    }
                    /*se nao esta buscando as bolinhas, incrementa aqui, senao incrementa somente na rotina de varredura*/
                    if (servo_halfHome == 1 && searching_balls <= 3) {
                        servo_halfHome_counter++;
                    }
                    if (servo_halfHome_counter == 5 && searching_balls <=3) { /* 5x10 = 50 frames*/
                        servoCamera_goHome();
                        servoTiltPosition = 60;
                        servo_atHome = 1;
                        servo_halfHome = 0;
                        servo_halfHome_counter = 0;
                    }
                }
                
                searching_balls++;
                null_counter = 0;
            }
            
            /*searching_balls = 0 means that robot is seeing a ball*/
            
            if (searching_balls > 3) {
                if(servo_halfHome == 1) {
                    _servo_halfHome_counter++;
                }
                if (_servo_halfHome_counter == 50) { /* 6x5 = 30 frames*/
                    servoCamera_goHome();
                    servoTiltPosition = 60;
                    servo_atHome = 1;
                    servo_halfHome = 0;
                    _servo_halfHome_counter = 0;
                }
                
                autonomous_right((char *)"4"); //99
            }
            
            if (searching_balls >= 900) {
                move_stop();
                searching_balls = 0;
            }
            
            #endif
            
            
            #ifdef CLASSIC_OPENCV
            //ipl_original = cvQueryFrame(capture);
            // The first several frames tend to come out black.
            for (int i = 0; i < 20; ++i) {
                ipl_original = cvQueryFrame(capture);
                usleep(1000);
            }
            m_cameraFrame = cvarrToMat(ipl_original);
            #endif
            
            #ifdef OCVCAPTURE_MJPEG
            capture.grab();
            capture.rgb(m_cameraFrame);
            #endif
            
            #ifdef OCVCAPTURE_YUYV
            capture.grab();
            capture.rgb(m_cameraFrame);
            #endif
            
            #ifdef LIBCAM
            while(cam.Get()==0) usleep(10); // get the image
            cam.toIplImage(ipl_original);// translate the image to IplImage
            m_cameraFrame = cvarrToMat(ipl_original);
            #endif
            
            m_original_frame = m_cameraFrame.clone();
            //imwrite( "Image.jpg", m_cameraFrame );
            //usleep(1000);


            #ifdef ORIGINAL_HISTOGRAM
            showHistogram_rgb(m_original_frame, "original_red", "original_green", "original_blue");
            #endif
            
            #ifdef DEBUG_ORIGINAL
            if (!m_original_frame.empty())
                imshow("original", m_original_frame);
            else
                printf("m_original_frame vazio...\n");
            
            #endif
            m_camFrameCopy = m_cameraFrame.clone(); //"saving" the original frame
            if (m_camFrameCopy.empty()) {
                DBG(DBG_TRACE, "frame quebrado...");
                printf("frame quebrado...\n");
                break;
            }
            GaussianBlur(m_camFrameCopy, m_camFrameCopy, cvSize(3, 3), 1.0); //need more tests!
            cvtColor(m_camFrameCopy, m_camFrameCopy, CV_BGR2HSV_FULL);
            
            #ifdef DEBUG_HSV
            if (!m_camFrameCopy.empty())
                imshow("hsv", m_camFrameCopy);
            #endif
            
            #ifdef SPLIT_HSV_PLANE
            split(m_camFrameCopy, channels);
            m_h = channels[0];
            m_s = channels[1];
            m_v = channels[2];
            #ifdef DEBUG_SPLIT_HSV
            
            if (!m_h.empty()) imshow("h", m_h);
            else printf("m_h vazio...\n");
                
            if (!m_s.empty()) imshow("s", m_s);
            else printf("m_s vazio...\n");
                
            if (!m_v.empty()) imshow("v", m_v);
            else printf("m_v vazio...\n");
                #endif
            
            #endif
            
            
            #ifdef EQ_HSV_HIST
            
            #ifdef MANUAL_THRESHOLD
            equalizeHist(channels[2], channels[2]);
            m_v = channels[2];
            merge(channels, m_camFrameCopy);
            #endif
            #ifdef DEBUG_HSV_HIST
            cvtColor(m_camFrameCopy, m_hist_dbg, CV_HSV2BGR_FULL);
            if (!m_hist_dbg.empty()) imshow("debug_hsv_histogram", m_hist_dbg);
            showHistogram_rgb(m_hist_dbg, "hsv_eq_hist_red", "hsv_eq_hist_green", "hsv_eq_hist_blue");
            #endif
            
            #endif //EQ_HSV_HIST
                
            #ifdef EQ_CLAHE
            Ptr<CLAHE> clahe = createCLAHE();
            clahe->setClipLimit(2);
            clahe->apply(channels[2], channels[2]);
            m_v = channels[2];
            clahe->apply(channels[1], channels[1]); //TODO: testar
            m_s = channels[1]; //TODO: testar
            merge(channels, m_camFrameCopy);
            cvtColor(m_camFrameCopy, eq_clahe_result, CV_HSV2BGR_FULL);
            #ifdef DEBUG_CLAHE_HIST
            showHistogram_rgb(eq_clahe_result, (char *)"clahe_hist_red", (char *)"clahe_hist_green", (char *)"clahe_hist_blue");
            #endif
            
            #ifdef  DEBUG_EQ_CLAHE
            imshow("clahe_result", eq_clahe_result);
            #endif
            
            #endif //EQ_CLAHE
                
            #ifdef MANUAL_THRESHOLD
            threshold(m_h, m_h1, h1, UCHAR_MAX, CV_THRESH_BINARY);
            threshold(m_h, m_h2, h2, UCHAR_MAX, CV_THRESH_BINARY_INV);
            threshold(m_s, m_s1, s1, UCHAR_MAX, CV_THRESH_BINARY);
            threshold(m_s, m_s2, s2, UCHAR_MAX, CV_THRESH_BINARY_INV);
            threshold(m_v, m_v1, v1, UCHAR_MAX, CV_THRESH_BINARY);
            threshold(m_v, m_v2, v2, UCHAR_MAX, CV_THRESH_BINARY_INV);
            
            #ifdef DEBUG_MANUAL_THRESHOLD
            if (!m_h1.empty())
                imshow("h1_filtrado", m_h1);
            if (!m_h2.empty())
                imshow("h2_filtrado", m_h2);
            if (!m_s1.empty())
                imshow("s1_filtrado", m_s1);
            if (!m_s2.empty())
                imshow("s2_filtrado", m_s2);
            if (!m_v1.empty())
                imshow("v1_filtrado", m_v1);
            if (!m_v2.empty())
                imshow("v2_filtrado", m_v2);
            #endif
            bitwise_and(m_h1, m_h2, m_h);
            bitwise_and(m_s1, m_s2, m_s);
            bitwise_and(m_v1, m_v2, m_v);
            bitwise_and(m_h, m_s, m_hsv_thr);
            bitwise_and(m_hsv_thr, m_v, m_hsv_thr);
            
            #ifdef DEBUG_MANUAL_THRESHOLD
            if (!m_hsv_thr.empty()) imshow("hsv_thr", m_hsv_thr);
            #endif
            
            #endif //MANUAL_THRESHOLD
                
            #ifdef MORPH_TRANSFORM
            ipl_temp = m_hsv_thr; //morphologic operations work only with IplImage structure
            _mask = &ipl_temp;
            CvMat mstub, *mask = cvGetMat(_mask, &mstub);
            cvMorphologyEx(mask, mask, 0, element2, CV_MOP_OPEN, m_open);
            #ifdef DEBUG_MORPH
            if (mask)
                cvShowImage("open", mask);
            #endif
            cvMorphologyEx(mask, mask, 0, element1, CV_MOP_CLOSE, m_close);
            #ifdef DEBUG_MORPH
            if (mask)
                cvShowImage("close", mask);
            #endif
            cvSmooth(mask, mask, CV_GAUSSIAN, 3, 3, 0, 0);
            #ifdef DEBUG_MORPH
            /// Detect edges using canny
            if (mask)
                cvShowImage("gaussian", mask);
            else
                printf("mask vazio...\n");
            
            #endif
            m_ret = cvarrToMat(mask);
            
            #endif //MORPH_TRANSFORM
                
            coord_x = 0;
            coord_y = 0;
            
            #ifdef DRAW_CONTOUR
            #ifdef EQ_HSV_HIST
            m_result_contour = m_hist_dbg.clone();
            #else
                m_result_contour = eq_clahe_result.clone();
            #endif
            findContours(m_ret, contours, hierarchy, CV_RETR_EXTERNAL,
            CV_CHAIN_APPROX_SIMPLE);
            first_largest_contour_index = 0;
            first_largest_area = 0;
            
            for (size_t i = 0; i < contours.size(); i++) {
                double a = contourArea(contours[i], false);
                if (a > first_largest_area) {
                    first_largest_area = a;
                    first_largest_contour_index = i;
                    bounding_rect = boundingRect(contours[i]);
                }
            }
            /*detection step */
            if (first_largest_area > 250) { //value choosen while in test steps, "empirically choosen"
                searching_balls = 0;
                null_counter = 0;
                
                #ifdef MINIMAL_DEBUG
                printf("Blob area: %d\n", first_largest_area);
                #endif
                
                #ifdef SCREEN_DEBUG
                memset(&text[0], 0, sizeof(text)); //clear text content
                sprintf(text, "Blob area: %d", (int)first_largest_area);
                putText(m_result_contour, text, cvPoint(30, 30),
                CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1, CV_AA);
                #endif
                
                drawContours(m_result_contour, contours, first_largest_contour_index, color,CV_FILLED, 8, hierarchy);
                rectangle(m_result_contour, bounding_rect, Scalar(0, 255, 0), 1, 8, 0);
                
                /* draw the circle center point */
                center = cvPoint(bounding_rect.x + bounding_rect.width / 2, bounding_rect.y + bounding_rect.height / 2);
                circle(m_result_contour, center, 3, CV_RGB(255, 0, 0), -1, 8, 0);
                coord_x = bounding_rect.x + (bounding_rect.width / 2);
                coord_y = bounding_rect.y + (bounding_rect.height / 2);
                
                #ifdef SERVO_ON
                /* tilt servo camera up or down */
                //Find out if the Y component of the contour is below the middle of the screen.
                if (coord_y > ((MID_SCREEN_Y + 40) - ACPT_ERROR)) {
                    if ((servoTiltPosition - stepDOWN) >= 0)
                        servoTiltPosition -= stepDOWN; //If it is below the middle of the screen, update the tilt position variable to lower the tilt servo.
                    else servoTiltPosition = 0;
                    }
                
                //Find out if the Y component of the contour is above the middle of the screen.
                else if (coord_y < ((MID_SCREEN_Y - 40) + ACPT_ERROR)) {
                    if ((servoTiltPosition + stepUP) <= 75)
                        servoTiltPosition += stepUP; //Update the tilt position variable to raise the tilt servo.
                }
                servo_atHome = 0;
                servoCamera_move(servoTiltPosition);
                
                DBG(DBG_TRACE, "sending to servo, value: %d\n",servoTiltPosition);
                #endif
                
                /* move so much left */
                if (coord_x >= 0 && coord_x < (CAMERA_COLS*0.17)) {
                    #ifdef SCREEN_DEBUG
                    memset(&text[0], 0, sizeof(text)); //clear text content
                    sprintf(text, "move more LEFT");
                    if (!m_result_contour.empty()) putText(m_result_contour, text, cvPoint(30, 200),
                        CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1,
                    CV_AA);
                    #endif
                    DBG(DBG_TRACE, "move more left");
                    #ifdef MINIMAL_DEBUG
                    printf("move more left...\n");
                    #endif
                    autonomous_left((char *)"4"); //pcDuinov2 PWM range 0 to 9 - 10% increments
                }
                
                /* move forward */
                if (coord_x >= (CAMERA_COLS*0.17) && coord_x <= (CAMERA_COLS*0.83)) {
                    #ifdef SCREEN_DEBUG
                    memset(&text[0], 0, sizeof(text)); //clear text content
                    sprintf(text, "move FORWARD");
                    if (!m_result_contour.empty()) putText(m_result_contour, text, cvPoint(30, 200),
                        CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1,
                    CV_AA);
                    #endif
                    DBG(DBG_TRACE, "move forward");
                    #ifdef MINIMAL_DEBUG
                    printf("move forward...\n");
                    #endif
                    if (first_largest_area >= 2000) autonomous_forward((char *)"3"); /* when the area start to grow up, the robot moves slowly*/
                    else autonomous_forward((char *)"4");//pcDuinov2 PWM range 0 to 9 - 10% increments
                    }
                
                /* move more right */
                if (coord_x > (CAMERA_COLS*0.83) && coord_x <= (CAMERA_COLS)) {
                    #ifdef SCREEN_DEBUG
                    memset(&text[0], 0, sizeof(text)); //clear text content
                    sprintf(text, "move more RIGHT");
                    if (!m_result_contour.empty()) putText(m_result_contour, text, cvPoint(30, 200),
                        CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1,
                    CV_AA);
                    #endif
                    DBG(DBG_TRACE, "move right");
                    #ifdef MINIMAL_DEBUG
                    printf("move right...\n");
                    #endif
                    autonomous_right((char *)"4");
                }
            } //else detection step
            
            else {
                null_counter++;
                #ifdef SCREEN_DEBUG
                memset(&text[0], 0, sizeof(text)); //clear text content
                sprintf(text, "NOTHING");
                if (!m_result_contour.empty()) putText(m_result_contour, text, cvPoint(30, 200),
                    CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1, CV_AA);
                #endif
                DBG(DBG_TRACE, "nothing here...");
                
                #ifdef MINIMAL_DEBUG
                printf("nothing...\n");
                #endif
                
                move_stop();
            }//else do draw_contours
            
            
            contours.clear();
            hierarchy.clear();
            #endif //DRAW_CONTOUR
                
            #ifdef DEBUG_CONTOUR
            if (!m_result_contour.empty())
                imshow("result_contour", m_result_contour);
            else
                printf("m_result_contour vazio...\n");
            
            #endif
            
            /*method used to find circles based on contour area and trigonometry properties of a circle, i.e number of vertices, convexity, area, etc..*/
            #ifdef NEW_CONTOUR_METHOD
            #ifdef EQ_HSV_HIST
                m_result_newContour = m_hist_dbg.clone();
            #else
                m_result_newContour = eq_clahe_result.clone();
            #endif
            findContours(m_ret, n_contours, n_hierarchy, CV_RETR_EXTERNAL,
            CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image
            second_largest_contour_index = 0;
            second_largest_area = 0;
            for (unsigned int i = 0; i < n_contours.size(); i++) {
                // Approximate contour with accuracy proportional to the contour perimeter
                approxPolyDP(Mat(n_contours[i]), approx, arcLength(Mat(n_contours[i]), true) * 0.02, true);
                // Skip small or non-convex objects
                if (fabs(contourArea(n_contours[i])) < 100 || !isContourConvex(approx)) continue;
                
                area = contourArea(n_contours[i]);
                if (area > second_largest_area) {
                    second_largest_area = area;
                    second_largest_contour_index = i;
                    n_bounding_rect = boundingRect(n_contours[i]);
                    radius = n_bounding_rect.width / 2;
                }
            }
            if (second_largest_area > 250) { /*filtro para evitar artefatos sendo detectados como circulos*/
                /* ambos limiares eram 0.2, mudei para 0.75 para testes*/
                if (abs(1 - ((double) n_bounding_rect.width / n_bounding_rect.height)) <= 0.2 && abs(1 - (second_largest_area / (CV_PI * pow(radius, 2)))) <= 0.2) {
                    this_is_a_circle = 1;
                    
                    #ifdef MINIMAL_DEBUG
                    printf("this is a CIRCLE!\n");
                    #endif
                    #ifdef SCREEN_DEBUG
                    memset(&text[0], 0, sizeof(text)); //clear text content
                    sprintf(text, "CIRCLE!");
                    putText(m_result_newContour, text, cvPoint(30, 30),
                    CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1, CV_AA);
                    #endif
                    drawContours(m_result_newContour, n_contours,
                    second_largest_contour_index, n_color, CV_FILLED, 8,
                    n_hierarchy); // Draw the largest contour using previously stored index.
                    rectangle(m_result_newContour, n_bounding_rect,
                    Scalar(0, 255, 0), 1, 8, 0);
                } else {
                    #ifdef MINIMAL_DEBUG
                    printf("this is a NOT A CIRCLE!\n");
                    #endif
                    #ifdef SCREEN_DEBUG
                    memset(&text[0], 0, sizeof(text)); //clear text content
                    sprintf(text, "NOT CIRCLE!");
                    putText(m_result_newContour, text, cvPoint(30, 30),
                    CV_FONT_HERSHEY_SIMPLEX, font_scale, font_color, 1, CV_AA);
                    #endif
                }
            }
            
            #ifdef DEBUG_NEW_CONTOUR
            if (!m_result_newContour.empty())
                imshow("result_new_contour", m_result_newContour);
            #endif
            
            #endif //NEW_CONTOUR_METHOD
                
            /*more complex check, if the contour found is really a circle and is next to the camera, turn on the collector motor */
            #ifdef NEW_CONTOUR_METHOD
            if (first_largest_area >= 7000 && this_is_a_circle == 1) {
                taskCollector_timer(5);  //turn collector for 5s
            }
            #else
            if (first_largest_area >= 7000) {
                taskCollector_timer(5);
            }
            #endif


            if (!m_result_newContour.empty()) 
                imwrite( "Image.jpg", m_result_newContour );

            
            //if (!m_result_contour.empty()) 
            //    imwrite( "Image.jpg", m_result_contour );    
              
            //if (!m_ret.empty()) 
            //    imwrite( "Image.jpg", m_ret );            

            usleep(1000);    
            
            #if 0
                /*necessario para mostrar as imagens*/
            if (waitKey(30) == 'q')
                break;
            #endif

        } //while working task OpenCV
        
        DBG(DBG_TRACE,"voltando a dormir:task opencv.");
        fflush(stdout);
    }
    
    /*
    desalocar memória
    */
    /****** memory release ******/
    destroyAllWindows();
    #ifdef CLASSIC_OPENCV
    cvReleaseCapture(&capture);
    cvReleaseImage(&ipl_original);
    #endif
    
    #ifdef OCVCAPTURE_YUYV
    capture.close();
    #endif
    
    #ifdef OCVCAPTURE_MJPEG
    capture.close();
    #endif
    
    #ifdef LIBCAM
    cam.StopCam();
    #endif
    
    m_cameraFrame.release();
    m_cameraFrame.deallocate();
    m_camFrameCopy.release();
    m_camFrameCopy.deallocate();
    m_original_frame.release();
    m_original_frame.deallocate();
    
    #ifdef SPLIT_HSV_PLANE
    m_h.release();
    m_h.deallocate();
    m_s.release();
    m_s.deallocate();
    m_v.release();
    m_v.deallocate();
    //dealloc vectors memory
    channels.clear();
    vector<Mat>().swap(channels);
    #endif
    
    #ifdef DEBUG_HSV_HIST
    m_hist_dbg.release();
    #endif
    
    #ifdef EQ_CLAHE
    eq_clahe_result.release();
    eq_clahe_result.deallocate();
    #endif
    
    #ifdef MANUAL_THRESHOLD
    m_h1.release();
    m_h1.deallocate();
    m_h2.release();
    m_h2.deallocate();
    m_s1.release();
    m_s1.deallocate();
    m_s2.release();
    m_s2.deallocate();
    m_v1.release();
    m_v1.deallocate();
    m_v2.release();
    m_v2.deallocate();
    m_hsv_thr.release();
    m_hsv_thr.deallocate();
    #endif
    
    #ifdef MORPH_TRANSFORM
    cvReleaseStructuringElement(&element1);
    cvReleaseStructuringElement(&element2);
    m_ret.release();
    m_ret.deallocate();
    #endif
    
    #ifdef DRAW_CONTOUR
    m_result_contour.release();
    m_result_contour.deallocate();
    //dealloc vectors memory
    contours.clear();
    hierarchy.clear();
    vector<vector<Point> >().swap(contours);
    vector<Vec4i>().swap(hierarchy);
    #endif
    
    #ifdef NEW_CONTOUR_METHOD
    m_result_newContour.release();
    m_result_newContour.deallocate();
    //dealloc vectors memory
    n_contours.clear();
    n_hierarchy.clear();
    vector<vector<Point> >().swap(n_contours);
    vector<Vec4i>().swap(n_hierarchy);
    #endif
    
    #ifdef HOUGH_TRANSFORM
    m_tmp.release();
    m_tmp.deallocate();
    m_result_hough.release();
    m_result_hough.deallocate();
    //dealloc vectors memory
    circles.clear();
    vector<Vec3f>().swap(circles);
    #endif
    
    #endif//if 0 de debug
    
    DBG(DBG_TRACE, "fim da taskOpenCV");
    pthread_exit(NULL);
    return 0;
}


int taskOpenCV_run(void) {
    if (taskOpenCV_running == 1) return 1;
    searching_balls = 0;
    move_stop();
    coletor_off();
    servoCamera_goHome();
    servoTiltPosition = 60;
    taskCollector_stop();
    pthread_mutex_lock(&taskOpenCV_mtx);
    pthread_cond_signal(&taskOpenCV_cond);
    pthread_mutex_unlock(&taskOpenCV_mtx);
    return 0;
}

int taskOpenCV_stop(void) {
    move_stop();
    if (taskOpenCV_running == 0) return 1;
    pthread_mutex_lock(&taskOpenCV_mtx);
    taskOpenCV_running = 0;
    searching_balls = 0;
    coletor_off();
    move_stop();
    servoCamera_goHome();
    servoTiltPosition = 60;
    taskCollector_stop();
    pthread_cond_signal(&taskOpenCV_cond);
    pthread_mutex_unlock(&taskOpenCV_mtx);
    return 0;
}

int taskCollector_run(void) {
    pthread_mutex_lock(&taskCollector_mtx);
    taskCollector_action = COLLECTOR_RUN;
    pthread_cond_signal(&taskCollector_cond);
    pthread_mutex_unlock(&taskCollector_mtx);
    
    return 0;
}

int taskCollector_stop(void) {
    pthread_mutex_lock(&taskCollector_mtx);
    taskCollector_action = COLLECTOR_STOP;
    pthread_cond_signal(&taskCollector_cond);
    pthread_mutex_unlock(&taskCollector_mtx);
    
    return 0;
}

int taskCollector_timer(int t) {
    pthread_mutex_lock(&taskCollector_mtx);
    taskCollector_action = COLLECTOR_TIMER;
    taskCollector_delay = t;
    pthread_cond_signal(&taskCollector_cond);
    pthread_mutex_unlock(&taskCollector_mtx);
    
    return 0;
}

int tasks_start(void) {
    int ret;
    
    DBG(DBG_TRACE, "pthread_create()...");
    ret = pthread_create(&taskOpenCV_thread, NULL, (void * (*)(void *))taskOpenCV, (void *)0);
    ret = pthread_create(&taskCollector_thread, NULL, (void * (*)(void *))taskCollector, (void *)0);
    
    //inits
    return ret;
}

int tasks_stop(void) {
    
    pthread_cancel(taskCollector_thread);
    pthread_join(taskCollector_thread, NULL);
    
    //pthread_cancel(taskOpenCV_thread);
    pthread_mutex_lock(&taskOpenCV_mtx);
    taskOpenCV_exit = 1;
    pthread_cond_signal(&taskOpenCV_cond);
    pthread_mutex_unlock(&taskOpenCV_mtx);
    pthread_join(taskOpenCV_thread, NULL);
    
    DBG(DBG_TRACE, "bye-bye!");
    return 0;
}

