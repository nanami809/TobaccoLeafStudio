#include"Header.h"






//ͼ���
void FillImage( Mat &t){
	for (int i = 0; i <t.rows; i++){
		for (int j = 0; j < t.cols; j++){
			Scalar temp = t.at<Vec3b>(i, j);
			if (temp[0] == 255 && temp[1] == 255 && temp[2] == 255)
				t.at<Vec3b>(i, j) = { 0, 0, 0 };
		}
	}
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
	string dst_pathname1 = "C:/Users/eva72/Desktop/��ҶCACHE/Leftcut/";
	char temp[4];
	itoa(file_num + 1, temp, 10);
	string dst_num = temp;
	imwrite(dst_pathname1 + dst_num + "��_" + file_name, r);
	imwrite(dst_pathname1 + dst_num + "͸_" + file_name, t);
	
}



//��ʾֱ��ͼ
void Showhist(Mat &srcImage){
		system("color 3F");
		Mat grayImage;
		cvtColor(srcImage, grayImage, CV_BGR2GRAY);

		//��2������׼��
		int bins = 256;
		int hist_size[] = { bins };
		float range[] = { 0, 256 };
		const float* ranges[] = { range };
		MatND grayHist,redHist, greenHist, blueHist;


		



		//��3������ֱ��ͼ�ļ��㣨��ɫ�������֣�
		int channels_r[] = { 0 };
		calcHist(&srcImage, 1, channels_r, Mat(), //��ʹ����Ĥ
			redHist, 1, hist_size, ranges,
			true, false);

		//��4������ֱ��ͼ�ļ��㣨��ɫ�������֣�
		int channels_g[] = { 1 };
		calcHist(&srcImage, 1, channels_g, Mat(), // do not use mask
			greenHist, 1, hist_size, ranges,
			true, // the histogram is uniform
			false);

		//��5������ֱ��ͼ�ļ��㣨��ɫ�������֣�
		int channels_b[] = { 2 };
		calcHist(&srcImage, 1, channels_b, Mat(), // do not use mask
			blueHist, 1, hist_size, ranges,
			true, // the histogram is uniform
			false);

		//�����ԻҶȽ���ֱ��ͼ����
		int channels_gray[] = { 0 };
		calcHist(&grayImage, 1, channels_gray, Mat(), //��ʹ����Ĥ
			grayHist, 1, hist_size, ranges,
			true, false);

		//-----------------------���Ƴ���ɫֱ��ͼ------------------------
		//����׼��
		double maxValue_red, maxValue_green, maxValue_blue,maxValue_gray;
		minMaxLoc(redHist, 0, &maxValue_red, 0, 0);
		minMaxLoc(greenHist, 0, &maxValue_green, 0, 0);
		minMaxLoc(blueHist, 0, &maxValue_blue, 0, 0);
		minMaxLoc(grayHist, 0, &maxValue_gray, 0, 0);
		int scale = 1;
		int histHeight = 256;
		Mat histImage = Mat::zeros(histHeight, bins * 4, CV_8UC3);

		//��ʽ��ʼ����
		for (int i = 0; i<bins; i++)
		{
			//����׼��
			float binValue_red = redHist.at<float>(i);
			float binValue_green = greenHist.at<float>(i);
			float binValue_blue = blueHist.at<float>(i);
			float binValue_gray = grayHist.at<float>(i);
			int intensity_red = cvRound(binValue_red*histHeight / maxValue_red);  //Ҫ���Ƶĸ߶�
			int intensity_green = cvRound(binValue_green*histHeight / maxValue_green);  //Ҫ���Ƶĸ߶�
			int intensity_blue = cvRound(binValue_blue*histHeight / maxValue_blue);  //Ҫ���Ƶĸ߶�
			int intensity_gray = cvRound(binValue_gray*histHeight / maxValue_gray);  //Ҫ���Ƶĸ߶�

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

			//���ƻ�ɫ������ֱ��ͼ
			rectangle(histImage, Point((i + bins * 3)*scale, histHeight - 1),
				Point((i + bins * 3 + 1)*scale - 1, histHeight - intensity_gray),
				Scalar(96, 96, 96));



		}

		//�ڴ�������ʾ�����ƺõ�ֱ��ͼ
		imshow("ͼ���RGBֱ��ͼ", histImage);

		waitKey(1000);
		string dst_filename = "rgbֱ��ͼ.bmp";
		WriteBMP(dst_filename, 0, histImage, histImage);
}









