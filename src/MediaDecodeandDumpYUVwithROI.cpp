//Test File which takes input video file, ROI as argumets
//and dumps the YUV output file highlighting ROI Part.

//Written by Maheshwar Reddy K
//Date:03/016/2017
//mail@ mahesh.kadapa@gmail.com

//This file use open source Opencv libraries.

#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <sstream> 

using namespace cv;

#define ENABLE_DISPLAY 0

//Function prototype of the function,
//which does filtering on non roi partof the image.
int median_blur_nonroi(unsigned char *inbuf,
	unsigned int img_width,
	unsigned int img_height,
	unsigned int x_pos,
	unsigned int y_pos,
	unsigned int roi_w,
	unsigned int roi_h,
	unsigned char *outbuf);

int main(int argc, char*argv[])
{
	//To store the input filenames
	char inputfilename[256];
	char outputfilename[256];

	// Default input parameters.
	unsigned int x = 100;
	unsigned int y = 100;
	unsigned int roi_w = 300;
	unsigned int roi_h = 300;

	if (argc < 7)
	{
		printf("check Usage: <.exe> <inputfile> <xpos> <ypos> <roi_width> <roi_height> <outputfile> \n");
		printf("sample for reference::  small.mp4 0 0 100 100 out.yuv \n");
		return 0;
	}

	//one more check for vlaidity
	if (argv[1] != NULL && argv[6] != NULL)
	{
		strcpy_s(inputfilename, argv[1]);
		strcpy_s(outputfilename, argv[6]);
	}

	//Read the ROI Parameters, Start point and width and height.
	x = atoi(argv[2]);
	y = atoi(argv[3]);

	roi_w = atoi(argv[4]);
	roi_h = atoi(argv[5]);
	
	printf("input arguments: %s, %d, %d, %d, %d, %s\n", inputfilename, x, y, roi_w, roi_h, outputfilename);

	//FILE *infileptr = fopen(inputfilename, "r+b");
	FILE *outfileptr = fopen(outputfilename, "w+b");

	//FILE *outfileptr1 = fopen("Finalout.yuv", "w+b");
#if ENABLE_DISPLAY
	//Show the input Video
	cvNamedWindow(inputfilename, CV_WINDOW_AUTOSIZE);
	//show the Output video
	cvNamedWindow(outputfilename, CV_WINDOW_AUTOSIZE);
#endif


	//Open the media file.
	CvCapture* capture = cvCreateFileCapture(inputfilename);

	if (capture == NULL)  // check if we succeeded
	{
		printf("Input file not found \n");
		return -1;
	}


	//Hold Frame data
	Mat frame;
	Mat frame_yuv;
	Mat frame_yuvfull;

	frame = cvQueryFrame(capture);

	if (frame.empty())
	{
		printf("File does not conatian any valid frame \n");
		return 0;
	}
	
	//Read width and height
	int img_width = frame.cols;
	int img_height = frame.rows;
	
	int rows = frame_yuv.rows;
	int clms = frame_yuv.cols;

	//Frame size based on yuv type.
	int size = img_width * img_height * 3;
	int size_yuv = size / 2;
	int size_y = img_width * img_height;

	//allocate an output buffer for processing.
	unsigned char *outdataptr = (unsigned char *)calloc(1, size);

	unsigned char *temp123 = outdataptr;

	printf("Input Video Resolution WxH = %d x %d\n", img_width, img_height);


	printf("Processing Started, Please Wait OR Close the terminal to exit\n");

	int cnt = 0;
	//Lopp through every video frame untill end or "q" is pressed.
	while (1) {

		++cnt;
		printf("Processing Frame num : %d\n", cnt);
		//Query and  get a video frame.
		//The data in this Mat is of BGR 888.
		frame = cvQueryFrame(capture);

		//Exit at end of file
		if (frame.empty())			
			break;

		//Convert BGR 888 to yuv 420 for processing
		frame_yuvfull= frame.clone();;
		cv::cvtColor(frame, frame_yuvfull, CV_BGR2YUV_I420);

		//Below 2 lines will dump the yuv before processing, 
		//if these lines are enabled, adjacent frames can be comapred
		// as with and without processing.

		//temp123 = (unsigned char *)frame_yuvfull.data;
		//fwrite((char*)temp123, 1, size_yuv, outfileptr);

		//Get a Copy of image
		frame_yuv = frame_yuvfull.clone();

		//Uncomment this if we want non ROI data set to green frame.
		//Note that its a gree frame not the black frame.
		//memset((unsigned char*)frame_yuv.data, 0, size_y);

		unsigned char *dataptr = (unsigned char*)frame.data;
		unsigned char*out1dataptr = (unsigned char *)frame_yuv.data;

		//Main function which blurs the non ROI region using simple median filter
		//Inside the fucntion kernel size is hard coded , if this value is increased,
		//it blurs more.
		median_blur_nonroi(dataptr, img_width, img_height, x, y, roi_w, roi_h, out1dataptr);

		fwrite((unsigned char*)out1dataptr, 1, size_yuv, outfileptr);

     	/* if ESC is pressed then exit loop */
		char c = cvWaitKey(30);

		if (c == 27 || c == 'q') break;
	}

	
	printf("successfully completed\n");

	//Release the Resources
	cvReleaseCapture(&capture);

#if ENABLE_DISPLAY
	cvDestroyWindow("Inputvideo");
	cvDestroyWindow("OutPutvideo");
#endif

	fclose(outfileptr);
	free(outdataptr);

	return EXIT_SUCCESS;
}

