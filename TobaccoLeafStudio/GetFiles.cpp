#include"Header.h"

//图像平滑
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);//双边滤波
	bilateralFilter(t, dst_t, 15, 15 * 2, 15 / 2);
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
	string dst_pathname1 = "C:/Users/eva72/Desktop/烟叶CACHE/";
	char temp[2];
	itoa(file_num + 1, temp, 10);
	string dst_num = temp;
	imwrite(dst_pathname1 + dst_num + "反_" + file_name, r);
	imwrite(dst_pathname1 + dst_num + "透_" + file_name, t);
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
					sum[k] += temp2 / (255-temp1);
				}
			}
			sum[k] = sum[k] / (board_b.rows*board_b.cols);
		}
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);//检验黑板校验效果
}



void SegmentationImage(Mat &img_R, Mat &img_T){

	


	int new_rows = img_T.rows / 3;
	int new_cols = img_T.cols / 3;
	Mat mean_img(Size(new_cols, new_rows), CV_32FC1);
	Mat std_img(Size(new_cols, new_rows), CV_32FC1);
	vector<Mat> channels_T;
	split(img_T, channels_T);
	for (int i = 0; i < new_rows; i++){
		for (int j = 0; j < new_cols; j++){
			
			Mat rect_Blue_T = channels_T[0](Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
			Mat mean_Mat, std_Mat;
			meanStdDev(rect_Blue_T, mean_Mat, std_Mat);		
			double mean_rect = mean_Mat.at<double>(0, 0);
			double std_rect = std_Mat.at<double>(0, 0);
			mean_img.at<float>(i, j) =(float) mean_rect;
			std_img.at<float>(i, j) = (float) std_rect;
		}
	}

	int t_mean = Getthreshold(mean_img);//单通道图像
	int t_std = Getthreshold(std_img);//单通道图像

	Mat temp;
	threshold(mean_img, mean_img, t_mean, 255, CV_THRESH_BINARY);
	threshold(std_img, std_img, t_std, 255, CV_THRESH_BINARY);

	for(int i = 0; i < new_rows; i++){
		for (int j = 0; j < new_cols; j++){
			if (mean_img.at<float>(i, j) == 0){
				//烟叶
				if (img_R.at<Vec3b>(3 * i, 3 * j)[0] > img_R.at<Vec3b>(3 * i, 3 * j)[2]){//背景边缘
					Mat window_T = img_T(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat window_R = img_R(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
					Mat black(3, 3, CV_8UC3, Scalar(0, 0, 0));
					white.copyTo(window_R);
					black.copyTo(window_T);
				}				
			}
			else{
				if (std_img.at<float>(i, j) != 0){  //边缘
					Mat window_T = img_T(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat window_R = img_R(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
					Mat black(3, 3, CV_8UC3, Scalar(0, 0, 0));
					white.copyTo(window_R);
					black.copyTo(window_T);
					/*
					Mat window_T = img_T(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat window_R = img_R(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					for (int m = 0; m < 3; m++){
						for (int n = 0; n < 3; n++){
							if (window_T.at<Vec3b>(m, n)[0]>0)	{//背景
								window_T.at<Vec3b>(m, n) = { 0, 0, 0 };
								window_R.at<Vec3b>(m, n) = { 255, 255, 255 };
							}

						}
					}
					*/

				}
				else {  //背景
					Mat window_T = img_T(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat window_R = img_R(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
					Mat black(3, 3, CV_8UC3, Scalar(0, 0, 0));
					white.copyTo(window_R);
					black.copyTo(window_T);					
				}
			}
		}
	}

	


	/*
	
	double t1, t2, t3;
	t1 = threshold(new_img, temp, 20, 255, CV_THRESH_BINARY);
	//t2 = threshold(new_img, temp, 0, 255, CV_THRESH_OTSU);
	//t3 = threshold(new_img, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);




	for (int i = 0; i < img_T.rows / 3; i += 3){
		for (int j = 0; j < img_T.cols / 3; j += 3){
			Mat rect_R = img_R(Rect(Point(i, j), Point(i + 3, j + 3)));
			Mat rect_T = img_T(Rect(Point(i, j), Point(i + 3, j + 3)));
			vector<Mat> channels_T;
			split(rect_T, channels_T);
			Mat m_T, s_T;
			meanStdDev(channels_T[0], m_T, s_T);
			if (m_T.at<double>(0, 0) == 0){//考虑为烟叶部分
				Scalar rgb = mean(rect_R);//考察相应得到反射图像
				if (rgb[0] > rgb[2]){//反射图的背景蓝色大于红色
					rect_R = Scalar::all(255);
					rect_T = Scalar::all(0);
				}
			}
			else{
				if (s_T.at<double>(0, 0) > 1){//边缘部分
					for (int k = 0; k < 3; k++){
						for (int l = 0; l < 3; l++){
							if (channels_T[0].at<uchar>(k, l) != 0){
								rect_R.at<Vec3b>(k, l) = Vec3b(255, 255, 255);
								rect_T.at<Vec3b>(k, l) = Vec3b(0, 0, 0);
							}

						}
					}
				}
				else{//背景部分
					cout << "背景" << endl;
				}
			}

		}
	}
	*/
}



int Getthreshold(Mat &srcImage){
	int i;

	//【2】参数准备

	int hist_size=256;
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
	medianBlur(blueHist, blueHist, 5);


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
	return i-1;
}



		




		

		/*-----------------------绘制出三色直方图------------------------
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







