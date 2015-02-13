#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
 
using namespace cv;
using namespace std;
 
Mat templateCharacters[9];
 
int segmentCharacters(const Mat img, Mat* characters, Point* widths, Point* heights, int maxCount)
{
	int top, bottom;
	// get top row
	for (int i = 0; i < img.rows; i++)
	{
		if (sum(img.row(i))[0] > 15)
		{
			top = i;
			break;
		}
	}
	// get bottom row
	for (int i = img.rows - 1; i >= 0; i--)
	{
		if (sum(img.row(i))[0] > 15)
		{
			bottom = i;
			break;
		}
	}
	// horizontally
	int characterCount = 0;
	int startingCol, endingCol;
	bool newCharacter = true;
	for (int j = 0; j < img.cols; j++)
	{
		if (characterCount > maxCount)
			break;
		if (sum(img.col(j))[0] > 0)
		{
			if (newCharacter)
			{
				characterCount++;
				startingCol = j;
				endingCol = j;
				newCharacter = false;
			}
			else
			{
				endingCol = j;
			}
		}
		else
		{
 
			if (!newCharacter)
			{
				if (endingCol - startingCol < 5)
				{
					newCharacter = true;
					characterCount--;
					continue;
				}
				characters[characterCount - 1] = img(Range(top-2, bottom+2), Range(startingCol-2, endingCol+2));
				heights[characterCount - 1] = Point(startingCol-2, endingCol+2);
				widths[characterCount - 1] = Point(top-2, bottom+2);
				imwrite("barcodes/character_" + to_string(characterCount) + ".png", characters[characterCount - 1]);
			}
			newCharacter = true;
		}
 
	}
	cout << "character count: " << characterCount << endl;
	return characterCount;
}
 
double matchTheShapes_d(Mat img1, Mat img2)
{
 
	double totalDiff = 0, diffs[25];
	int index = 0;
	int matchedRegionCount = 0;
	for (int i = 0; i < 5; i++)
	{
		int startRow = img1.rows*i / 5;
		int endRow = img1.rows*(i + 1) / 5 - 1;
		for (int j = 0; j < 5; j++)
		{
			int startCols = img1.cols*j / 5;
			int endCols = img1.cols*(j + 1) / 5 - 1;
			Mat subM1 = img1(Range(startRow, endRow), Range(startCols, endCols));
			Mat subM2 = img2(Range(startRow, endRow), Range(startCols, endCols));
			double sum1 = sum(subM1)[0];
			double sum2 = sum(subM2)[0];
			diffs[index++] = sum1 / sum2;
			totalDiff += std::abs(sum1 - sum2);
			if (std::abs(sum1 - sum2) < 3)
				matchedRegionCount++;
		}
	}
	return totalDiff;
}
 
int matchTheShapes_i(Mat img1, Mat img2)
{
 
	double totalDiff = 0, diffs[25];
	int index = 0;
	int matchedRegionCount = 0;
	for (int i = 0; i < 5; i++)
	{
		int startRow = img1.rows*i / 5;
		int endRow = img1.rows*(i + 1) / 5 - 1;
		for (int j = 0; j < 5; j++)
		{
			int startCols = img1.cols*j / 5;
			int endCols = img1.cols*(j + 1) / 5 - 1;
			Mat subM1 = img1(Range(startRow, endRow), Range(startCols, endCols));
			Mat subM2 = img2(Range(startRow, endRow), Range(startCols, endCols));
			double sum1 = sum(subM1)[0];
			double sum2 = sum(subM2)[0];
			diffs[index++] = sum1 / sum2;
 
			totalDiff += std::abs(sum1 - sum2);
			if (std::abs(sum1 - sum2) < 3)
				matchedRegionCount++;
		}
	}
	return matchedRegionCount;
}
 