#define CLIP(val, min, max)  (val < min ? min: (val > max ? max:val))

//Do it in RGB
int median_blur_nonroi (unsigned char *inbuf,
						unsigned int img_width,
					    unsigned int img_height,
						unsigned int x_pos,
						unsigned int y_pos,
					    unsigned int roi_w,
						unsigned int roi_h,
						unsigned char *outbuf)
{

	int kernel = 9;


	unsigned int x_index = 0;
	unsigned int y_index = 0;

	double temp = 0.0;

	int rowidx1 = 0;
	int rowidx2 = 0;
	int rowidx3 = 0;
	int rowidx4 = 0;
	int rowidx5 = 0;
	int rowidx6 = 0;

	int num_pixels;

	unsigned char *tempptr = NULL;
	unsigned char *outtempbuf = NULL;

	//control the height
	for (y_index = 0; y_index < img_height; ++y_index)
	{

		//control the width
			for (x_index = 0; x_index < img_width; ++x_index)
			{

				if ((x_index >= x_pos) && (x_index <= (x_pos + roi_w)) && (y_index >= y_pos) && (y_index <= (y_pos + roi_h)))
				{
					//Do nothing
					//Already input data is copied to outbut 
					//So nothing to di here
					// we can add a sharpening filter.
				}
				else
				{
					num_pixels = kernel * kernel;

					//Variabel length Kernel
					//Bit high in CPU usage,
					//Can be Optimized further based on time.
					for (int k = -kernel; k < kernel; ++k)
					{
						int y_k = y_index + k;

						y_k = CLIP(y_k, 0, img_height);

						tempptr = inbuf + (y_k * img_width);

						for (int m = -kernel; m < kernel; ++m)
						{
							int x_k = x_index + m;

							x_k = CLIP(x_k, 0, img_width);

							//tempptr = inbuf + (y_k * im) + ;
							temp += tempptr[x_k];
						}

					}

					outtempbuf = outbuf + (y_index * img_width) + x_index;

					//Average(Normalization)
					temp = temp / (kernel * kernel);

					//Safety to check pixel values are within range.
					temp = CLIP(temp, 0, 255);

					*outtempbuf =(unsigned char) temp;

					//Reset for next calculation
					temp = 0;
				}
			}
	}


	return 0;
}