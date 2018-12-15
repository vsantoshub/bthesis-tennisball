#include "OCVCapture.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <unistd.h>

using namespace cv;
using namespace std;

int main()
{
    OCVCapture capture;
    capture.deviceID("/dev/video0");
    capture.setDesiredSize(640,480);
    capture.setFramerate(30);
    capture.setPixelFormat("MJPEG");
    capture.setVerbose(true);
    capture.open();

    if (!capture.isOpen())
    {
        cerr << "Failed to open camera" << endl;
        return -1;
    }

    cout << "Capture " << capture.width() << " x " << capture.height();
    cout << " pixels at " << capture.frameRate() << " fps" << endl;
    int i = 0;
    // The first several frames tend to come out black.
    for (i = 0; i < 20; ++i)
    {
        capture.grab();
        usleep(1000);
    }

    //namedWindow("RGB", CV_WINDOW_AUTOSIZE);

#if 0
    Mat rgb;

    for (i = 0; i < 1000; ++i)
    {
        capture.rgb(rgb);
        usleep(1000);
        cout << "." << i << flush;
        if (i % 100 == 0){
            char buf[50];
            sprintf(buf,"test%3d.jpg",i);
            imwrite(buf,rgb);
        }
    }
#endif
#if 1   
    Mat rgb;

    bool done = false;
    bool paused = false;

    i=0;
    //while (!done)
    while (i<1000)
    {
        //if (i==10) break;
        //i++;
        if (!paused)
            done = !capture.grab();

        if (!done)
        {
            capture.rgb(rgb);
            //imshow("RGB", rgb);
#if 0
            if (i % 100 == 0){
                char buf[50];
                sprintf(buf,"test%3d.jpg",i);
                imwrite(buf,rgb);
            }
#endif
            cout << "." << i << flush;
        }

        if (!done)
        {
            int key = waitKey(10);
            switch (key)
            {
                case 27:
                    done = true;
                    break;
                case ' ':
                    paused = !paused;
                    break;
            }
        }
        i++;
    }
    //capture.close();
#endif
    return 0;
}


