/*
**************图像预处理*****************
*/

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#include "ImageReadWrite.h"
#include "ImageProcess.h"


unsigned char * MinutiaSourceImag;

int  g_DDSite[12][7][2] = {
	-3, 0,  -2, 0,  -1, 0,   0, 0,   1, 0,   2, 0,   3, 0,
	-3,-1,  -2,-1,  -1, 0,   0, 0,   1, 0,   2, 1,   3, 1,
	-3,-2,  -2,-1,  -1,-1,    0, 0,   1, 1,   2, 1,   3, 2,
	-3,-3,  -2,-2,  -1,-1,   0, 0,   1, 1,   2, 2,   3, 3,
	-2,-3,  -1,-2,  -1,-1,   0, 0,   1, 1,   1, 2,   2, 3,
	-1,-3,  -1,-2,   0,-1,   0, 0,   0, 1,   1, 2,   1, 3,
	0,-3,   0,-2,   0,-1,   0, 0,   0, 1,   0, 2,   0, 3,
	-1, 3,  -1, 2,   0, 1,   0, 0,   0,-1,   1,-2,   1,-3,
	-2, 3,  -1, 2,  -1, 1,   0, 0,   1,-1,   1,-2,   2,-3,
	-3, 3,  -2, 2,  -1, 1,   0, 0,   1,-1,   2,-2,   3,-3,
	-3, 2,  -2, 1,  -1, 1,   0, 0,   1,-1,   2,-1,   3,-2,
	-3, 1,  -2, 1,  -1, 0,   0, 0,   1, 0,   2,-1,   3,-1
};

int RectifyParm[8][8] = { 31,31,606,31,121,463,521,463,
25,6,609,6,104,475,538,475 };//畸形矫正参数
int SensorID = 0;


/*
****************y坐标畸变矫正*********************
//oldI为坐标，W0位梯形上底长度，W1为梯形下底长度，TH为梯形高度
*/
double computeY(double oldI, double W0, double W1, double TH)
{

	double oldTH = 15 * 1.414, u = 15;

	double RH = RSTIMGH, SH = SRCIMGH;//RSTIMGH为原图的高度，SRCIMGH为畸变图的高度

	double sina = 1.414 / 2, cosa = 1.414 / 2;

	double oY = SH * (W0 / (W0 + W1));//oY为畸变图像中心点的纵坐标

	double f = u / 2;//u - oldTH * cosa * (W1 / (W0 - W1));

	double u_f = (W1 / (W0 - W1)) * oldTH * cosa;//u-f的值

	double a_b = oldTH * sina / 2 * f * (1 / (u_f + oldTH * cosa) + 1 / u_f);//|a|+|b|的值


	double oldY = ((oldI - RH / 2) / RH) * oldTH;//矫正好后的图形单位（像素与毫米）转换

	double newY = -oldY * sina * f / (u_f + oldTH * sina / 2 - oldY * cosa);//畸变图形纵坐标newY与矫正后图形纵坐标oldY之间的转换

	double newI = newY * SH / (a_b)+oY;//畸变图形单位（像素与毫米）的转换

	newI = SH - newI;

	return newI;

}

/*
P2	  W1	P3
-------------        ----
/             \
/               \      TH
/                 \
/                   \
-----------------------   ----
P0          W0         P1

BMP图像左下角为坐标原点


P0_X       P0横坐标
P0_Y       P0纵坐标
P1_X       P1横坐标
P1_Y       P1纵坐标
P2_X       P2横坐标
P2_Y       P2纵坐标
P3_X       P3横坐标
P4_Y       P3纵坐标

*/
//矫正函数（lpSrcBitmap源图指针，lpRstBitmap矫正图指针，pParam参数指针）
void RectifyFunc(unsigned char *lpSrcBitmap, unsigned char *lpRstBitmap, int *pParam)
{

	int P0_X = pParam[0];
	int P0_Y = pParam[1];
	int P1_X = pParam[2];
	int P1_Y = pParam[3];
	int P2_X = pParam[4];
	int P2_Y = pParam[5];
	int P3_X = pParam[6];
	int P3_Y = pParam[7];
	unsigned short	YTable[RSTIMGH];//RSTIMGH=360
	int W0 = P1_X - P0_X;
	int W1 = P3_X - P2_X;
	int TH = P2_Y - P0_Y;



	int i, j;
	double oldy;
	int		iy;

	unsigned int x, y;
	short	sum;
	int y1;
	int RS;


	for (i = 0; i < RSTIMGH; i++)//0--360
	{
		oldy = computeY(i, W0, W1, TH);
		iy = (int)oldy;
		YTable[i] = iy;
	}


	for (i = 0; i < RSTIMGH; i++)
	{
		for (j = 0; j < RSTIMGW; j++)
		{
			y = YTable[i];//特定的y计算x
			oldy = SRCIMGH - y;
			RS = (int)(W1 + (W0 - W1)*(TH - oldy + 0.0) / SRCIMGH);
			x = (int)((W0 - RS) / 2 + (j + 0.0) / RSTIMGW*RS);
			//x = j * (SRCIMGW - 100) / RSTIMGW + 50;
			//y = YTable[i];
			if (y == 0)
			{
				y1 = y + 1;
			}
			else
			{
				y1 = y;
			}

			sum =
				*(lpSrcBitmap + (y + 1) * SRCIMGW + x - 1) + *(lpSrcBitmap + (y + 1) * SRCIMGW + x) * 2 +
				*(lpSrcBitmap + (y + 1) * SRCIMGW + x + 1) + *(lpSrcBitmap + (y)* SRCIMGW + x - 1) * 2 +
				*(lpSrcBitmap + (y)* SRCIMGW + x) * 4 + *(lpSrcBitmap + (y)* SRCIMGW + x - 1) * 2 +
				*(lpSrcBitmap + (y1 - 1) * SRCIMGW + x - 1) + *(lpSrcBitmap + (y1 - 1) * SRCIMGW + x) * 2 +
				*(lpSrcBitmap + (y1 - 1) * SRCIMGW + x + 1);

			sum = (sum >> 4);
			*(lpRstBitmap + i* RSTIMGW + RSTIMGW - j - 1) = (unsigned char)sum; //[i][RSTIMGW-j-1]
		}
	}
}



