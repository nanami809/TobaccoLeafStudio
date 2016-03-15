#include"Header.h"


//图像预处理
/*
//图像平滑
Mat img_blur_R,img_blur_T;
//img_R.copyTo(img_blur_R); img_T.copyTo(img_blur_T);
BlurImage(img_R, img_T, img_blur_R, img_blur_T);

//颜色校正
Mat img_cor_R, img_cor_T;
//img_blur_R.copyTo(img_cor_R); img_blur_T.copyTo(img_cor_T);
Rect recwindow(680, 415, 130, 100);//预测白板出现的位置
ColorCorect(recwindow, img_blur_R, img_blur_T, img_cor_R, img_cor_T);

//图像分割
Mat img_seg_R, img_seg_T;
SegmentationImage(img_cor_R, img_cor_T,img_seg_R,img_seg_T);

//把白板处填好
Mat white(recwindow.size(), CV_8UC3, Scalar(255, 255, 255));
Mat black(recwindow.size(), CV_8UC3, Scalar(0, 0, 0));
white.copyTo(img_seg_R(recwindow));
black.copyTo(img_seg_T(recwindow));
*/


//图像平滑
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);//双边滤波
bilateralFilter(t, dst_t, 15, 15 * 2, 15 / 2);
}

//颜色校正
void ColorCorect(Rect window, Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	r.copyTo(dst_r);
	t.copyTo(dst_t);
	Mat mask;//掩膜
	Mat bgModel, fgModel;//两个临时矩阵变量，作为算法的中间变量使用，不用care
	double tt = getTickCount();
	// GrabCut 分段
	cv::grabCut(r,    //输入图像
		mask,   //分段结果
		window,// 包含前景的矩形 
		bgModel, fgModel, // 前景、背景
		1,        // 迭代次数
		cv::GC_INIT_WITH_RECT); // 用矩形
	compare(mask, cv::GC_PR_FGD, mask, cv::CMP_EQ);// 表示出白板区域
	Mat element = getStructuringElement(MORPH_RECT, Size(9, 9));
	erode(mask, mask, element);//腐蚀算法

	Rect box = boundingRect(mask);//求外接矩形边界
	/*
	//绘制盒子
	line(img_blur_R, Point(box.x, box.y), Point(box.x + box.width, box.y), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x, box.y), Point(box.x, box.y + box.height), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x + box.width, box.y), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x, box.y + box.height), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255), 3);
	*/
	//求白板、黑板在非标准情况下的颜色
	Mat board_w = dst_r(Rect(box.x + 5, box.y + 5, box.width - 10, box.height - 10));
	Mat board_b = dst_t(Rect(box.x + 5, box.y + 5, box.width - 10, box.height - 10));
	double sum[3] = { 0, 0, 0 };
	do{
		Scalar w_rgb = mean(board_w);
		for (int i = 0; i < r.rows; i++){
			for (int j = 0; j < r.cols; j++){
				for (int k = 0; k <= 2; k++){
					//白板校正
					int temp1 = 255 * r.at<Vec3b>(i, j)[k] / w_rgb[k];
					if (temp1 > 255) dst_r.at<Vec3b>(i, j)[k] = 255;
					else            dst_r.at<Vec3b>(i, j)[k] = temp1;
				}
			}
		}
		for (int k = 0; k <= 2; k++){
			for (int i = 0; i < board_w.rows; i++){
				for (int j = 0; j < board_w.cols; j++){
					int temp1 = board_w.at<Vec3b>(i, j)[k];
					int temp2 = abs(255 - temp1);
					sum[k] += temp2 / temp1;
				}
			}
			sum[k] = sum[k] / (board_w.rows*board_w.cols);
		}
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);//检验白板校正效果

	sum[0] = 0; sum[1] = 0; sum[2] = 0;
	do{
		Scalar b_rgb = mean(board_b);
		for (int i = 0; i < r.rows; i++){
			for (int j = 0; j < r.cols; j++){
				for (int k = 0; k <= 2; k++){
					//黑板校正
					int temp2 = 255 - (255 - t.at<Vec3b>(i, j)[k]) * 255 / (255 - b_rgb[k]);
					if (temp2 <0)   dst_t.at<Vec3b>(i, j)[k] = 0;
					else            dst_t.at<Vec3b>(i, j)[k] = temp2;
				}
			}
		}
		for (int k = 0; k <= 2; k++){
			for (int i = 0; i < board_b.rows; i++){
				for (int j = 0; j < board_b.cols; j++){
					int temp1 = board_b.at<Vec3b>(i, j)[k];
					int temp2 = abs(0 - temp1);
					sum[k] += temp2 / (255 - temp1);
				}
			}
			sum[k] = sum[k] / (board_b.rows*board_b.cols);
		}
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);//检验黑板校验效果
}

