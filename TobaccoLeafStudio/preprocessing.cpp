#include"Header.h"

void BlurImage(Mat &r, Mat &t, Mat &dst_r, Mat &dst_t)
{
	bilateralFilter(r, dst_r, 15, 15 * 2, 15 / 2);
	bilateralFilter(t, dst_t, 15, 15 * 2, 15 / 2);
}

