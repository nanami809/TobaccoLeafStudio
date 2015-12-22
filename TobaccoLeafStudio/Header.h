#include<opencv2/opencv.hpp>
#include<iostream>
#include <io.h>
#include<vector>
#include "direct.h"

using namespace std;
using namespace cv;



//void showMat(Mat arr);

class FileList{
public:
	int qty;
	void getList(const char *initDir, const char *specfile);
	vector<string> names;
private:
	string dir;
	string spec;
};


/*void showMat(Mat arr){
	for (int i = 0; i < arr.rows; i++){
		for (int j = 0; j < arr.cols; j++)
			cout << arr.at<uchar>(i, j) << "\0";
		cout << endl;
	}
}*/