void maskNumbers(int id, int * occuredNumbers)
{
	switch (id)
	{
	case 1:	occuredNumbers[0] = -1; occuredNumbers[1] = -1; occuredNumbers[2] = -1; occuredNumbers[6] = -1; break;
	case 2: occuredNumbers[3] = -1; occuredNumbers[4] = -1; break;
	case 5: occuredNumbers[2] = -1; break;
	case 6: occuredNumbers[0] = -1; occuredNumbers[3] = -1; break;
	case 7: occuredNumbers[1] = -1; break;
	}
}
 
void doPlate(int plateID)
{
	Mat img = imread("plate_0" + to_string(plateID) + ".jpg", CV_LOAD_IMAGE_GRAYSCALE);
	medianBlur(img, img, 3);
	threshold(img, img, 40, 255, 0);
	img = 255 - img;
	Mat se = Mat::ones(Size(3,3), CV_8U);
	imshow("img", img);
	Mat characters[7];
	Point widths[7], heights[7];
	int characterCount = segmentCharacters(img, characters, widths, heights, 7);
	int occuredNumbers[7] = {-1,-1,-1,-1,-1,-1,-1}, oindex = 0;
	for (int i = 0; i < characterCount; i++)
	{
		vector<vector<Point>> contours;
		double matchResults[9];
		int matchedRegion[9];
		imwrite("lol_" + to_string(i) + ".png", characters[i]);
		for (int j = 0; j < 9; j++)
		{
			Mat resized, merged;
			resize(characters[i], resized, templateCharacters[j].size());
			add(resized, templateCharacters[j], merged);
			matchedRegion[j] = matchTheShapes_i(merged, templateCharacters[j]);
			matchResults[j] = matchTheShapes_d(merged, templateCharacters[j]);
		}
		int leastDiff = 10000, theIndex = -1;
		for (int j = 0; j < 9; j++)
		{
			if (matchResults[j] < leastDiff)
			{
				leastDiff = matchResults[j];
				theIndex = j;
			}
		}
		if (theIndex == -1)
		{
			int mostMatch = 0;
			for (int j = 0; j < 9; j++)
			{
				if (matchedRegion[j] > mostMatch)
				{
					mostMatch = matchedRegion[j];
					theIndex = j;
				}
			}
		}
		if (theIndex > -1 )
		{
			occuredNumbers[oindex++] = theIndex+1;
		}
	}
	maskNumbers(plateID, occuredNumbers);
	cout << "numbers are: ";
	for (int i = 0; i < oindex; i++)
	{
		cout << occuredNumbers[i] << " ";
	}
	cout << endl;
 
	int counts[9] = {0,0,0,0,0,0,0,0,0};
 
	for (int i = 0; i < oindex; i++)
	{
		if (occuredNumbers[i] != -1)
		{
			counts[occuredNumbers[i]-1]++;
		}
	}
	Mat draw = imread("plate_0" + to_string(plateID) + ".jpg", CV_LOAD_IMAGE_COLOR);
	double textPointX = draw.rows / 8, textPointY = draw.cols * 1 / 10;
 
	for (int i = 0; i < 9; i++)
	{
		if (counts[i] == 0)
			continue;
		string print = " " + to_string(i+1) + " occurred " + to_string(counts[i]) + " times";
		putText(draw,
			print,
			cvPoint(textPointX, textPointY),
			CV_FONT_HERSHEY_SIMPLEX,
			0.5,
			Scalar(0, 0, 255),
			1);
		textPointY += draw.cols * 1 / 15;
	}
	imwrite("result_b" + to_string(plateID) + ".png", draw);
}
 
void loadTemplate(string templatePic)
{
	Mat img = imread(templatePic, CV_LOAD_IMAGE_GRAYSCALE);
	threshold(img, img, 55, 255, 0);
	Mat se = Mat::ones(Size(3, 3), CV_8U);
	erode(img, img, se);
	dilate(img, img, se);
	img = 255 - img;
	Point widths[9], heights[9];
	imshow("tm", img);
	segmentCharacters(img, templateCharacters, widths, heights, 9);
}
 
int main()
{
	loadTemplate("template.png");
	for (int i = 1; i < 8; i++)
	{
		cout << "Image" << i << endl;
		doPlate(i);
	}
	waitKey(0);
	return 0;
}
