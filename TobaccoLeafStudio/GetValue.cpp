#include"Header.h"


void RotateLeaf(Mat &r, Mat &t){
	Point Left, Right; bool flag = 0;
	for (int j = 0; j < r.cols; j++){
		if (flag) break;
		for (int i = 0; i < r.rows; i++){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				Left.x = j;
				Left.y = i;
				flag = 1;
				break;
			}
		}
	}
	flag = 0;
	for (int j = r.cols - 1; j >= 0; j--){
		if (flag) break;
		for (int i = r.rows - 1; i >= 0; i--){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				Right.x = j;
				Right.y = i;
				flag = 1;
				break;
			}
		}
	}

	double s = (double)(Right.y - Left.y) / (double)(Right.x - Left.x);
	double angle = atan(s) * 180 / CV_PI;
	Mat temp = getRotationMatrix2D(Left, angle, 1);
	Mat dst;
	warpAffine(r, r, temp, r.size(), INTER_NEAREST, 0, Scalar(255, 255, 255));
	warpAffine(t, t, temp, r.size(), INTER_NEAREST, 0, Scalar(0, 0, 0));
	//Æ½ÒÆ
	Point2f middle = Point2f((Left.x + Right.x) / 2, (Left.y + Right.y) / 2);
	Point2f srcTriangle[3];
	Point2f dstTriangle[3];
	srcTriangle[0] = middle;
	dstTriangle[0] = Point2f(408, 308);
	srcTriangle[1] = Point2f(0, 0);
	dstTriangle[1] = Point2f(408 - middle.x, 308 - middle.y);
	srcTriangle[2] = Point2f(100, 100);
	dstTriangle[2] = Point2f(100 +408- middle.x, 100 + 308 - middle.y);
	Mat	warpMat = getAffineTransform(srcTriangle, dstTriangle);
	warpAffine(r, r, warpMat, r.size(), INTER_NEAREST, 0, Scalar(255, 255, 255));
	warpAffine(t, t, warpMat, r.size(), INTER_NEAREST, 0, Scalar(0, 0, 0));



}

void CutandRotate(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	int cutposition=0;
	Mat w = GetLeafwide(r);
	medianBlur(w, w, 5);
	vector<double> slopes;
	double maxValue;
	Point maxIdx;
	minMaxLoc(w, 0, &maxValue, 0, &maxIdx);
	for (int i = maxIdx.x+20; i <r.cols; i++){
		double temp = ((double)w.at<ushort>(0, i-20 )-(double)w.at<ushort>(0, i))/20;
		slopes.push_back (temp);
	}
	for (int i = r.cols - 1; i >= 0; i--){
		int s = i - (r.cols - slopes.size());
		if (s>=0 && w.at<ushort>(0, i) < 20 && slopes[s]< 1){
			Mat white(Size(r.cols - i, r.rows), CV_8UC3, Scalar(255, 255, 255));
			Mat black(Size(r.cols - i, r.rows), CV_8UC3, Scalar(0, 0, 0));
			Rect window = Rect(Point(i, 0), Point(r.cols, r.rows));
			white.copyTo(r(window));
			black.copyTo(t(window));
		}
		else if (s < 0) break;
	}
	RotateLeaf(r, t);
	r.copyTo(dst_r);
	t.copyTo(dst_t);
}


Mat GetLeafwide(Mat &r){
	int top, bottom;
	Mat wide(Size(r.cols, 1), CV_16UC1);
	bool findtop=0,findbottom=0;
	for (int j = 0; j < r.cols; j++){
		int i = 0;
		while (!findtop && i < r.rows){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				top = i;
				findtop = 1;
				i=r.rows-1;
				break;
			}
			i++;
		}
		while (findtop &&!findbottom && i >=top && i < r.rows){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				bottom = i+1;
				findbottom = 1;
				break;
			}
			i--;
		}
		int w = 0;
		if (findtop&&findbottom) w = bottom - top;
		wide.at<ushort>(0, j) = (ushort)w;
		findtop = 0; findbottom = 0;
	}
	return wide;
}





RotatedRect GetRotatedRect(Mat &r){
	Mat gray_R, canny_R;
	cvtColor(r, gray_R, CV_RGB2GRAY);
	Canny(gray_R, canny_R, 100, 300, 3);
	vector<Point> points;
	for (int i = 0; i < canny_R.rows; i++){
		for (int j = 0; j < canny_R.cols; j++){
			if (canny_R.at<uchar>(i, j) == 255){
				Point p;
				p.x = j;
				p.y = i;
				points.push_back(p);
			}

		}
	}
	RotatedRect box = minAreaRect(Mat(points));
	Point2f vertex[4];
	box.points(vertex);

	line(r, vertex[0], vertex[1], Scalar(0, 0, 255), 1);
	line(r, vertex[1], vertex[2], Scalar(0, 0, 255), 1);
	line(r, vertex[2], vertex[3], Scalar(0, 0, 255), 1);
	line(r, vertex[3], vertex[0], Scalar(0, 0, 255), 1);
	return box;
}