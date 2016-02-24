#include"Header.h"

//ͼ��ƽ��
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);//˫���˲�
	bilateralFilter(t, dst_t, 15, 15 * 2, 15 / 2);
}

//��ȡ�����ļ�
void FileList::getList(const char *initDir, const char *specfile){
	char dir_Full[200];
	_fullpath(dir_Full, initDir, 200);//ת��Ϊ����·��
	_chdir(dir_Full);//changedir
	int len = strlen(dir_Full);
	if (dir_Full[len - 1] != '\\')
		strcat(dir_Full, "\\");//���ν�β
	long Hfile;//�������
	_finddata_t fileinfo;//������
	Hfile = _findfirst(specfile, &fileinfo);//�ڵ�ǰĿ¼���ҵ�һ�������ļ�����
	if (Hfile != -1){
		int flag;
		do{
			string buf = dir_Full;
			buf += fileinfo.name;
			names.push_back(buf);
			flag = _findnext(Hfile, &fileinfo);//������һ��

		} while (flag == 0);
	}
	else{//��ȡ��һ���ļ�ʧ��
		cout << "��ȡ��һ���ļ�ʧ��" << endl;
	}
	_findclose(Hfile);//���ҽ������رվ��	
	dir = dir_Full;
	spec = specfile;
	qty = names.size();
}

//�洢���ͼƬ
void WriteBMP(string file_name, int file_num, Mat& r, Mat& t)//
{
	string dst_pathname1 = "C:/Users/eva72/Desktop/��ҶCACHE/";
	char temp[2];
	itoa(file_num + 1, temp, 10);
	string dst_num = temp;
	imwrite(dst_pathname1 + dst_num + "��_" + file_name, r);
	imwrite(dst_pathname1 + dst_num + "͸_" + file_name, t);
}

