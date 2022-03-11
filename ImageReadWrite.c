#include "ImageReadWrite.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int ImageWidth;
int ImageHeight;
int ImageBitCount;

RGBQUAD *pColorTable;

/*
************读入图像***********
ImageName读入图像路径
*/
unsigned char * ImageRead(char *ImageName)
{
	int lineByte = 0;
	FILE * fp = fopen(ImageName, "rb");
	int i = 0, j = 0;

	BITMAPINFOHEADER head;
	unsigned char *pImageBuf;

	if (fp == NULL)
	{
		exit(-1);
		printf("error");
	}
	else
	{
		fseek(fp, sizeof(BITMAPFILEHEADER), SEEK_SET);
		fread(&head, sizeof(BITMAPINFOHEADER), 1, fp);
	}

	ImageWidth = head.biWidth;
	ImageHeight = head.biHeight;
	ImageBitCount = head.biBitCount;

	lineByte = (ImageWidth*ImageBitCount / 8 + 3) / 4 * 4;

	if (ImageBitCount == 8)
	{
		pColorTable = (RGBQUAD *)malloc(sizeof(RGBQUAD) * 256);
		fread(pColorTable, sizeof(RGBQUAD), 256, fp);
	}

	pImageBuf = (unsigned char *)malloc(sizeof(unsigned char)*lineByte*ImageHeight);
	fread(pImageBuf, 1, lineByte*ImageHeight, fp);


	fclose(fp);
	return pImageBuf;

}

/*
************写出图像***********
ImageName写出图像路径
*/
void ImageWrite(char *ImageName, unsigned char * imgBuf)
{
	if (!imgBuf)
		return;
	//颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0  
	int colorTablesize = 0;
	if (ImageBitCount == 8)
		colorTablesize = 1024;

	//待存储图像数据每行字节数为4的倍数  
	int lineByte = (ImageWidth * ImageBitCount / 8 + 3) / 4 * 4;
	//以二进制写的方式打开文件  

	FILE *fp = fopen(ImageName, "wb");
	if (fp == 0)
		return;
	//申请位图文件头结构变量，填写文件头信息  
	BITMAPFILEHEADER fileHead;
	fileHead.bfType = 0x4D42;//bmp类型  
							 //bfSize是图像文件4个组成部分之和  
	fileHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*ImageHeight;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	//bfOffBits是图像文件前3个部分所需空间之和  
	fileHead.bfOffBits = 54 + colorTablesize;
	//写文件头进文件  
	fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);

	//申请位图信息头结构变量，填写信息头信息  
	BITMAPINFOHEADER head;
	head.biBitCount = ImageBitCount;
	head.biClrImportant = 0;
	head.biClrUsed = 0;
	head.biCompression = 0;
	head.biHeight = ImageHeight;
	head.biPlanes = 1;
	head.biSize = 40;
	head.biSizeImage = lineByte*ImageHeight;
	head.biWidth = ImageWidth;
	head.biXPelsPerMeter = 0;
	head.biYPelsPerMeter = 0;
	//写位图信息头进内存  
	fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);
	//如果灰度图像，有颜色表，写入文件    
	if (ImageBitCount == 8)
		fwrite(pColorTable, sizeof(RGBQUAD), 256, fp);
	//写位图数据进文件  
	fwrite(imgBuf, ImageHeight*lineByte, 1, fp);
	//关闭文件  
	fclose(fp);
	return;
}

