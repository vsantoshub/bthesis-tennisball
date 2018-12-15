//
// camera.cpp - code for demonstrating the use of the OCVCapture
// class
// 
// Written in 2012 by Martin Fox
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to
// the public domain worldwide. This software is distributed without
// any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.

#include "OCVCapture.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	OCVCapture capture;
	
	capture.setDesiredSize(320, 240);
	capture.setVerbose(true);
	capture.open();

	if (!capture.isOpen())
	{
		cerr << "Failed to open camera" << endl;
		return -1;
	}
	
	cout << "Capture " << capture.width() << " x " << capture.height();
	cout << " pixels at " << capture.frameRate() << " fps" << endl;

	// The first several frames tend to come out black.
	for (int i = 0; i < 20; ++i)
	{
		capture.grab();
		usleep(1000);
	}
	
	namedWindow("Gray", CV_WINDOW_AUTOSIZE);
	namedWindow("RGB", CV_WINDOW_AUTOSIZE);
	
	Mat gray, rgb;
	
	bool done = false;
	bool paused = false;

	while (!done)
	{
		if (!paused)
			done = !capture.grab();
			
		if (!done)
		{
			capture.gray(gray);
			imshow("Gray", gray);
			capture.rgb(rgb);
			imshow("RGB", rgb);
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
	}

	capture.close();

	return 0;
}

