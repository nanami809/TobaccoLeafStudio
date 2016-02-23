#include"Header.h"

//图像平滑
void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t){
	bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);
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
	Mat mask;//
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
	erode(mask, mask, element);//腐蚀算法 去掉白板周边一圈
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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);

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
	} while (sum[0] > 0.01 || sum[1] > 0.01 || sum[2] > 0.01);
}


