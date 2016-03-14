#include"Header.h"






//图像填补
void FillImage( Mat &t){
	for (int i = 0; i <t.rows; i++){
		for (int j = 0; j < t.cols; j++){
			Scalar temp = t.at<Vec3b>(i, j);
			if (temp[0] == 255 && temp[1] == 255 && temp[2] == 255)
				t.at<Vec3b>(i, j) = { 0, 0, 0 };
		}
	}
}




//读取批量文件
void FileList::getList(const char *initDir, const char *specfile){
	char dir_Full[200];
	_fullpath(dir_Full, initDir, 200);//转换为绝对路径
	_chdir(dir_Full);//changedir
	int len = strlen(dir_Full);
	if (dir_Full[len - 1] != '\\')
		strcat(dir_Full, "\\");//修饰结尾
	long Hfile;//搜索句柄
	_finddata_t fileinfo;//数据体
	Hfile = _findfirst(specfile, &fileinfo);//在当前目录查找第一个具体文件类型
	if (Hfile != -1){
		int flag;
		do{
			string buf = dir_Full;
			buf += fileinfo.name;
			names.push_back(buf);
			flag = _findnext(Hfile, &fileinfo);//查找下一个

		} while (flag == 0);
	}
	else{//获取第一个文件失败
		cout << "获取第一个文件失败" << endl;
	}
	_findclose(Hfile);//查找结束，关闭句柄	
	dir = dir_Full;
	spec = specfile;
	qty = names.size();
}

//存储输出图片
void WriteBMP(string file_name, int file_num, Mat& r, Mat& t)//
{
	string dst_pathname1 = "C:/Users/eva72/Desktop/烟叶CACHE/Leftcut/";
	char temp[4];
	itoa(file_num + 1, temp, 10);
	string dst_num = temp;
	imwrite(dst_pathname1 + dst_num + "反_" + file_name, r);
	imwrite(dst_pathname1 + dst_num + "透_" + file_name, t);
	
}



//显示直方图
void Showhist(Mat &srcImage){
		system("color 3F");
		Mat grayImage;
		cvtColor(srcImage, grayImage, CV_BGR2GRAY);

		//【2】参数准备
		int bins = 256;
		int hist_size[] = { bins };
		float range[] = { 0, 256 };
		const float* ranges[] = { range };
		MatND grayHist,redHist, greenHist, blueHist;


		



		//【3】进行直方图的计算（红色分量部分）
		int channels_r[] = { 0 };
		calcHist(&srcImage, 1, channels_r, Mat(), //不使用掩膜
			redHist, 1, hist_size, ranges,
			true, false);

		//【4】进行直方图的计算（绿色分量部分）
		int channels_g[] = { 1 };
		calcHist(&srcImage, 1, channels_g, Mat(), // do not use mask
			greenHist, 1, hist_size, ranges,
			true, // the histogram is uniform
			false);

		//【5】进行直方图的计算（蓝色分量部分）
		int channels_b[] = { 2 };
		calcHist(&srcImage, 1, channels_b, Mat(), // do not use mask
			blueHist, 1, hist_size, ranges,
			true, // the histogram is uniform
			false);

		//【】对灰度进行直方图计算
		int channels_gray[] = { 0 };
		calcHist(&grayImage, 1, channels_gray, Mat(), //不使用掩膜
			grayHist, 1, hist_size, ranges,
			true, false);

		//-----------------------绘制出三色直方图------------------------
		//参数准备
		double maxValue_red, maxValue_green, maxValue_blue,maxValue_gray;
		minMaxLoc(redHist, 0, &maxValue_red, 0, 0);
		minMaxLoc(greenHist, 0, &maxValue_green, 0, 0);
		minMaxLoc(blueHist, 0, &maxValue_blue, 0, 0);
		minMaxLoc(grayHist, 0, &maxValue_gray, 0, 0);
		int scale = 1;
		int histHeight = 256;
		Mat histImage = Mat::zeros(histHeight, bins * 4, CV_8UC3);

		//正式开始绘制
		for (int i = 0; i<bins; i++)
		{
			//参数准备
			float binValue_red = redHist.at<float>(i);
			float binValue_green = greenHist.at<float>(i);
			float binValue_blue = blueHist.at<float>(i);
			float binValue_gray = grayHist.at<float>(i);
			int intensity_red = cvRound(binValue_red*histHeight / maxValue_red);  //要绘制的高度
			int intensity_green = cvRound(binValue_green*histHeight / maxValue_green);  //要绘制的高度
			int intensity_blue = cvRound(binValue_blue*histHeight / maxValue_blue);  //要绘制的高度
			int intensity_gray = cvRound(binValue_gray*histHeight / maxValue_gray);  //要绘制的高度

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

			//绘制灰色分量的直方图
			rectangle(histImage, Point((i + bins * 3)*scale, histHeight - 1),
				Point((i + bins * 3 + 1)*scale - 1, histHeight - intensity_gray),
				Scalar(96, 96, 96));



		}

		//在窗口中显示出绘制好的直方图
		imshow("图像的RGB直方图", histImage);

		waitKey(1000);
		string dst_filename = "rgb直方图.bmp";
		WriteBMP(dst_filename, 0, histImage, histImage);
}