/*
*******得到图像的方向场*******

g_lpTemp:输入图像指针，g_lpOrient：输出方向场指针，r：输入，点为中心，r为长度的范围进行方向场计算
*/


void getOrientMap(unsigned char* g_lpTemp, unsigned char* g_lpOrient, int  r)
{
	long  x, y, i, j;
	long  vx, vy, lvx, lvy;
	unsigned char   *lpSrc = NULL;
	unsigned char   *lpOri = NULL;
	long	angle, num;
	double  fAngle;

	int IMGW = ImageWidth;
	int IMGH = ImageHeight;

	for (y = 0; y < IMGH / 2; y++)
	{
		for (x = 0; x < IMGW / 2; x++)
		{
			lpOri = g_lpOrient + 2 * y * IMGW + 2 * x;
			lvx = 0;
			lvy = 0;
			num = 0;
			for (i = -r; i <= r; i++)	// 为提高速度，步长为
			{
				if (y + i<1 || y + i >= IMGH / 2 - 1) continue;
				for (j = -r; j <= r; j++)	// 为提高速度，步长为
				{
					if (x + j<1 || x + j >= IMGW / 2 - 1) continue;
					lpSrc = g_lpTemp + (y + i)*(IMGW / 2) + x + j;
					//求x方向偏导
					vx = *(lpSrc + IMGW / 2 + 1) - *(lpSrc + IMGW / 2 - 1) +
						*(lpSrc + 1) * 2 - *(lpSrc - 1) * 2 +
						*(lpSrc - IMGW / 2 + 1) - *(lpSrc - IMGW / 2 - 1);
					//求y方向偏导
					vy = *(lpSrc + IMGW / 2 - 1) - *(lpSrc - IMGW / 2 - 1) +
						*(lpSrc + IMGW / 2) * 2 - *(lpSrc - IMGW / 2) * 2 +
						*(lpSrc + IMGW / 2 + 1) - *(lpSrc - IMGW / 2 + 1);

					lvx += vx * vy * 2;//sin(2sita)
					lvy += vx*vx - vy*vy;//cos(2sita)
					num++;
				}
			}

			if (num == 0) num = 1;

			// 求弧度
			fAngle = atan2(lvy, lvx);
			// 变换到(0 - 2*pi)
			if (fAngle < 0)	fAngle += 2 * PI;

			// 求纹线角度
			fAngle = (fAngle*EPI*0.5 + 0.5);
			angle = (long)fAngle;

			// 因为采用sobel算子，所以角度偏转了度，所以要旋转求得的角度
			angle -= 135;
			// 角度变换到（-180）
			if (angle <= 0)	angle += 180;

			angle = 180 - angle;
			// 最终纹线角度
			*lpOri = (unsigned char)angle;
			*(lpOri + 1) = (unsigned char)angle;
			*(lpOri + IMGW) = (unsigned char)angle;
			*(lpOri + IMGW + 1) = (unsigned char)angle;

		}
	}
}

/*
************梯度场**************
g_lpTemp输入图像指针，g_lpDivide输出梯度场指针，r为点的周围
*/
void getGrads(unsigned char* g_lpTemp, unsigned char* g_lpDivide, int r)
{
	long  x, y, i, j;
	long  vx, vy, lvx, lvy;
	unsigned char   *lpSrc = NULL;
	unsigned char   *lpDiv = NULL;
	long	num;
	int gradSum;
	int grad;

	int IMGW = ImageWidth;
	int IMGH = ImageHeight;


	for (y = 0; y < IMGH / 2; y++)
	{
		for (x = 0; x < IMGW / 2; x++)
		{
			lpDiv = g_lpDivide + 2 * y*IMGW + 2 * x;
			lvx = 0;
			lvy = 0;
			num = 0;
			gradSum = 0;
			for (i = -r; i <= r; i++)	// 为提高速度，步长为
			{
				if (y + i<1 || y + i >= IMGH / 2 - 1) continue;
				for (j = -r; j <= r; j++)	// 为提高速度，步长为
				{
					if (x + j<1 || x + j >= IMGW / 2 - 1) continue;
					lpSrc = g_lpTemp + (y + i)*(IMGW / 2) + x + j;
					//求x方向偏导
					vx = *(lpSrc + IMGW / 2 + 1) - *(lpSrc + IMGW / 2 - 1) +
						*(lpSrc + 1) * 2 - *(lpSrc - 1) * 2 +
						*(lpSrc - IMGW / 2 + 1) - *(lpSrc - IMGW / 2 - 1);
					//求y方向偏导
					vy = *(lpSrc + IMGW / 2 - 1) - *(lpSrc - IMGW / 2 - 1) +
						*(lpSrc + IMGW / 2) * 2 - *(lpSrc - IMGW / 2) * 2 +
						*(lpSrc + IMGW / 2 + 1) - *(lpSrc - IMGW / 2 + 1);

					gradSum += (abs(vx) + abs(vy));
					num++;
				}
			}

			if (num == 0)
				num = 1;
			// 求幅值，保存到g_lpDivide中，用于分割前景背景
			grad = gradSum / num;

			if (grad > 255)
				grad = 255;
			*lpDiv = (unsigned char)grad;
			*(lpDiv + 1) = (unsigned char)grad;
			*(lpDiv + IMGW) = (unsigned char)grad;
			*(lpDiv + IMGW + 1) = (unsigned char)grad;



		}
	}

}