//��ɫУ��
void ColorCorect(Rect window, Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	r.copyTo(dst_r);
	t.copyTo(dst_t);
	Mat mask;//��Ĥ
	Mat bgModel, fgModel;//������ʱ�����������Ϊ�㷨���м����ʹ�ã�����care
	double tt = getTickCount();
	// GrabCut �ֶ�
	cv::grabCut(r,    //����ͼ��
		mask,   //�ֶν��
		window,// ����ǰ���ľ��� 
		bgModel, fgModel, // ǰ��������
		1,        // ��������
		cv::GC_INIT_WITH_RECT); // �þ���
	compare(mask, cv::GC_PR_FGD, mask, cv::CMP_EQ);// ��ʾ���װ�����
	Mat element = getStructuringElement(MORPH_RECT, Size(9, 9));
	erode(mask, mask, element);//��ʴ�㷨
	Rect box = boundingRect(mask);//����Ӿ��α߽�
	/*
	//���ƺ���
	line(img_blur_R, Point(box.x, box.y), Point(box.x + box.width, box.y), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x, box.y), Point(box.x, box.y + box.height), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x + box.width, box.y), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255), 3);
	line(img_blur_R, Point(box.x, box.y + box.height), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255), 3);
	*/
	//��װ塢�ڰ��ڷǱ�׼����µ���ɫ
	Mat board_w = dst_r(Rect(box.x + 5, box.y + 5, box.width - 10, box.height - 10));	
	Mat board_b = dst_t(Rect(box.x + 5, box.y + 5, box.width - 10, box.height - 10));
	double sum[3] = { 0, 0, 0 };
	do{
		Scalar w_rgb = mean(board_w);
		for (int i = 0; i < r.rows; i++){
			for (int j = 0; j < r.cols; j++){
				for (int k = 0; k <= 2; k++){
					//�װ�У��
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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);//����װ�У��Ч��

	sum[0] = 0; sum[1] = 0; sum[2] = 0;
	do{
		Scalar b_rgb = mean(board_b);
		for (int i = 0; i < r.rows; i++){
			for (int j = 0; j < r.cols; j++){
				for (int k = 0; k <= 2; k++){
					//�ڰ�У��
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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);//����ڰ�У��Ч��
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

	int t_mean = Getthreshold(mean_img);//��ͨ��ͼ��
	int t_std = Getthreshold(std_img);//��ͨ��ͼ��

	Mat temp;
	threshold(mean_img, mean_img, t_mean, 255, CV_THRESH_BINARY);
	threshold(std_img, std_img, t_std, 255, CV_THRESH_BINARY);

	for(int i = 0; i < new_rows; i++){
		for (int j = 0; j < new_cols; j++){
			if (mean_img.at<float>(i, j) == 0){
				//��Ҷ
				if (img_R.at<Vec3b>(3 * i, 3 * j)[0] > img_R.at<Vec3b>(3 * i, 3 * j)[2]){//������Ե
					Mat window_T = img_T(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat window_R = img_R(Rect(Point(3 * j, 3 * i), Point(3 * j + 3, 3 * i + 3)));
					Mat white(3, 3, CV_8UC3, Scalar(255, 255, 255));
					Mat black(3, 3, CV_8UC3, Scalar(0, 0, 0));
					white.copyTo(window_R);
					black.copyTo(window_T);
				}				
			}
			else{
				if (std_img.at<float>(i, j) != 0){  //��Ե
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
							if (window_T.at<Vec3b>(m, n)[0]>0)	{//����
								window_T.at<Vec3b>(m, n) = { 0, 0, 0 };
								window_R.at<Vec3b>(m, n) = { 255, 255, 255 };
							}

						}
					}
					*/

				}
				else {  //����
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
			if (m_T.at<double>(0, 0) == 0){//����Ϊ��Ҷ����
				Scalar rgb = mean(rect_R);//������Ӧ�õ�����ͼ��
				if (rgb[0] > rgb[2]){//����ͼ�ı�����ɫ���ں�ɫ
					rect_R = Scalar::all(255);
					rect_T = Scalar::all(0);
				}
			}
			else{
				if (s_T.at<double>(0, 0) > 1){//��Ե����
					for (int k = 0; k < 3; k++){
						for (int l = 0; l < 3; l++){
							if (channels_T[0].at<uchar>(k, l) != 0){
								rect_R.at<Vec3b>(k, l) = Vec3b(255, 255, 255);
								rect_T.at<Vec3b>(k, l) = Vec3b(0, 0, 0);
							}

						}
					}
				}
				else{//��������
					cout << "����" << endl;
				}
			}

		}
	}
	*/
}



int Getthreshold(Mat &srcImage){
	int i;

	//��2������׼��

	int hist_size=256;
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	MatND redHist, grayHist, blueHist;


	/*

	//��3������ֱ��ͼ�ļ��㣨��ɫ�������֣�
	calcHist(&srcImage, 1, channels_r, Mat(), //��ʹ����Ĥ
	redHist, 1, hist_size, ranges,
	true, false);

	//��4������ֱ��ͼ�ļ��㣨��ɫ�������֣�
	int channels_g[] = { 1 };
	calcHist(&srcImage, 1, channels_g, Mat(), // do not use mask
	grayHist, 1, hist_size, ranges,
	true, // the histogram is uniform
	false);
	*/
	//��5������ֱ��ͼ�ļ��㣨��ɫ�������֣�
	int channels_b[] = { 0 };
	calcHist(&srcImage, 1, channels_b, Mat(), // do not use mask
		blueHist, 1, &hist_size, ranges);
	medianBlur(blueHist, blueHist, 5);


	//������ֵ
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



		




		

		/*-----------------------���Ƴ���ɫֱ��ͼ------------------------
		//����׼��
		double maxValue_red, maxValue_green, maxValue_blue;
		minMaxLoc(redHist, 0, &maxValue_red, 0, 0);
		minMaxLoc(grayHist, 0, &maxValue_green, 0, 0);
		minMaxLoc(blueHist, 0, &maxValue_blue, 0, 0);
		int scale = 1;
		int histHeight = 256;
		Mat histImage = Mat::zeros(histHeight, bins * 3, CV_8UC3);

		//��ʽ��ʼ����
		for (int i = 0; i<bins; i++)
		{
			//����׼��
			float binValue_red = redHist.at<float>(i);
			float binValue_green = grayHist.at<float>(i);
			float binValue_blue = blueHist.at<float>(i);
			int intensity_red = cvRound(binValue_red*histHeight / maxValue_red);  //Ҫ���Ƶĸ߶�
			int intensity_green = cvRound(binValue_green*histHeight / maxValue_green);  //Ҫ���Ƶĸ߶�
			int intensity_blue = cvRound(binValue_blue*histHeight / maxValue_blue);  //Ҫ���Ƶĸ߶�

			//���ƺ�ɫ������ֱ��ͼ
			rectangle(histImage, Point(i*scale, histHeight - 1),
				Point((i + 1)*scale - 1, histHeight - intensity_red),
				Scalar(255, 0, 0));

			//������ɫ������ֱ��ͼ
			rectangle(histImage, Point((i + bins)*scale, histHeight - 1),
				Point((i + bins + 1)*scale - 1, histHeight - intensity_green),
				Scalar(0, 255, 0));

			//������ɫ������ֱ��ͼ
			rectangle(histImage, Point((i + bins * 2)*scale, histHeight - 1),
				Point((i + bins * 2 + 1)*scale - 1, histHeight - intensity_blue),
				Scalar(0, 0, 255));

		}

		//�ڴ�������ʾ�����ƺõ�ֱ��ͼ
		imshow("ͼ���RGBֱ��ͼ", histImage);
		waitKey(0);
		*/







