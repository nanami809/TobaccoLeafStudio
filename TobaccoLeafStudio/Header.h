#include<opencv2/opencv.hpp>
#include<iostream>
#include <io.h>
#include<vector>
#include "direct.h"
#include<math.h>

using namespace std;
using namespace cv;



//函数声明

//GetFiles函数组
void WriteBMP(string file_name, int file_num, Mat &r, Mat &t);
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t);
//void showMat(Mat arr);
void ColorCorect(Rect window, Mat &r, Mat &t, Mat &dst_r, Mat &dst_t);
void CalCorect(Scalar w_rgb, Scalar b_rgb, Mat &r, Mat &t, Mat& dst_r, Mat& dst_t);
void SegmentationImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t);
int Getthreshold_Gray(Mat &srcImage);
int Getthreshold_CV(Mat &srcImage);
int Getthreshold_DieDai(Mat &srcImage,Mat &dst);//单通道图像
//void FillImage(Mat &t);
void Showhist(Mat &srcImage);

//Getvalue函数组
RotatedRect GetRotatedRect(Mat &r);
Mat GetLeafwide(Mat &r);
void RotateLeaf(Mat &r, Mat &t);
void CutandRotate(Mat &r,Mat &t ,Mat &dst_r,Mat &dst_t);


//类
class FileList{//批量文件
public:
	int qty;
	void getList(const char *initDir, const char *specfile);
	vector<string> names;
private:
	string dir;
	string spec;
};


