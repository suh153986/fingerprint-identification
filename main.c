#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "ImageReadWrite.h"
#include "main.h"
#include "ImageProcess.h"
#include "GetFeature.h"

long long length = 0;//定义读入图像之后的长度为length
FEATURE_CD feature_cd;//特征端点和叉点
FEATURE_ZS feature_centre;//特征中心点
FEATURE_ZS feature_triangle;//特征三角点

FEATURE feature;//特征点
MATCHRESULT MatchResult;//匹配结果
VF_FLAG matchMode = 2;//匹配模式

int main(void)
{
	unsigned char * SrcImg = ImageRead("./Image/0.BMP");
	ImageWrite("./Image/0SrcImg.BMP", SrcImg);
	length = ImageWidth*ImageHeight;

	unsigned char * TempImg = (unsigned char *)malloc(length);
	unsigned char * OrientImg = (unsigned char *)malloc(length);
	unsigned char * GradImg = (unsigned char *)malloc(length);
	unsigned char * EqualizeImg = (unsigned char *)malloc(length);
	unsigned char * SrcImgSeg = (unsigned char *)malloc(length);
	unsigned char * OrientImgSeg = (unsigned char *)malloc(length);
	unsigned char * ConvergeImg = (unsigned char *)malloc(length);
	//unsigned char * SrcImgSeg = (unsigned char *)malloc(length);
	//RectifyFunc(SrcImg, TempImg, &RectifyParm[SensorID][0]); 
	//ImageWrite("./Image/1rectify.BMP",TempImg);


	zoomout(SrcImg, TempImg);//提取低频信息
	getOrientMap(TempImg, OrientImg, 6);//方向场
	ImageWrite("./Image/2Orient.BMP", OrientImg);

	
	zoomout(SrcImg, TempImg);//提取低频信息
	getGrads(TempImg, GradImg, 6);//梯度场
	ImageWrite("./Image/3Grad.BMP", GradImg);
	
	memcpy(SrcImgSeg, SrcImg, length);
	memcpy(OrientImgSeg, OrientImg, length);

	if (0 == segment(GradImg, TempImg, 6, 80))//设定步长和阈值，将梯度场进行分割。
	{
		printf("前景（指纹区域）太小，无法分割，请重新调整！\n");
		exit(-1);
	}
	else
	{
		ImageWrite("./Image/4GradImgsegment.BMP", GradImg);
		segment_clearEdge(SrcImgSeg, OrientImgSeg, GradImg);//按照梯度场对原始图像和方向场图像进行背景清除
		ImageWrite("./Image/5SrcImgsegment.BMP", SrcImgSeg);
		ImageWrite("./Image/6OrientImgsegment.BMP", OrientImgSeg);
	}
	
	equalize(SrcImgSeg, SrcImg);//图像灰度均衡, 输入为背景清除之后的原图像指针
	ImageWrite("./Image/7equalize.BMP", SrcImg);

	//将均衡后的图像当做收敛操作的输入数据
	GaussSmooth(SrcImg, ConvergeImg, 0.4);//收敛:利用高斯函数进行收敛，即固定一点的灰度，防止发散
	ImageWrite("./Image/8Converge.BMP", ConvergeImg);


	memcpy(SrcImg, ConvergeImg, length);
	orientEnhance(OrientImg, SrcImg);//利用方向场来增强图像
	ImageWrite("./Image/9Enhance.BMP", SrcImg);
	
	binary(SrcImg, TempImg, OrientImg);//二值化
	ImageWrite("./Image/10Binary.BMP", SrcImg);

	binaryClear(SrcImg, TempImg, GradImg);//清除二值化带来的噪声
	ImageWrite("./Image/11Binaryclear.BMP", SrcImg);

	
	imageThin(SrcImg, TempImg);//指纹细化，每一条指纹由一个一个点连接起来
	ImageWrite("./Image/12Thin.BMP", SrcImg);
	
	thinClear(SrcImg, 12);//清除细化后的短棒：指纹长度太小；毛刺：指纹中一些污点
	ImageWrite("./Image/13Thinclear.BMP", SrcImg);


	
	if (getMinutia(SrcImg, OrientImgSeg, &feature_cd))//细节特征点：端点和叉点提取
	{
		printf("总数已经超过允许最大数目");
		exit(-1);
	}
	getSingular(OrientImgSeg, SrcImg, &feature_triangle, 1);//提取三角点
	getSingular(OrientImgSeg, SrcImg, &feature_centre, -1);//提取中心点
	Sign_Square(SrcImg, feature_cd, feature_centre, feature_triangle);//标记特征点
	ImageWrite("./Image/14Signimage.BMP", SrcImg);

	
	feature = GetFeature(feature_cd, feature_centre, feature_triangle);
	patternMatch(&feature, &feature, &MatchResult, matchMode);//指纹匹配

	if (MatchResult.Similarity > 60)
	{
		printf("Match success!\n");
	}
	else
	{
		printf("Match failure!\n");
	}

	free(SrcImg);
	free(TempImg);
	free(OrientImg);
	free(GradImg);
	free(EqualizeImg);
	free(SrcImgSeg);
	free(OrientImgSeg);
	free(ConvergeImg);

	return 0;
}