//图像分割(新论文方法）

void SegmentationImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){

	//建立输出图片
	Mat img_R;
	Mat img_T;
	r.copyTo(img_R);
	t.copyTo(img_T);


	//分析反射图像红蓝差的矩阵
	Mat D_redblue(r.rows, r.cols, CV_8UC1);
	for (int i = 0; i < t.rows; i++){
		for (int j = 0; j < t.cols; j++){
			Scalar temp = r.at<Vec3b>(i, j);
			if (temp[2] - temp[0]<0)D_redblue.at<uchar>(i, j) = 0;
			else 			D_redblue.at<uchar>(i, j) = (uchar)(temp[2] - temp[0]);
		}
	}
	int t_DRB = Getthreshold_Gray(D_redblue);

	//记录边缘区域的矩阵
	Mat img_Counter1(Size(t.cols, t.rows), CV_8UC3, Scalar(0, 0, 0));
	Mat img_Counter2(Size(t.cols, t.rows), CV_8UC3, Scalar(255, 255, 255));//记录边缘	
	//计算均值和变异系数分布
	Mat mean_img(Size(img_T.cols - 2, img_T.rows - 2), CV_8UC1);
	Mat cv_img(Size(img_T.cols - 2, img_T.rows - 2), CV_8UC1);
	vector<Mat> channels_T;
	split(t, channels_T);//分离透射图像蓝色值
	for (int i = 0; i < t.rows - 2; i++){
		for (int j = 0; j < t.cols - 2; j++){
			Mat rect_Blue_T = channels_T[0](Rect(Point(j, i), Point(j + 3, i + 3)));//3×3矩阵
			Mat mean_Mat, std_Mat;
			meanStdDev(rect_Blue_T, mean_Mat, std_Mat);
			double mean_rect = mean_Mat.at<double>(0, 0);
			double std_rect = std_Mat.at<double>(0, 0);
			mean_img.at<uchar>(i, j) = (uchar)mean_rect;
			if (mean_rect != 0){
				cv_img.at<uchar>(i, j) = (uchar)(std_rect / mean_rect / 3 * 255);
			}
			else cv_img.at<uchar>(i, j) = 0;
		}
	}

	int t_mean = Getthreshold_Gray(mean_img);//单通道图像
	threshold(mean_img, mean_img, t_mean, 255, CV_THRESH_BINARY);//平均值分割，标示烟叶部分

	int t_cv = threshold(cv_img, cv_img, 0, 255, CV_THRESH_OTSU);//变异系数分割，标示边缘

	Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
	Mat black(3, 3, CV_8UC3, Scalar(0, 0, 0));

	for (int i = 0; i < mean_img.rows; i++){
		for (int j = 0; j < mean_img.cols; j++){
			Rect window = Rect(Point(j, i), Point(j + 3, i + 3));//3×3大小窗口单步扫描
			if (mean_img.at<uchar>(i, j) == 0){
				//是烟叶		
				Scalar temp = mean(r(window));
				int blue = temp[0];
				int red = temp[2];
				if (red - blue <= t_DRB){
					//排除背景边缘	
					white.copyTo(img_R(window));
					black.copyTo(img_T(window));
				}
			}
			else{
				if (cv_img.at<uchar>(i, j) != 0){
					//边缘
					t(window).copyTo(img_Counter1(window));
					t(window).copyTo(img_Counter2(window));

				}
				else {
					//背景
					white.copyTo(img_R(window));
					black.copyTo(img_T(window));
				}
			}
		}
	}

	vector<Mat> channels_C;
	split(img_Counter2, channels_C);
	Mat temp = channels_C[0];
	int t_C = Getthreshold_Gray(temp);
	//int t_C = threshold(temp, temp, 0, 255, CV_THRESH_OTSU);
	threshold(temp, temp, t_C, 255, CV_THRESH_BINARY);
	for (int m = 0; m < temp.rows; m++){
		for (int n = 0; n < temp.cols; n++){
			if (temp.at<uchar>(m, n) == 0)	{
				//背景
				img_T.at<Vec3b>(m, n) = t.at<Vec3b>(m, n);
				img_R.at<Vec3b>(m, n) = r.at<Vec3b>(m, n);
			}
		}
	}

	split(img_Counter1, channels_C);
	temp = channels_C[0];
	threshold(temp, temp, t_C, 255, CV_THRESH_BINARY);
	for (int m = 0; m < temp.rows; m++){
		for (int n = 0; n < temp.cols; n++){
			if (temp.at<uchar>(m, n) != 0)	{
				//背景
				img_T.at<Vec3b>(m, n) = { 0, 0, 0 };
				img_R.at<Vec3b>(m, n) = { 255, 255, 255 };
			}
		}
	}

	img_R.copyTo(dst_r);
	img_T.copyTo(dst_t);

}

