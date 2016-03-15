#include"Header.h"

//获得分区颜色值
vector<double> CalLabValue(Mat &r, Mat &t, vector<vector<Point>> Points){
	vector<double> TriColors;
	double temp[6];
	double sum[3][6] = { 0 };
	int num_pieces = 3;
	for (int k = 0; k < num_pieces; k++){
		int num_pp = Points[k].size();
		for (int i = 0; i < num_pp; i++){
			//x,y顺序不能搞错了……
			sum[k][0] += ((double)(r.at<Vec3b>(Points[k][i].y, Points[k][i].x)[0])/255*100);
			sum[k][1] += ((double)(r.at<Vec3b>(Points[k][i].y, Points[k][i].x)[1])-128);
			sum[k][2] += ((double)(r.at<Vec3b>(Points[k][i].y, Points[k][i].x)[2])-128);
			sum[k][3] += ((double)(t.at<Vec3b>(Points[k][i].y, Points[k][i].x)[0]) / 255 * 100);
			sum[k][4] += ((double)(t.at<Vec3b>(Points[k][i].y, Points[k][i].x)[1])-128);
			sum[k][5] += ((double)(t.at<Vec3b>(Points[k][i].y, Points[k][i].x)[2])-128);
		}
		temp[0] = sum[k][0] / num_pp;
		temp[1] = sum[k][1] / num_pp;
		temp[2] = sum[k][2] / num_pp;
		temp[3] = sum[k][3] / num_pp;
		temp[4] = sum[k][4] / num_pp;
		temp[5] = sum[k][5] / num_pp;

		TriColors.push_back(temp[0]);
		TriColors.push_back(temp[1]);
		TriColors.push_back(temp[2]);
		TriColors.push_back(temp[3]);
		TriColors.push_back(temp[4]);
		TriColors.push_back(temp[5]);

	}
	return TriColors;
}


vector<vector<Point>> GetPerPoints(Mat &mask){
	vector<vector<Point>> perPoints;//区域点集

	vector<double> Value;

	//用外周矩求烟叶长
	Rect brect = boundingRect(mask);

	//求分区域点集
	int length = brect.width;
	int perlen = length / 3;
	if (perlen * 3 < length) perlen++;

	float radius[3];
	for (int k = 0; k < 3; k++){
		Rect window = Rect(Point(60 + k*perlen, 0), Point(60 + (k + 1)*perlen, mask.rows));
		vector<Point> Ptemp;
		for (int i = 0; i < mask.rows; i++){
			for (int j = 0; j < perlen; j++){
				if (mask(window).at<uchar>(i, j) == 255){
					Ptemp.push_back(Point(60 + k*perlen + j, i));
				}
			}
		}
		perPoints.push_back(Ptemp);
	}
	return perPoints;
}

vector<double> GetValue(vector<Point> LeafPoints,Mat &mask){
	vector<double> Value;
	//求最小外周矩
	Rect brect = boundingRect(Mat(LeafPoints));
	//求分区域点集
	int length = brect.width;
	int perlen = length / 3;
	if (perlen * 3 < length) perlen++;
	vector<vector<Point>> perPoints;//区域点集
	float radius[3];
	for (int k = 0; k < 3; k++){
		Rect window = Rect(Point(60 + k*perlen, 0), Point(60 + (k + 1)*perlen, mask.rows));
		vector<Point> Ptemp;
		for (int i = 0; i < mask.rows; i++){
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
		Mat per_mask(mask.size(), CV_8UC1, Scalar::all(0));
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