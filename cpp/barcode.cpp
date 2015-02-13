#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
 
using namespace cv;
using namespace std;
 
int getUnitWidth(const Mat widthRow)
{
 
	int unitWidth = widthRow.at<int>(0, 1) + widthRow.at<int>(0, 2) + 
					widthRow.at<int>(0, 3) + widthRow.at<int>(0, 28) + 
					widthRow.at<int>(0, 29) + widthRow.at<int>(0, 30) +
					widthRow.at<int>(0, 31) + widthRow.at<int>(0, 32) + 
					widthRow.at<int>(0, 57) + widthRow.at<int>(0, 58) + 
					widthRow.at<int>(0, 59);
	unitWidth = unitWidth / 11;
	return unitWidth;
}
 
int getMatch(int combo[4])
{
	if (combo[0] == 3 && combo[1] == 2 && combo[2] == 1 && combo[3] == 1)
		return 0;
	if (combo[0] == 2 && combo[1] == 2 && combo[2] == 2 && combo[3] == 1)
		return 1;
	if (combo[0] == 2 && combo[1] == 1 && combo[2] == 2 && combo[3] == 2)
		return 2;
	if (combo[0] == 1 && combo[1] == 4 && combo[2] == 1 && combo[3] == 1)
		return 3;
	if (combo[0] == 1 && combo[1] == 1 && combo[2] == 3 && combo[3] == 2)
		return 4;
	if (combo[0] == 1 && combo[1] == 2 && combo[2] == 3 && combo[3] == 1)
		return 5;
	if (combo[0] == 1 && combo[1] == 1 && combo[2] == 1 && combo[3] == 4)
		return 6;
	if (combo[0] == 1 && combo[1] == 3 && combo[2] == 1 && combo[3] == 2)
		return 7;
	if (combo[0] == 1 && combo[1] == 2 && combo[2] == 1 && combo[3] == 3)
		return 8;
	if (combo[0] == 3 && combo[1] == 1 && combo[2] == 1 && combo[3] == 2)
		return 9;
 
	return 10;
 
}
 
int parseDigit(int w1, int w2, int w3, int w4, int unitWidth)
{
	int digit = 10;
	if (w1*w2*w3*w4 == 0)
		return digit;
	if ((w1 + w2 + w3 + w4) < 5 * unitWidth)
		return digit;
	int i1 = w1 / unitWidth;
	int i2 = w2 / unitWidth;
	int i3 = w3 / unitWidth;
	int i4 = w4 / unitWidth;
	while ((i1 + i2 + i3 + i4) < 7)
	{
		i1 *= 2;
		i2 *= 2;
		i3 *= 2;
		i4 *= 2;
	}
	int combo[4] = { i1, i2, i3, i4 };
	if ((i1 + i2 + i3 + i4 > 7)){
		int diff = i1 + i2 + i3 + i4 - 7;
		int subtractLog[4] = { 0, 0, 0, 0 };
		int attempNo = 0;
		while (diff > 0 && attempNo < 20)
		{
			attempNo++;
			if (subtractLog[0] == 1 &&
				subtractLog[1] == 1 &&
				subtractLog[2] == 1 &&
				subtractLog[3] == 1)
			{
				subtractLog[0] = 0;
				subtractLog[1] = 0;
				subtractLog[2] = 0;
				subtractLog[3] = 0;
			}
			// find largest index, then subtract 1;
			// next time find another untouched largest index, then subtract 1;
			// ... until sum equals 7;
			int largestIndex = -1, largestWidth = 1;
			for (int i = 0; i < 4; i++)
			{
				if (combo[i] > largestWidth && subtractLog[i] == 0)
				{
					largestIndex = i;
					largestWidth = combo[i];
				}
			}
			if (largestIndex == -1 || combo[largestIndex] == 1)
				break;
			else
			{
				combo[largestIndex]--;
				subtractLog[largestIndex] = 1;
			}
			diff = combo[0] + combo[1] + combo[2] + combo[3] - 7;
		}
	}
	digit = getMatch(combo);
	return digit;
}
 