//图像分割(马文杰）
/*

void SegmentationImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
//不建立中间输出图片
//Mat img_R(r.rows, r.cols, CV_8UC3, Scalar(255, 255, 255));
//Mat img_T(t.rows, t.cols, CV_8UC3, Scalar(0, 0, 0));


//无分析反射图像红蓝差的矩阵
//Mat D_redblue(r.rows, r.cols, CV_8UC1);
//for (int i = 0; i < t.rows; i++){
//	for (int j = 0; j < t.cols; j++){
//		Scalar temp = r.at<Vec3b>(i, j);
//		if (temp[2] - temp[0]<0)D_redblue.at<uchar>(i, j) = 0;
//		else 			D_redblue.at<uchar>(i, j) = (uchar)(temp[2] - temp[0]);
//	}
//}
//int t_DRB = Getthreshold_Gray(D_redblue);

//无记录边缘区域的矩阵
//Mat img_Counter1(Size(t.cols, t.rows), CV_8UC3, Scalar(255,255, 255));
//Mat img_Counter2(Size(t.cols, t.rows), CV_8UC3, Scalar(255, 255, 255));//记录边缘
//计算均值和变异系数分布
Mat mean_img(Size(t.cols - 2, t.rows - 2), CV_8UC1);
Mat cv_img(Size(t.cols - 2, t.rows - 2), CV_8UC1);
vector<Mat> channels_T;
split(t, channels_T);//分离透射图像蓝色值
for (int i = 0; i < t.rows - 2; i++){
for (int j = 0; j < t.cols - 2; j++){
Mat rect_Blue_T = channels_T[0](Rect(Point(j, i), Point(j + 3, i + 3)));//3×3矩阵
Mat mean_Mat, std_Mat;
meanStdDev(rect_Blue_T, mean_Mat, std_Mat);
double mean_rect = mean_Mat.at<double>(0, 0);
double std_rect = std_Mat.at<double>(0, 0);
mean_img.at<uchar>(i, j) = (uchar)mean_rect;
if (mean_rect != 0){
cv_img.at<uchar>(i, j) = (uchar)(std_rect / mean_rect / 3 * 255);
}
else cv_img.at<uchar>(i, j) = 0;
}
}

int t_mean = 0; //取0               Getthreshold_Gray(mean_img);//单通道图像
int t_cv =55; //取100和10的中间值    threshold(cv_img, cv_img, 0, 255, CV_THRESH_OTSU);//变异系数分割，标示边缘


threshold(mean_img, mean_img, t_mean, 255, CV_THRESH_BINARY);//平均值分割，标示烟叶部分
threshold(cv_img, cv_img, t_cv, 255, CV_THRESH_BINARY);


for (int i = 0; i < mean_img.rows; i++){
for (int j = 0; j < mean_img.cols; j++){
Rect window = Rect(Point(j, i), Point(j + 3, i + 3));//3×3大小窗口单步扫描
if (mean_img.at<uchar>(i, j) == 0){
//是烟叶		不做处理
//Scalar temp = mean(r(window));
//int blue = temp[0];
//int red = temp[2];
//if (red - blue>t_DRB)
{
//不区分排除背景边缘
//r(window).copyTo(img_R(window));
//t(window).copyTo(img_T(window));
}
}
else{
if (cv_img.at<uchar>(i, j) != 0){
//边缘   不做处理
//t(window).copyTo(img_Counter1(window));
//t(window).copyTo(img_Counter2(window));
//r(window).copyTo(img_R(window));
//t(window).copyTo(img_T(window));

}
else {
//背景    设255 255 255
Mat white(3, 3, CV_8UC3, Scalar( 0,0,255));
white.copyTo(r(window));
white.copyTo(t(window));
}
}
}
}




r.copyTo(dst_r);
t.copyTo(dst_t);

}
*/


