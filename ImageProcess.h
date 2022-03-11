#pragma once

#include <stdbool.h>

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define PI 3.1415926
#define EPI	57.29578   

#define SRCIMGW	640//畸形矫正参数
#define SRCIMGH	480
#define RSTIMGW	256
#define RSTIMGH	360

extern int ImageWidth;//定义图像宽度
extern int ImageHeight;//定义图像高度


double computeY(double oldI, double W0, double W1, double TH);//畸变校正
void RectifyFunc(unsigned char *lpSrcBitmap, unsigned char *lpRstBitmap, int *pParam);//矫正函数（源图指针，矫正图指针，参数指针）

void zoomout(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp);//提取低频信息，放大缩小函数
void getOrientMap(unsigned char* g_lpTemp, unsigned char* g_lpOrient, int  r);//方向场
void getGrads(unsigned char* g_lpTemp, unsigned char* g_lpDivide, int r);//梯度场

bool segment(unsigned char *g_lpDivide, unsigned char *g_lpTemp, int r, int threshold);//分割背景和前景
void segment_clearEdge(unsigned char *g_lpOrgFinger, unsigned char *g_lpOrient, unsigned char* g_lpDivide);//清除背景，提取前景（指纹）

void equalize(unsigned char *lpDIBBits, unsigned char *lpDataOut);//图像灰度均衡

void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize);//创建高斯函数值
void GaussSmooth(unsigned char *pUnchImg, unsigned char *pUnchSmthdImg, double sigma);//利用高斯平滑处理使图像像素点收敛于一点

void orientEnhance(unsigned char *g_lpOrient, unsigned char *g_lpOrgFinger);//利用方向场来增强图像

void binary(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp, unsigned char *g_lpOrient);//二值化，图像变成黑白图片
void binaryClear(unsigned char *g_lpOrgFinger, unsigned char *g_lpTemp, unsigned char *g_lpDivide);//二值化之后噪声清除

void imageThin(unsigned char *lpBits, unsigned char *g_lpTemp);//指纹细化
void  thinClear(unsigned char *g_lpOrgFinger, int  len);//清除细化图像中短棒和毛刺