/*
********** 放大缩小函数：提取低频信息**********
g_lpOrgFinger输入图像指针，g_lpTemp输出提取低频信息之后的指针
*/
void zoomout(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int	x, y;
	unsigned char	*lpSrc;
	unsigned char	*lpRst;
	int	sum;
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };


	// 边缘部分
	for (y = 0; y < IMGH; y += 2)
	{
		lpSrc = g_lpOrgFinger + y*IMGW;
		lpRst = g_lpTemp + (y / 2)*(IMGW / 2);
		*lpRst = *lpSrc;
		lpSrc = g_lpOrgFinger + y*IMGW + IMGW - 1;
		lpRst = g_lpTemp + (y / 2)*(IMGW / 2) + (IMGW / 2 - 1);
		*lpRst = *lpSrc;
	}
	for (x = 0; x < IMGW; x += 2)
	{
		lpSrc = g_lpOrgFinger + x;
		lpRst = g_lpTemp + x / 2;
		*lpRst = *lpSrc;
		lpSrc = g_lpOrgFinger + (IMGH - 1)*IMGW + x;
		lpRst = g_lpTemp + (IMGH / 2 - 1)*(IMGW / 2) + x / 2;
		*lpRst = *lpSrc;
	}

	// 非边缘部分用高斯模板提取低频信息
	for (y = 2; y < IMGH - 2; y += 2)
	{
		for (x = 2; x < IMGW - 2; x += 2)
		{
			lpSrc = g_lpOrgFinger + y*IMGW + x;
			lpRst = g_lpTemp + (y / 2)*(IMGW / 2) + x / 2;
			sum = *lpSrc * 4 + *(lpSrc + SiteD8[0]) +
				*(lpSrc + SiteD8[1]) * 2 + *(lpSrc + SiteD8[2]) +
				*(lpSrc + SiteD8[3]) * 2 + *(lpSrc + SiteD8[4]) +
				*(lpSrc + SiteD8[5]) * 2 + *(lpSrc + SiteD8[6]) +
				*(lpSrc + SiteD8[7]) * 2;
			sum = sum >> 4;
			*lpRst = (unsigned char)sum;
		}
	}
}


////////////////////////////////////////////////////////////////////////
//	lpInBuffer: [in] 要平滑的图像数据缓冲区
//	lpOutBuffer: [out] 平滑后的图像数据缓冲区
//	r: [in] 平滑滤波器半径
//	d: [in] 平滑滤波器步长
////////////////////////////////////////////////////////////////////////
void smooth(unsigned char *lpInBuffer, unsigned char *lpOutBuffer, int IMGW, int IMGH, int r, int d)
{

	int	x, y, i, j;
	unsigned char	*lpSrc;
	unsigned char	*lpRst;
	int	sum, num;


	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			lpSrc = lpInBuffer + y*IMGW + x;
			lpRst = lpOutBuffer + y*IMGW + x;
			sum = 0;
			num = 0;
			for (i = -r; i <= r; i += d)
			{
				if (i + y<0 || i + y >= IMGH)
					continue;
				for (j = -r; j <= r; j += d)
				{
					if (j + x<0 || j + x >= IMGW)
						continue;
					sum += *(lpSrc + i*IMGW + j);
					num++;
				}
			}
			*lpRst = (unsigned char)(sum / num);
		}
	}


}


/*
**********图像分割，提取前景（指纹）和背景（杂质）**********
//g_lpDivide输入图像，g_lpTemp输出图像
//	r: [in] 对幅值图像高度平滑滤波的滤波器半径
//  threshold: [in] 分割的阈值
*/
bool segment(unsigned char *g_lpDivide, unsigned char *g_lpTemp, int r, int threshold)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int	x, y;
	int  num = 0;

	// 对方向场幅值图像进行高度平滑滤波
	smooth(g_lpDivide, g_lpTemp, IMGW, IMGH, r, 2);

	// 图像边缘均设置为背景
	for (y = 0; y < IMGH; y++)
	{
		*(g_lpDivide + y*IMGW) = 255;
		*(g_lpDivide + y*IMGW + IMGW - 1) = 255;
	}
	for (x = 0; x < IMGW; x++)
	{
		*(g_lpDivide + x) = 255;
		*(g_lpDivide + (IMGH - 1)*IMGW + x) = 255;
	}

	for (y = 1; y < IMGH - 1; y++)
	{
		for (x = 1; x < IMGW - 1; x++)
		{
			// 根据幅值与阈值大小判断是否为背景区域
			if (*(g_lpDivide + y*IMGW + x) < threshold)
			{
				*(g_lpDivide + y*IMGW + x) = 0;
			}
			else
			{
				*(g_lpDivide + y*IMGW + x) = 255;
				num++;
			}
		}
	}

	// 如果前景区域面积小于总面积的十分之一，则表示前景区域太小，返回错误
	if (num < IMGH * IMGW / 10)
		return false;
	else
		return true;

}