//获取灰度直方图阈值（单通道图像）
int Getthreshold_Gray(Mat &srcImage){
	int i;

	//【2】参数准备

	int hist_size = 256;
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	MatND redHist, grayHist, blueHist;

	/*

	//【3】进行直方图的计算（红色分量部分）
	calcHist(&srcImage, 1, channels_r, Mat(), //不使用掩膜
	redHist, 1, hist_size, ranges,
	true, false);

	//【4】进行直方图的计算（绿色分量部分）
	int channels_g[] = { 1 };
	calcHist(&srcImage, 1, channels_g, Mat(), // do not use mask
	grayHist, 1, hist_size, ranges,
	true, // the histogram is uniform
	false);
	*/
	//【5】进行直方图的计算（蓝色分量部分）
	int channels_b[] = { 0 };
	calcHist(&srcImage, 1, channels_b, Mat(), // do not use mask
		blueHist, 1, &hist_size, ranges);
	medianBlur(blueHist, blueHist, 3);


	//查找阈值
	double maxValue_blue;
	Point maxIdx_blue;
	minMaxLoc(blueHist, 0, &maxValue_blue, 0, &maxIdx_blue);
	if (maxIdx_blue.y == 0){
		for (i = 1; i < 256; i++){
			if (blueHist.at<float>(i) > blueHist.at<float>(i - 1)){
				break;
			}
		}
	}
	else i = 0;
	return i - 1;
}

//CV直方图阈值（未采用）
int Getthreshold_CV(Mat &srcImage){



	if (srcImage.type() != CV_8UC1)
		cvtColor(srcImage, srcImage, CV_RGB2GRAY);


	int i;
	//【2】参数准备

	int hist_size = 51;
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	MatND redHist, grayHist, blueHist;

	/*

	//【3】进行直方图的计算（红色分量部分）
	calcHist(&srcImage, 1, channels_r, Mat(), //不使用掩膜
	redHist, 1, hist_size, ranges,
	true, false);

	//【4】进行直方图的计算（绿色分量部分）
	int channels_g[] = { 1 };
	calcHist(&srcImage, 1, channels_g, Mat(), // do not use mask
	grayHist, 1, hist_size, ranges,
	true, // the histogram is uniform
	false);
	*/
	//【5】进行直方图的计算
	int channels_b[] = { 0 };
	calcHist(&srcImage, 1, channels_b, Mat(), // do not use mask
		blueHist, 1, &hist_size, ranges);
	medianBlur(blueHist, blueHist, 3);
	Mat temp = (Mat)blueHist;






	//查找阈值
	double maxValue_blue;
	Point maxIdx_blue;
	minMaxLoc(blueHist, 0, &maxValue_blue, 0, &maxIdx_blue);
	if (maxIdx_blue.y == 0){
		for (i = 1; i < 300; i++){
			if (blueHist.at<float>(i) > blueHist.at<float>(i - 1)){
				break;
			}
		}
	}
	else i = 0;
	return (i - 1) * 5;


}

