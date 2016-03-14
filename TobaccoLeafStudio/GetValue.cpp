#include"Header.h"

vector<double> GetValue(Mat &r, Mat &t){

	vector<double> Value;

	//图像去梗、旋转
	Mat cut_R, cut_T;
	Mat mask(r.size(), CV_8UC1);
	
	vector<Point> LeafPoints = CutandRotate(r, t, cut_R, cut_T,mask);

	//求最小外周矩
	Rect brect = boundingRect(Mat(LeafPoints));
	//求分区域点集
	int length = brect.width;
	int perlen = length / 3;
	if (perlen * 3 < length) perlen++;
	vector<vector<Point>> perPoints;
	float radius[3];
	for (int k = 0; k < 3; k++){
		Rect window = Rect(Point(60+k*perlen, 0), Point(60 + (k+1)*perlen, r.rows));
		vector<Point> Ptemp;
		for (int i = 0; i < r.rows; i++){
			for (int j = 0; j < perlen; j++){
				if (mask(window).at<uchar>(i, j) == 255){
					Ptemp.push_back(Point(60 + k*perlen+j, i));
				}
			}
		}
		Point2f center;

		//绘制外接最小圆
		/*
		minEnclosingCircle(Mat(Ptemp), center, radius[k]);
		Mat circle_mask(r.size(), CV_8UC1, Scalar::all(0));
		circle(circle_mask, center, cvRound(radius[k]), Scalar(255, 0, 0));
		floodFill(circle_mask, center, Scalar(255, 0, 0));
		*/

		//分块绘制外接矩形
		Mat per_mask(r.size(), CV_8UC1, Scalar::all(0));
		Rect perRect = boundingRect(Mat(Ptemp));
		Mat white(perRect.size(), CV_8UC1, Scalar::all(255));
		white.copyTo(per_mask(perRect));
		Value.push_back((double)perRect.height / (double)perRect.width);
		
		double r = CalRoundness(mask, per_mask);
		Value.push_back(r);
		perPoints.push_back(Ptemp);
	}

	
	//求最小外接旋转矩b
	RotatedRect rrect = minAreaRect(Mat(LeafPoints));

	//绘制外接最小旋转矩形
	Point2f vertex[4];
	rrect.points(vertex);


	if (rrect.size.height > rrect.size.width)
		Value.push_back(rrect.size.height / rrect.size.width);//长宽比
	else
		Value.push_back(rrect.size.width / rrect.size.height);//长宽比

	Value.push_back((double)brect.width / (double)brect.height);




	





	//保存图像
	//string dst_filename = "Leftcut.bmp";
	//WriteBMP(dst_filename, img_num, cut_R, cut_T);

	return Value;
}


double CalRoundness(Mat &mask, Mat &circle_mask){
	int m=0,n = 0;
	for (int i = 0; i < mask.rows; i++){
		for (int j = 0; j < mask.rows; j++){
			if (circle_mask.at<uchar>(i, j) == 255 ){
				m++;				
				if (mask.at<uchar>(i, j) == 255){
					n++;
				}
			}
		}
	}
	double r = (double)n / (double)m;
	return r;

}



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
	//平移
	//Point2f middle = Point2f((Left.x + Right.x) / 2, (Left.y + Right.y) / 2);
	Point2f srcTriangle[3];
	Point2f dstTriangle[3];
	srcTriangle[0] = Left;
	dstTriangle[0] = Point2f(60, 308);
	srcTriangle[1] = Point2f(0, 0);
	dstTriangle[1] = Point2f(60 - Left.x, 308 - Left.y);
	srcTriangle[2] = Point2f(100, 100);
	dstTriangle[2] = Point2f(100 + 60 - Left.x, 100 + 308 - Left.y);
	Mat	warpMat = getAffineTransform(srcTriangle, dstTriangle);
	warpAffine(r, r, warpMat, r.size(), INTER_NEAREST, 0, Scalar(255, 255, 255));
	warpAffine(t, t, warpMat, r.size(), INTER_NEAREST, 0, Scalar(0, 0, 0));


}

vector<Point> CutandRotate(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t,Mat &mask){
	int cutposition = 0;
	Mat w = GetLeafwide(r);
	medianBlur(w, w, 5);
	vector<double> slopes;
	double maxValue;
	Point maxIdx;
	minMaxLoc(w, 0, &maxValue, 0, &maxIdx);
	for (int i = maxIdx.x + 20; i <r.cols; i++){
		double temp = ((double)w.at<ushort>(0, i - 20) - (double)w.at<ushort>(0, i)) / 20;
		slopes.push_back(temp);
	}
	for (int i = r.cols - 1; i >= 0; i--){
		int s = i - (r.cols - slopes.size());
		if (s >= 0 && w.at<ushort>(0, i) < 20 && slopes[s]< 1){
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

	

	vector<Point> points;
	for (int i = 0; i < r.rows; i++){
		for (int j = 0; j < r.cols; j++){
			if (r.at<Vec3b>(i, j)[0] == 255 && r.at<Vec3b>(i, j)[1] == 255 && r.at<Vec3b>(i, j)[2] == 255){
				mask.at<uchar>(i, j) = 0;
			}
			else {
				points.push_back(Point(j, i));
				mask.at<uchar>(i, j) = 255;
			}

		}
	}

	return points;
}


Mat GetLeafwide(Mat &r){
	int top, bottom;
	Mat wide(Size(r.cols, 1), CV_16UC1);
	bool findtop = 0, findbottom = 0;
	for (int j = 0; j < r.cols; j++){
		int i = 0;
		while (!findtop && i < r.rows){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				top = i;
				findtop = 1;
				i = r.rows - 1;
				break;
			}
			i++;
		}
		while (findtop &&!findbottom && i >= top && i < r.rows){
			if (r.at<Vec3b>(i, j)[0] != 255 || r.at<Vec3b>(i, j)[1] != 255 || r.at<Vec3b>(i, j)[2] != 255){
				bottom = i + 1;
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