/*
*****************clearEdge: 清除背景***********
g_lpOrgFinger输入图像指针，g_lpOrient输入方向指针，g_lpDivide输入分割参考指针，以g_lpDivide为参考点进行分割
*/
void segment_clearEdge(unsigned char *g_lpOrgFinger, unsigned char *g_lpOrient, unsigned char* g_lpDivide)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int	x, y;
	int  temp;
	temp = 0;

	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			// 如果是背景区域，则置该点为白点
			if (*(g_lpDivide + temp + x) == 0)
			{
				*(g_lpOrgFinger + temp + x) = 255;
				*(g_lpOrient + temp + x) = 255;
			}
		}
		temp += IMGW;
	}
}

/*
*************图像均衡：图像中各种灰度场强上都均匀分布相等的参数，均衡后黑白对比更清晰**************
lpDIBBits输入图像指针，lpDataOut输出均衡后图像指针
*/
void equalize(unsigned char *lpDIBBits, unsigned char *lpDataOut)
{
	int lWidth = ImageWidth;
	int lHeight = ImageHeight;

	// 指向源图像的指针
	unsigned char *	lpSrc;
	unsigned char *	lpRst;

	// 临时变量
	int	lTemp;
	// 循环变量
	int	i;
	int	j;

	// 灰度映射表
	unsigned char	bMap[256];

	// 灰度映射表
	int	lCount[256];

	// 图像每行的字节数
	int	lLineBytes;

	// 计算图像每行的字节数
	//lLineBytes = WIDTHBYTES(lWidth * 8);
	lLineBytes = lWidth;

	// 重置计数为0
	for (i = 0; i < 256; i++)
	{
		// 清零
		lCount[i] = 0;
	}

	// 计算各个灰度值的计数
	for (i = 0; i < lHeight; i++)
	{
		for (j = 0; j < lWidth; j++)
		{
			lpSrc = (unsigned char *)lpDIBBits + lLineBytes * i + j;

			//printf("%p:%d\t",lpSrc,*lpSrc);

			// 计数加1
			lCount[*(lpSrc)]++;
		}
	}

	// 计算灰度映射表
	for (i = 0; i < 256; i++)
	{
		// 初始为0
		lTemp = 0;

		for (j = 0; j <= i; j++)
		{
			lTemp += lCount[j];
		}

		// 计算对应的新灰度值
		bMap[i] = (unsigned char)(lTemp * 255 / lHeight / lWidth);
	}
	// 每行
	for (i = 0; i < lHeight; i++)
	{		// 每列
		for (j = 0; j < lWidth; j++)
		{
			// 指向DIB第i行，第j个像素的指针
			lpSrc = (unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - i) + j;
			lpRst = (unsigned char*)lpDataOut + lLineBytes * (lHeight - 1 - i) + j;
			// 计算新的灰度值
			*lpRst = bMap[*lpRst];
		}
	}
	// 返回


}



/*************************************************************************
*
* \函数名称：
*   MakeGauss()
*
* \输入参数:
*   double sigma									        - 高斯函数的标准差
*   double **pdKernel										- 指向高斯数据数组的指针
*   int *pnWindowSize										- 数据的长度
*
* \返回值:
*   无
*
* \说明:
*   这个函数可以生成一个一维的高斯函数的数字数据，理论上高斯数据的长度应
*   该是无限长的，但是为了计算的简单和速度，实际的高斯数据只能是有限长的
*   pnWindowSize就是数据长度
*
*************************************************************************
*/
void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize)
{
	int i;// 循环控制变量	
	int nCenter;// 数组的中心点
	double  dDis;// 数组的某一点到中心点的距离	
	double  dValue;// 中间变量
	double  dSum;
	dSum = 0;
	*pnWindowSize = (int)(1 + 2 * ceil(3 * sigma));	// 数组长度，根据概率论的知识，选取[-3*sigma, 3*sigma]以内的数据。
													// 这些数据会覆盖绝大部分的滤波系数	
	nCenter = (*pnWindowSize) / 2;// 中心
	*pdKernel = (double *)malloc(sizeof(double)*(*pnWindowSize));// 分配内存

	for (i = 0; i< (*pnWindowSize); i++)
	{
		dDis = (double)(i - nCenter);
		dValue = exp(-(1 / 2)*dDis*dDis / (sigma*sigma)) / (sqrt(2 * PI) * sigma);
		(*pdKernel)[i] = dValue;
		dSum += dValue;
	}

	// 归一化
	for (i = 0; i<(*pnWindowSize); i++)
	{
		(*pdKernel)[i] /= dSum;
	}
}


/*
************高斯函数对图像进行平滑处理*************
pUnchImg输入图像指针，pUnchSmthdImg输出图像指针：输出高斯平滑后的图像，从而使图像像素点收敛于一定范围内
*/

