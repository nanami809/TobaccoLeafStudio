#include"Header.h"

//ͼ��ƽ��
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);
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
	Mat mask;//
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
	erode(mask, mask, element);//��ʴ�㷨 ȥ���װ��ܱ�һȦ
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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);

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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);
}


