#ifndef _IMAGEREADWRITE_H_
#define _IMAGEREADWRITE_H_

#pragma pack(push, 1)//把原来对齐方式设置压栈，并设新的对齐方式设置为一个字节对齐


typedef unsigned int        DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef int                 LONG;


//文件信息头BITMAPFILEHEADER
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;


//位图信息头BITMAPINFOHEADER
typedef struct tagBITMAPINFOHEADER {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;

//颜色表RGBQUAD：
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;

unsigned char * ImageRead(char *ImageName);//读入图片
void ImageWrite(char *ImageName, unsigned char * imgBuf);//写出图片

#pragma pack(pop)

#endif