void GaussSmooth(unsigned char *pUnchImg, unsigned char *pUnchSmthdImg, double sigma)
{
	int nWidth = ImageWidth;
	int nHeight = ImageHeight;

	int y;
	int x;
	int i;
	int nWindowSize;// 高斯滤波器的数组长度
	int	nHalfLen;//  窗口长度的1/2
	double *pdKernel;// 一维高斯数据滤波器	
	double  dDotMul;// 高斯系数与图像数据的点乘	
	double  dWeightSum;// 高斯滤波系数的总和	
	double * pdTmp;// 中间变量

	pdTmp = (double *)malloc(sizeof(double)*nWidth*nHeight);// 分配内存

	MakeGauss(sigma, &pdKernel, &nWindowSize); // 产生一维高斯数据滤波器
	nHalfLen = nWindowSize / 2;// MakeGauss返回窗口的长度，利用此变量计算窗口的半长

	for (y = 0; y<nHeight; y++)// x方向进行滤波
	{
		for (x = 0; x<nWidth; x++)
		{
			dDotMul = 0;
			dWeightSum = 0;
			for (i = (-nHalfLen); i <= nHalfLen; i++)
			{
				// 判断是否在图像内部
				if ((i + x) >= 0 && (i + x) < nWidth)
				{
					dDotMul += (double)pUnchImg[y*nWidth + (i + x)] * pdKernel[nHalfLen + i];
					dWeightSum += pdKernel[nHalfLen + i];
				}
			}
			pdTmp[y*nWidth + x] = dDotMul / dWeightSum;
		}
	}

	// y方向进行滤波
	for (x = 0; x<nWidth; x++)
	{
		for (y = 0; y<nHeight; y++)
		{
			dDotMul = 0;
			dWeightSum = 0;
			for (i = (-nHalfLen); i <= nHalfLen; i++)
			{
				// 判断是否在图像内部
				if ((i + y) >= 0 && (i + y) < nHeight)
				{
					dDotMul += (double)pdTmp[(y + i)*nWidth + x] * pdKernel[nHalfLen + i];
					dWeightSum += pdKernel[nHalfLen + i];
				}
			}
			pUnchSmthdImg[y*nWidth + x] = (unsigned char)(int)(dDotMul / dWeightSum);
		}
	}

	// 释放内存
	free(pdKernel);
	pdKernel = NULL;
	free(pdTmp);
	pdTmp = NULL;
}



int DDIndex(int angle)
{
	/////////////////////////////////////////////////////////////////////////
	//	angle: [in] 角度 （0 - 180）
	/////////////////////////////////////////////////////////////////////////
	if (angle >= 173 || angle < 8)
	{
		return 0;
	}
	else
	{
		return ((angle - 8) / 15 + 1);
	}
}

/*
**************方向增强：利用图像中的方向场来增强图像************
g_lpOrient输入指针：方向场，g_lpOrgFinger输入图像指针
*/

void orientEnhance(unsigned char *g_lpOrient, unsigned char *g_lpOrgFinger)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int x, y;
	int i;
	int d = 0;
	int sum = 0;
	// 纹线方向上进行平滑滤波的平滑滤波器
	int Hw[7] = { 1, 1, 1, 1, 1, 1, 1 };
	// 纹线方向的垂直方向上进行锐化滤波的锐化滤波器
	int Vw[7] = { -3, -1, 3, 9, 3, -1, -3 };
	int hsum = 0;
	int vsum = 0;
	int temp = 0;
	unsigned char  *lpSrc = NULL;
	unsigned char  *lpDir = NULL;

	unsigned char *g_lpTemp = (unsigned char *)malloc(IMGW * IMGH);

	// 纹线方向上进行平滑滤波
	temp = 0;
	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			lpDir = g_lpOrient + temp + x;
			lpSrc = g_lpOrgFinger + temp + x;
			// 纹线方向的索引
			d = DDIndex(*lpDir);
			sum = 0;
			hsum = 0;
			for (i = 0; i < 7; i++)
			{
				if (y + g_DDSite[d][i][1] < 0 || y + g_DDSite[d][i][1] >= IMGH ||
					x + g_DDSite[d][i][0] < 0 || x + g_DDSite[d][i][0] >= IMGW)
				{
					continue;
				}
				sum += Hw[i] * (*(lpSrc + g_DDSite[d][i][1] * IMGW + g_DDSite[d][i][0]));
				hsum += Hw[i];
			}
			if (hsum != 0)
			{
				*(g_lpTemp + temp + x) = (unsigned char)(sum / hsum);
			}
			else
			{
				*(g_lpTemp + temp + x) = 255;
			}
		}
		temp += IMGW;
	}

	// 纹线方向的垂直方向上进行锐化滤波
	temp = 0;
	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			lpDir = g_lpOrient + temp + x;
			lpSrc = g_lpTemp + temp + x;

			// 纹线方向的垂直方向的索引
			d = (DDIndex(*lpDir) + 6) % 12;

			sum = 0;
			vsum = 0;
			for (i = 0; i < 7; i++)
			{
				if (y + g_DDSite[d][i][1] < 0 || y + g_DDSite[d][i][1] >= IMGH ||
					x + g_DDSite[d][i][0] < 0 || x + g_DDSite[d][i][0] >= IMGW)
				{
					continue;
				}
				sum += Vw[i] * (*(lpSrc + g_DDSite[d][i][1] * IMGW + g_DDSite[d][i][0]));
				vsum += Vw[i];
			}
			if (vsum > 0)
			{
				sum /= vsum;
				if (sum > 255)
				{
					*(g_lpOrgFinger + temp + x) = 255;
				}
				else if (sum < 0)
				{
					*(g_lpOrgFinger + temp + x) = 0;
				}
				else
				{
					*(g_lpOrgFinger + temp + x) = (unsigned char)sum;
				}
			}
			else
			{
				*(g_lpOrgFinger + temp + x) = 255;
			}
		}
		temp += IMGW;
	}

}



