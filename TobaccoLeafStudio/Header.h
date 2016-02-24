#include<opencv2/opencv.hpp>
#include<iostream>
#include <io.h>
#include<vector>
#include "direct.h"
#include<math.h>

using namespace std;
using namespace cv;



//函数声明
void WriteBMP(string file_name, int file_num, Mat &r, Mat &t);
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t);
//void showMat(Mat arr);
void ColorCorect(Rect window, Mat &r, Mat &t, Mat &dst_r, Mat &dst_t);
void CalCorect(Scalar w_rgb, Scalar b_rgb, Mat &r, Mat &t, Mat& dst_r, Mat& dst_t);
void SegmentationImage(Mat &img_R, Mat &img_T);
int Getthreshold(Mat &srcImage);


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



//函数


/*void showMat(Mat arr){
	for (int i = 0; i < arr.rows; i++){
		for (int j = 0; j < arr.cols; j++)
			cout << arr.at<uchar>(i, j) << "\0";
		cout << endl;
	}
}*/




