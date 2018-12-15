// This class can be used to capture OpenCV
// images from a Camera using video4linux.
// It has only been tested on one camera so far,
// the MS Lifecam VX-7000.
//
// The VideoCapture component in OpenCV calls on
// the video4linux driver to deliver uncompressed
// RGB images. The driver apparently asks the camera
// for a JPEG image and then decompresses it which
// is so slow a process an ARM-based Beagleboard can't
// keep up leading to error messages and scrambled
// images (in part due to some race conditions in the
// OpenCV code.) This module asks the camera for YCbCr
// (aka YUV) images which avoids the JPEG path.

#ifndef OCVCAPTURE_H
#define OCVCAPTURE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <string>


class OCVCapture
{
public:
	// Instantiate a Capture object.
	// The default size is 320 x 240.
	OCVCapture();

	// The destructor automatically closes the camera.
	virtual ~OCVCapture();

	// You can turn on verbose mode to which will cause the capture
	// object to spew debug messages to cout.
	void setVerbose(bool verboseOn);
	bool verbose() const;

	// When the capture object is closed you can set the size. At the
	// time the capture object is opened the hardware will be queried
	// for a supported size which may differ from the requested size,
	// so do not assume the images you retrieve from the capture object
	// are the requested size.
	void setDesiredSize(uint32_t width, uint32_t height);

	void setFramerate(uint32_t fps);

    void setPixelFormat(char* pixfmt);

	void deviceID(char* id);

	// Before capturing images you must open the capture object.
	// The open call returns true if it was successful.
	// Open to read from the camera.
	bool open();
	// Returns true if the capture object is open.
	bool isOpen() const;

	// After the capture object is opened you can query for the final
	// size that it negotiated with the camera.
	uint32_t width() const;
	uint32_t height() const;
	uint32_t frameRate() const;

	// Close the capture object. This releases the video device and
	// frees up driver-related memory. You can change the desired image
	// size while the object is closed.
	void close();

	// The capturing process is divided into two parts. In the first
	// step you call 'grab' to actually grab the (YUV) image. Then
	// later you call 'gray' or 'rgb' to convert the grabbed image to
	// the desired color space.
	bool grab();
	bool rgb(cv::Mat& rgb);
	bool yuv2rgb(cv::Mat& rgb);
	bool yuv2gray(cv::Mat& gray);
	bool yuv2yuv(cv::Mat& yuv);
	bool mjpeg2rgb(cv::Mat& rgb);

private:
	int retry_ioctl(int request, void* argument);
	bool firstGrabSetup();
	void reportError(const char* error);
	void reportError(const char* error, int64_t value);

	void resizeMat(cv::Mat& mat, int matType);

private:
    char*           m_device_id;

	// What the client wants...
	uint32_t		m_desired_width;
	uint32_t		m_desired_height;
	uint32_t		m_desired_frame_rate;
	uint32_t        m_desired_pix_fmt;
	bool			m_verbose;

	// Internal bookkeeping for the camera
	int 			m_camera_handle;
	bool			m_first_grab;

	// The size we negotitated with the driver.
	uint32_t		m_final_width;
	uint32_t		m_final_height;
	uint32_t		m_final_frame_rate;

	// These are the memory mapped image buffers
	// provided by the camera driver.
	std::vector<void*>	m_mapped_buffer_ptrs;
	std::vector<size_t>	m_mapped_buffer_lens;

	// A copy of the mostly recently grabbed raw data.
	std::vector<uint8_t> dataBuffer;
	size_t			m_raw_image_size;
	uint8_t*		m_raw_image_data;
	uint32_t		m_raw_bytes_per_line;
};

#endif // OCVCAPTURE_H