/*
***********二值化：指纹图像里面数据要么为0（白），要么为255（黑）*************
g_lpOrgFinger输入图像指针，g_lpTemp输入数据指针，g_lpOrient输入方向场指针
*/
//二值化
void binary(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp, unsigned char *g_lpOrient)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int x, y;
	int i;
	int d = 0;
	int sum = 0;
	// 纹线方向上的7个点的权值
	int Hw[7] = { 2, 2, 3, 4, 3, 2, 2 };
	// 纹线方向的垂直方向上的7个点的权值
	int Vw[7] = { 1, 1, 1, 1, 1, 1, 1 };
	int hsum = 0;	// 纹线方向上的7个点的加权和
	int vsum = 0;	// 纹线方向的垂直方向上的7个点的加权和
	int Hv = 0;		// 纹线方向上的7个点的加权平均值
	int Vv = 0;		// 纹线方向的垂直方向上的7个点的加权平均值
	int temp = 0;
	unsigned char  *lpSrc = NULL;	// 指纹图像像素点指针
	unsigned char  *lpDir = NULL;	// 纹线方向指针

	temp = 0;
	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			lpDir = g_lpOrient + temp + x;
			lpSrc = g_lpOrgFinger + temp + x;

			// 如果该点非常黑，则在临时缓冲区内置该点为黑点，值为0
			if (*lpSrc < 4)
			{
				*(g_lpTemp + temp + x) = 0;
				continue;
			}
			// 计算方向索引（量化为12个方向）
			d = DDIndex(*lpDir);

			// 计算当前点在纹线方向上的加权平均值
			sum = 0;
			hsum = 0;
			for (i = 0; i < 7; i++)
			{
				// 坐标是否越界
				if (y + g_DDSite[d][i][1] < 0 || y + g_DDSite[d][i][1] >= IMGH ||
					x + g_DDSite[d][i][0] < 0 || x + g_DDSite[d][i][0] >= IMGW)
				{
					continue;
				}
				sum += Hw[i] * (*(lpSrc + g_DDSite[d][i][1] * IMGW + g_DDSite[d][i][0]));
				hsum += Hw[i];
			}
			if (hsum != 0)
			{
				Hv = sum / hsum;
			}
			else
			{
				Hv = 255;
			}

			// 纹线方向的垂直方向的索引
			d = (d + 6) % 12;

			// 计算当前点在纹线方向的垂直方向上的加权平均值
			sum = 0;
			vsum = 0;
			for (i = 0; i < 7; i++)
			{
				if (y + g_DDSite[d][i][1] < 0 || y + g_DDSite[d][i][1] >= IMGH ||
					x + g_DDSite[d][i][0] < 0 || x + g_DDSite[d][i][0] >= IMGW)
				{
					continue;
				}
				sum += Vw[i] * (*(lpSrc + g_DDSite[d][i][1] * IMGW + g_DDSite[d][i][0]));
				vsum += Vw[i];
			}
			if (vsum != 0)
			{
				Vv = sum / vsum;
			}
			else
			{
				Vv = 255;
			}


			if (Hv < Vv)
			{
				// 纹线方向上加权平均值较小则置当前点为黑点
				*(g_lpTemp + temp + x) = 0;
			}
			else
			{
				// 纹线方向上加权平均值较大则置当前点为白点
				*(g_lpTemp + temp + x) = 255;
			}
		}
		temp += IMGW;
	}

	// 将临时缓冲区内数据拷贝到原始图像数据缓冲区
	memcpy((void *)g_lpOrgFinger, (void *)g_lpTemp, ImageWidth*ImageHeight);

	return;

}

/*
************清除二值化之后的噪声***********
g_lpOrgFinger输入图像指针，g_lpTemp输入图像指针，g_lpDivide输入参考指针，以g_lpDivide为参考进行二值化之后的噪声清除
*/
void binaryClear(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp, unsigned char *g_lpDivide)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	int x, y, i;
	int num = 0;
	bool   bWorking;
	int temp;
	unsigned char  *lpSrc = NULL;
	bWorking = true;
	int  n = 0;
	// 某点周围8个点的地址偏移
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };

	// 初始化临时缓冲区
	memset((void *)g_lpTemp, 0xFF, IMGW * IMGH);

	// 循环处理直到处理完毕或者处理次数超过8
	while (bWorking && n < 8)
	{
		bWorking = false;
		n++;
		temp = IMGW;
		for (y = 1; y < IMGH - 1; y++)
		{
			for (x = 1; x < IMGW - 1; x++)
			{
				// 背景的点不处理
				if (*(g_lpDivide + temp + x) == 0)
				{
					continue;
				}
				// 统计当前点周围与它相同类型点的个数
				num = 0;
				lpSrc = g_lpOrgFinger + temp + x;
				for (i = 0; i < 8; i++)
				{
					if (*(lpSrc + SiteD8[i]) == *lpSrc)
					{
						num++;
					}
				}
				// 相同点个数小于二则改变当前点类型
				if (num < 2)
				{
					*(g_lpTemp + temp + x) = 255 - *lpSrc;
					bWorking = true;
				}
				else
				{
					*(g_lpTemp + temp + x) = *lpSrc;
				}

			}
			temp += IMGW;
		}
		// 将处理结果拷贝到原始图像缓冲区
		memcpy((void *)g_lpOrgFinger, (void *)g_lpTemp, ImageWidth*ImageHeight);
	}

}