void doBarcode(int barcodeID)
{
	Mat img = imread(
		"barcode_" + to_string(barcodeID) + ".jpg",
		CV_LOAD_IMAGE_GRAYSCALE) > 128;
	Mat se = Mat::ones(5,1,CV_8U);
	erode(img, img, se);
	// 1. collect a matrix widthMatrix (61*n) from the valid rows
	Mat widthMatrix = Mat::zeros(img.rows, 61, CV_32S);
	for (int i = 0; i < img.rows; i++)
	{
		Mat barRow = img.row(i);
 
		int widthIndex = 0;
		uchar lastPixel = 255;
		for (int j = 0; j < img.cols; j++)
		{
			uchar currentPixel = img.at<uchar>(i, j);
			if (currentPixel == lastPixel)
			{
				widthMatrix.at<int>(i, widthIndex)++;
			}
			else if (widthIndex < 60)
			{
				widthMatrix.at<int>(i, ++widthIndex)++;
				lastPixel = currentPixel;
			}
		}
	}
	// 2. for each row in widthMatrix, get UNIT_WIDTH, get the width combos of 12 DIGITS, find match
	Mat digitMatrix = Mat::ones(12, widthMatrix.rows, CV_8U) * 10;
	for (int i = 0; i < widthMatrix.rows; i++)
	{
		Mat widthRow = widthMatrix.row(i);
		if (countNonZero(widthRow) != 61)
			continue;
		int unitWidth = getUnitWidth(widthRow);
		int digitIndex = 0;
		for (int j = 4; j < 54; j+=4)
		{
			if (j == 28)
				j = 33;
			int w1 = widthMatrix.at<int>(i, j);
			int w2 = widthMatrix.at<int>(i, j + 1);
			int w3 = widthMatrix.at<int>(i, j + 2);
			int w4 = widthMatrix.at<int>(i, j + 3);
			int digit = parseDigit(w1, w2, w3, w4, unitWidth);
			digitMatrix.at<uchar>(digitIndex++, i) = (uchar)digit;
		}
	}
	widthMatrix.release();
	// 3. pick the most frequently appeared as result digit
	int bestResult[12], index = 0;
	for (int i = 0; i < 12; i++)
	{
		// build a map, then in the end pick the most frequent one
		map<char, int> frequencyMap;
		for (int j = 0; j < digitMatrix.cols; j++)
		{
			char digit = digitMatrix.at<char>(i, j);
			map<char, int>::iterator it = frequencyMap.find(digit);
			if (it != frequencyMap.end())
				it->second++;
			else
				frequencyMap.insert(pair<char, int>(digit, 1));
		}
		// find the most frequent one (beside 10)
		char bestDigit = -1, highestFreq = 0;
		for (map<char, int>::const_iterator it = frequencyMap.begin();
			 it != frequencyMap.end(); ++it)
		{
			uchar digit = it->first;
			if (digit == 10)
				continue;
			int freq = it->second;
			if (freq > highestFreq)
			{
				bestDigit = digit;
				highestFreq = freq;
			}
		}
		bestResult[index++] = bestDigit;
	}
	String print = "";
	for (int i = 0; i < 12; i++)
		if (bestResult[i] != -1)
			print = print + to_string(bestResult[i]);
		else print = print + "*";
	cout << "result: " << print << endl;
	img = imread(
		"barcode_" + to_string(barcodeID) + ".jpg", 
		CV_LOAD_IMAGE_COLOR);
	double imgWidth = img.rows;
	double fontScale = imgWidth / 210;
	putText(img, 
			print, 
			cvPoint(img.rows/8, img.cols * 3 / 10), 
			CV_FONT_HERSHEY_SIMPLEX, 
			fontScale, 
			Scalar(0, 0, 255), 
			2);
	imwrite("barcode_result_" + to_string(barcodeID) + ".png", img);
}
 
int main()
{
	for (int i = 1; i < 6; i++)
		doBarcode(i);
	return 0;
}
