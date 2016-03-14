#include"Header.h"

string filespec1 = "*反*.bmp";
string filespec2 = "*透*.bmp";
string Pathname = "C:/Users/eva72/Desktop/烟叶CACHE/seg2/";
//string dst_name = "输出结果";



int main(){
	//批量读取图片
	FileList imgs_R, imgs_T;
	imgs_R.getList(&Pathname[0], &filespec1[0]);//反射图像
	imgs_T.getList(&Pathname[0], &filespec2[0]);//透射图像
	if (imgs_R.qty != imgs_T.qty)  {
		cout << "错误！图片不配对";
		return -1;
	}
	vector<vector<double>> imgs_value;
	for (int img_num = 0; img_num < imgs_R.qty; img_num++){
		Mat img_R = imread(imgs_R.names[img_num], -1);
		Mat img_T = imread(imgs_T.names[img_num], -1);

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
		imgs_value.push_back(GetValue(img_R, img_T));




		

		
		



	//最小外接矩阵
	//	RotatedRect box = GetRotatedRect(img_R);
		
	}


		


	

	/*
	Mat img_R = imread("C:/Users/eva72/Documents/毕业实验/ps正反对齐_云南昭通中下部烟叶照片/云南昭通中下部烟叶照片/001云南昭通烟叶X1L一号叶反射.jpg", -1);
	Mat img_blur;
	//blur(img_R, img_dst, Size(5, 5));
	imshow("原图", img_R);
	//imshow("邻近均值滤波", img_dst);
	//medianBlur(img_R, img_dst, 5);
	//imshow("中值滤波", img_dst);
	bilateralFilter(img_R, img_blur, 5, 5 * 2, 5 / 2);
	imshow("双边滤波", img_blur);


	string dst_filename = "result.bmp";
	imwrite(dst_pathname + dst_filename, result);


	

	*/
	








	/*//批量读取图片
	
	for (int img_num = 0; img_num < imgs_R.qty; img_num++){

		Mat img_R = imread(imgs_R.names[img_num],-1);
		Mat img_T = imread(imgs_T.names[img_num],-1);

		int new_rows = img_T.rows / 3;
		int new_cols = img_T.cols / 3;
		Mat new_img(Size(new_cols, new_rows), CV_8UC1);
		vector<Mat> channels_T;
		split(img_T, channels_T);
		for (int i = 0; i < new_rows; i ++){
			for (int j = 0; j < new_cols; j ++){
				Mat rect_Blue_T = channels_T[0](Rect(Point(3*j, 3*i), Point(3*j + 3, 3*i + 3)));
				new_img.at<uchar>(i, j) =(uchar)mean(rect_Blue_T)[0];
			}
		}
		Mat temp;
		double t1,t2,t3;
		t1 = threshold(new_img, temp, 20, 255, CV_THRESH_BINARY);
		t2 = threshold(new_img, temp, 0, 255, CV_THRESH_OTSU);
		t3 = threshold(new_img, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	
	*/
	ofstream outfile("C:/Users/eva72/Desktop/烟叶CACHE/images_value.xls");
	for (int k = 0; k<imgs_R.qty; k++)
	{
		vector<double> temp = imgs_value[k];
		for (int j = 0; j<8; j++)
		{
			outfile <<temp[j]  << "\t";
		}
		outfile << endl;
	} 


		
		

	
	waitKey(6000);
return 0;
}