/*
***************指纹图像细化：将指纹部分细化成一条条细线，即一条细线由一个个数据点连接起来***************
lpBits输入数据指针，g_lpTemp输入数据指针
*/
void imageThin(unsigned char *lpBits, unsigned char *g_lpTemp)
{
	int Width = ImageWidth;
	int Height = ImageHeight;
	/////////////////////////////////////////////////////////////////
	//	lpBits: [in, out] 要细化的图像数据缓冲区 
	//	Width: [in] 要细化的图像宽度
	//	Height: [in] 要细化的图像高度
	/////////////////////////////////////////////////////////////////
	unsigned char  erasetable[256] = {
		0,0,1,1,0,0,1,1,             1,1,0,1,1,1,0,1,
		1,1,0,0,1,1,1,1,             0,0,0,0,0,0,0,1,
		0,0,1,1,0,0,1,1,             1,1,0,1,1,1,0,1,
		1,1,0,0,1,1,1,1,             0,0,0,0,0,0,0,1,
		1,1,0,0,1,1,0,0,             0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,             0,0,0,0,0,0,0,0,
		1,1,0,0,1,1,0,0,             1,1,0,1,1,1,0,1,
		0,0,0,0,0,0,0,0,             0,0,0,0,0,0,0,0,
		0,0,1,1,0,0,1,1,             1,1,0,1,1,1,0,1,
		1,1,0,0,1,1,1,1,             0,0,0,0,0,0,0,1,
		0,0,1,1,0,0,1,1,             1,1,0,1,1,1,0,1,
		1,1,0,0,1,1,1,1,             0,0,0,0,0,0,0,0,
		1,1,0,0,1,1,0,0,             0,0,0,0,0,0,0,0,
		1,1,0,0,1,1,1,1,             0,0,0,0,0,0,0,0,
		1,1,0,0,1,1,0,0,             1,1,0,1,1,1,0,0,
		1,1,0,0,1,1,1,0,             1,1,0,0,1,0,0,0
	};
	int		x, y;
	int      num;
	BOOL        Finished;
	unsigned char       nw, n, ne, w, e, sw, s, se;
	unsigned char       *lpPtr = NULL;
	unsigned char       *lpTempPtr = NULL;

	memcpy((void *)g_lpTemp, (void *)lpBits, Width*Height);

	//结束标志置成假
	Finished = FALSE;
	while (!Finished) { //还没有结束
						//结束标志置成假
		Finished = TRUE;
		//先进行水平方向的细化
		for (y = 1; y<Height - 1; y++)
		{ //注意为防止越界，y的范围从1到高度-2
		  //lpPtr指向原图数据，lpTempPtr指向新图数据

			lpPtr = (unsigned char *)lpBits + y*Width;
			lpTempPtr = (unsigned char *)g_lpTemp + y*Width;

			x = 1; //注意为防止越界，x的范围从1到宽度-2

			while (x<Width - 1)
			{
				if (*(lpPtr + x) == 0)
				{ //是黑点才做处理
					w = *(lpPtr + x - 1);  //左邻点
					e = *(lpPtr + x + 1);  //右邻点

					if ((w == 255) || (e == 255)) 
					{

						//如果左右两个邻居中至少有一个是白点才处理

						nw = *(lpPtr + x + Width - 1); //左上邻点

						n = *(lpPtr + x + Width); //上邻点

						ne = *(lpPtr + x + Width + 1); //右上邻点

						sw = *(lpPtr + x - Width - 1); //左下邻点

						s = *(lpPtr + x - Width); //下邻点

						se = *(lpPtr + x - Width + 1); //右下邻点

													   //计算索引

						num = nw / 255 + n / 255 * 2 + ne / 255 * 4 + w / 255 * 8 + e / 255 * 16 +
							sw / 255 * 32 + s / 255 * 64 + se / 255 * 128;

						if (erasetable[num] == 1)
						{ //经查表，可以删除

													//在原图缓冲区中将该黑点删除

							*(lpPtr + x) = 255;

							//结果图中该黑点也删除

							*(lpTempPtr + x) = 255;

							Finished = FALSE; //有改动，结束标志置成假

							x++; //水平方向跳过一个象素

						}

					}

				}

				x++; //扫描下一个象素

			}

		}

		//再进行垂直方向的细化

		for (x = 1; x<Width - 1; x++) { //注意为防止越界，x的范围从1到宽度-2

			y = 1; //注意为防止越界，y的范围从1到高度-2

			while (y<Height - 1) {

				lpPtr = lpBits + y*Width;

				lpTempPtr = g_lpTemp + y*Width;

				if (*(lpPtr + x) == 0) 
				{ //是黑点才做处理

					n = *(lpPtr + x + Width);

					s = *(lpPtr + x - Width);

					if ((n == 255) || (s == 255)) 
					{

						//如果上下两个邻居中至少有一个是白点才处理

						nw = *(lpPtr + x + Width - 1);

						ne = *(lpPtr + x + Width + 1);

						w = *(lpPtr + x - 1);

						e = *(lpPtr + x + 1);

						sw = *(lpPtr + x - Width - 1);

						se = *(lpPtr + x - Width + 1);

						//计算索引

						num = nw / 255 + n / 255 * 2 + ne / 255 * 4 + w / 255 * 8 + e / 255 * 16 +

							sw / 255 * 32 + s / 255 * 64 + se / 255 * 128;

						if (erasetable[num] == 1) 
						{ //经查表，可以删除

													//在原图缓冲区中将该黑点删除

							*(lpPtr + x) = 255;

							//结果图中该黑点也删除

							*(lpTempPtr + x) = 255;

							Finished = FALSE; //有改动，结束标志置成假

							y++;//垂直方向跳过一个象素

						}

					}

				}

				y++; //扫描下一个象素

			}

		}

	}

	memcpy((void *)lpBits, (void *)g_lpTemp, Width*Height);

	return;
}