//迭代法求阈值（未采用）
int Getthreshold_DieDai(Mat &srcImage, Mat &dst){
	Mat gray, temp;
	srcImage.copyTo(dst);

	if (srcImage.type() != CV_8UC1)
		cvtColor(srcImage, gray, CV_RGB2GRAY);
	else srcImage.copyTo(gray);
	//【2】参数准备
	double minval, maxval;
	int t, t1, t2, s;
	minMaxLoc(gray, &minval, &maxval);
	s = (minval + maxval) / 2;
	do{
		int n1 = 0, n2 = 0, sum1 = 0, sum2 = 0;
		t = s;
		threshold(gray, temp, t, 255, THRESH_BINARY);

		for (int i = 0; i < temp.rows; i++)
		{
			for (int j = 0; j < temp.cols; j++)
			{
				if (temp.at<uchar>(i, j) == 0){
					n1++;
					sum1 += gray.at<uchar>(i, j);
				}
				else{
					n2++;
					sum2 += gray.at<uchar>(i, j);
				}
			}
		}
		t1 = sum1 / n1;
		t2 = sum2 / n2;
		s = (t1 + t2) / 2;
	} while (s != t);


	if (srcImage.type() == CV_8UC3){

		//	Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
		for (int i = 0; i < temp.rows; i++)
		{
			for (int j = 0; j < temp.cols; j++)
			{
				if (temp.at<uchar>(i, j) == 0){
					dst.at<Vec3b>(i, j) = { 255, 255, 255 };
				}
				else{
				}
			}
		}
	}



	return t;
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

vector<Point> CutandRotate(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t, Mat &mask){
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



//对全局灰度图像迭代法（对比需要）
/*
Mat temp;
cvtColor(img_cor_R, temp, CV_RGB2GRAY);
int t = Getthreshold_DieDai(temp, temp);

threshold(temp, temp, t, 255, THRESH_BINARY);

for (int i = 0; i < temp.rows; i++)
{
for (int j = 0; j < temp.cols; j++)
{
if (temp.at<uchar>(i, j) == 0){
img_cor_R.at<Vec3b>(i, j) = {255, 255, 255};
}
else{

}
}
}

*/

//-----------------------绘制出三色直方图------------------------
/*
//参数准备
double maxValue_red, maxValue_green, maxValue_blue;
minMaxLoc(redHist, 0, &maxValue_red, 0, 0);
minMaxLoc(grayHist, 0, &maxValue_green, 0, 0);
minMaxLoc(blueHist, 0, &maxValue_blue, 0, 0);
int scale = 1;
int histHeight = 256;
Mat histImage = Mat::zeros(histHeight, bins * 3, CV_8UC3);

//正式开始绘制
for (int i = 0; i<bins; i++)
{
//参数准备
float binValue_red = redHist.at<float>(i);
float binValue_green = grayHist.at<float>(i);
float binValue_blue = blueHist.at<float>(i);
int intensity_red = cvRound(binValue_red*histHeight / maxValue_red);  //要绘制的高度
int intensity_green = cvRound(binValue_green*histHeight / maxValue_green);  //要绘制的高度
int intensity_blue = cvRound(binValue_blue*histHeight / maxValue_blue);  //要绘制的高度

//绘制红色分量的直方图
rectangle(histImage, Point(i*scale, histHeight - 1),
Point((i + 1)*scale - 1, histHeight - intensity_red),
Scalar(255, 0, 0));

//绘制绿色分量的直方图
rectangle(histImage, Point((i + bins)*scale, histHeight - 1),
Point((i + bins + 1)*scale - 1, histHeight - intensity_green),
Scalar(0, 255, 0));

//绘制蓝色分量的直方图
rectangle(histImage, Point((i + bins * 2)*scale, histHeight - 1),
Point((i + bins * 2 + 1)*scale - 1, histHeight - intensity_blue),
Scalar(0, 0, 255));

}

//在窗口中显示出绘制好的直方图
imshow("图像的RGB直方图", histImage);
waitKey(0);
*/



