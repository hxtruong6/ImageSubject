// 1612899_Lab02.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#define BIN_W 256

using namespace std;
using namespace cv;

void printArr(vector<int> arr, int n = BIN_W)
{
	for (int i = 0; i < n; ++i)
	{
		cout << arr[i] << " ";
	}
	cout << endl;
}

void rbgToYCbCr(const Mat& originImage, Mat & desImage)
{
	// convert RBG image to YCbCr image 
	for (int y = 0; y < originImage.rows; ++y)
	{
		for (int x = 0; x < originImage.cols; ++x)
		{
			float b = originImage.at<Vec3b>(y, x)[0];
			float g = originImage.at<Vec3b>(y, x)[1];
			float r = originImage.at<Vec3b>(y, x)[2];

			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(0.299*r + 0.587*g + 0.114*b);
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(128 - 0.168736*r - 0.331264*g + 0.5*b);
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar> (128 + 0.5*r - 0.418688*g - 0.081312*b);
		}
	}
}

void yCbCrToRBG(const Mat& originImage, Mat& desImage)
{
	// convert YCbCr image to RBG image 
	for (int y = 0; y < originImage.rows; ++y)
	{
		for (int x = 0; x < originImage.cols; ++x)
		{
			float Y = originImage.at<Vec3b>(y, x)[0];
			float Cb = originImage.at<Vec3b>(y, x)[1];
			float Cr = originImage.at<Vec3b>(y, x)[2];

			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(Y + 1.772*(Cb - 128));
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(Y - 0.344136*(Cb - 128) - 0.714136*(Cr - 128));
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(Y + 1.402*(Cr * 128));
		}
	}
}

void rbgToGray(const Mat& originImage, Mat & desImage) {
	// The algorithm linear: Y = B * 0.0722 + G * 0.7152 + R * 0.2126
	// Blue - Green - Red
	for (int y = 0; y < originImage.rows; y++) {
		for (int x = 0; x < originImage.cols; x++) {
			float yLinear = originImage.at<Vec3b>(y, x)[0] * 0.0722 +
				originImage.at<Vec3b>(y, x)[1] * 0.7152 +
				originImage.at<Vec3b>(y, x)[2] * 0.2126;

			if (desImage.channels() == 1)
			{
				desImage.at<uchar>(y, x) = yLinear;
			}
			else
			{
				desImage.at<Vec3b>(y, x)[0] = yLinear;
				desImage.at<Vec3b>(y, x)[1] = yLinear;
				desImage.at<Vec3b>(y, x)[2] = yLinear;
			}
		}
	}
}

void imHist(const Mat& image, vector<int>& histogram)
{
	// calculate the no of pixels for each intensity values
	for (int y = 0; y < image.rows; y++)
		for (int x = 0; x < image.cols; x++)
		{
			if (image.channels() == 0)
			{
				histogram[(int)image.at<uchar>(y, x)]++;
			}
			else
				histogram[(int)image.at<Vec3b>(y, x)[0]]++;
		}
}

void equalizationHistogram(int binW, int size, const vector<int>& histogram, vector<int> & eHistogram)
{
	// equalization a vector with binW and size
	vector<int> sumHistogram(histogram.size(), 0);
	sumHistogram[0] = histogram[0];
	for (int i = 1; i < histogram.size(); ++i)
	{
		sumHistogram[i] = sumHistogram[i - 1] + histogram[i];
	}

	float alpha = round((float)(binW - 1) / (float)size);
	for (int i = 0; i < sumHistogram.size(); ++i)
	{
		eHistogram[i] = saturate_cast<uchar>(alpha * sumHistogram[i]);
	}
}



void equalizationGrayImage(const Mat& originImage, Mat & desImage)
{
	vector<int> histogram(BIN_W, 0);
	imHist(originImage, histogram);

	vector<int> eHistogram(BIN_W, 0);
	equalizationHistogram(BIN_W, originImage.rows*originImage.cols, histogram, eHistogram);

	desImage = originImage.clone();
	for (int y = 0; y < originImage.rows; y++)
		for (int x = 0; x < originImage.cols; x++)
		{
			cout << desImage.at<uchar>(y, x) << " --> ";
			desImage.at<uchar>(y, x) = saturate_cast<uchar>(eHistogram[originImage.at<uchar>(y, x)]);
			cout << desImage.at<uchar>(y, x) << endl;
		}
}

void equalizationColorImage(const Mat& originImage, Mat & desImage)
{
	Mat newImage = originImage.clone();
	rbgToYCbCr(originImage, newImage);

	vector<int> histogram(BIN_W, 0);
	imHist(newImage, histogram);

	vector<int> eHistogram(BIN_W, 0);
	equalizationHistogram(BIN_W, newImage.rows*newImage.cols, histogram, eHistogram);

	desImage = newImage.clone();
	for (int y = 0; y < newImage.rows; y++)
		for (int x = 0; x < newImage.cols; x++)
		{
			cout << desImage.at<uchar>(y, x) << " --> ";
			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(eHistogram[newImage.at<uchar>(y, x)]);
			cout << desImage.at<uchar>(y, x) << endl;
		}

	yCbCrToRBG(newImage, desImage);
}

int main(int argc, char** argv)
{
	const char * fname = "C:\\Users\\Truong Hoang\\Pictures\\dora.jpg";

	//char * fname = argv[1];										// Read file name from command line
	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	imshow("The original image", image);						//	Show image in screen 



	waitKey(0);
	return 0;
}