/*
*************GetNext: 在纹线上根据当前点和前驱点地址找到下一个点的地址***********
//	lpNow : [in] 当前点的地址
//	lpLast : [in] 前驱点的地址
//	lppNext : [out] 下一个点的地址的指针
*/
int  GetNextofProcess(unsigned char *lpNow, unsigned char  *lpLast, unsigned char **lppNext)
{
	int IMGW = ImageWidth;
	int  i = 0;
	unsigned char   *lpSrc = NULL;
	int  n = 0;
	unsigned char   *Temp[8] = { NULL };
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };// 某点周围8个点的地址
	*lppNext = NULL;

	// 找到当前点周围的是黑点并且不是前驱点的黑点，保存到数组
	for (i = 0; i < 8; i++)
	{
		lpSrc = lpNow + SiteD8[i];
		if (*lpSrc == 0 && lpSrc != lpLast)
		{
			Temp[n] = lpSrc;
			n++;
		}
	}

	if (n == 0)	// 没有找到其他黑点则表示没有后继点
	{
		*lppNext = NULL;
		return 1;
	}
	else if (n == 1) // 找到了一个，则其为后继点
	{
		*lppNext = Temp[0];
		return 0;
	}
	else // 找到多个点则返回错误
	{
		*lppNext = NULL;
		return 1;
	}

	return 0;
}

/*
*************IsFork: 判断某点是否为叉点***********
lpNow : [in] 当前点的地址
//
*/

bool   IsForkofProcess(unsigned char * lpNow)
{
	int IMGW = ImageWidth;
	int i = 0, sum = 0;
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };// 某点周围8个点的地址偏移

																					 // 8个点所有相邻两个点的差的绝对值的和如果为6*255则为叉点
	for (i = 0; i < 8; i++)
	{
		sum += abs(*(lpNow + SiteD8[(i + 1) % 8]) - *(lpNow + SiteD8[i]));
	}
	if (sum == 255 * 6)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
**********thinClear: 清除细化图像中短棒和毛刺***********
g_lpOrgFinger 输入数据指针，len设定短棒长度阈值
*/
void  thinClear(unsigned char *g_lpOrgFinger, int  len)
{

	int IMGW = ImageWidth;
	int IMGH = ImageHeight;
	/////////////////////////////////////////////////////////////////
	//	len: [in] 短棒和毛刺的最大长度
	/////////////////////////////////////////////////////////////////
	int	x, y;
	int  i, n, num;
	int  temp;
	unsigned char   *Line[25];
	unsigned char   *lpNow = NULL;
	unsigned char   *lpLast = NULL;
	unsigned char   *lpNext = NULL;
	unsigned char   *tempPtr[8];
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };

	temp = 0;
	for (y = 0; y < IMGH; y++)
	{
		for (x = 0; x < IMGW; x++)
		{
			lpNow = g_lpOrgFinger + temp + x;
			if (*lpNow != 0)
			{
				continue;
			}
			Line[0] = lpNow;
			// 统计当前点的周围黑点个数
			n = 0;
			for (i = 0; i < 8; i++)
			{
				lpNext = lpNow + SiteD8[i];
				if (*lpNext == 0)
				{
					tempPtr[n] = lpNext;
					n++;
				}
			}
			// 黑点个数为零，表示当前点是孤点，置为白色
			if (n == 0)
			{
				*lpNow = 255;
				continue;
			}
			// 黑点个数为1，表示为端点
			else if (n == 1)
			{
				num = 0;
				lpLast = lpNow;
				lpNow = tempPtr[0];
				// 沿纹线跟踪len个点
				for (i = 0; i < len; i++)
				{
					// 如果遇到叉点则跳出循环
					if (IsForkofProcess(lpNow))
					{
						break;
					}
					num++;
					Line[num] = lpNow;
					if (GetNextofProcess(lpNow, lpLast, &lpNext) == 0)
					{
						lpLast = lpNow;
						lpNow = lpNext;
					}
					else // 如果遇到异常跳出循环
					{
						break;
					}

				}

				// 纹线较短，表示为短棒或者是毛刺
				if (num < len)
				{
					for (i = 0; i <= num; i++)
					{
						*Line[i] = 255;
					}
				}

			}
		}
		temp += IMGW;
	}

	MinutiaSourceImag = g_lpOrgFinger;
	return;
}
