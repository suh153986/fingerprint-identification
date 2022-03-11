#pragma once

/*
********获取指纹的特征点：中心点，三角点，交叉点，端点*********
*/
#include <stdbool.h>

#define PI 3.1415926
#define EPI	57.29578

#define     MAX_MINUTIANUM			60			// 最大特征点数
#define		MAX_SINGULARYNUM		30

#define     MAXMINUTIANUM			60			// 最大特征点数
#define     MAXRAWMINUTIANUM        100			// 包含虚假特征点的初始特征点最大数目
#define		VF_MINUTIA_END        1	// 端点
#define		VF_MINUTIA_FORK       2	// 叉点
#define		VF_MINUTIA_CORE		  3 //中心点
#define		VF_MINUTIA_DELTA	  4//三角点


//定义指纹特征点结构
typedef struct tagMinutiae {
	int    x;				// 横坐标
	int    y;				// 纵坐标
	int    Direction;		// 方向
	int	Triangle[3];	// 特征点为中心外接圆半径为定值的正三角形三个顶点的方向
	int    Type;			// 类型
} MINUTIA, *MINUTIAPTR;

// 指纹特征(模板)结构
typedef struct tagCDFeature {
	int		MinutiaNum;					// 特征点数
	MINUTIA		MinutiaArr[MAX_MINUTIANUM];	// 特征点数组
} FEATURE_CD, *FEATUREPTR_CD;//C：叉点，D：端点

							 // 指纹特征(模板)结构
typedef struct tagZSFeature {
	int		MinutiaNum;					// 特征点数
	MINUTIA		MinutiaArr[5];	// 特征点数组最多为5个，一个手指指纹假设最多为5个，实际上只有1~2个
} FEATURE_ZS, *FEATUREPTR_ZS;//Z:中心点，S:三角点


typedef struct DbPoint
{
	double x;
	double y;
} DBLPOINT;


extern int ImageWidth;
extern int ImageHeight;
extern unsigned char * MinutiaSourceImag;


int getOriChange(int angle1, int angle2, char flag);//计算方向场变化

int getSingular(unsigned char * lpOrient, unsigned char *lpSrc, FEATUREPTR_ZS lpArr, char flag);//获取奇异点：中心点和三角点
bool   IsFork(unsigned char * lpNow); //判断某点是否为叉点
bool   IsEnd(unsigned char * lpNow);//判断某点是否为端点
int  GetNext(unsigned char *lpNow, unsigned char  *lpLast, unsigned char **lppNext);//在纹线上根据当前点和前驱点地址找到下一个点的地址
int  GetByDis(unsigned char *lpEnd, unsigned char **lppPos, int d);//在纹线上找到距离当前端点为d的点的地址
int  PX(unsigned char *lpPos);//根据地址得到当前点的横坐标
int  PY(unsigned char *lpPos);//根据地址得到当前点的纵坐标
int  GetJiajiao(int angle1, int angle2);//求两个角度的夹角(0 - 90)
int  AngleAbs360(int angle1, int angle2);//求两个角度的夹角(0 - 180)
int  GetAngleDis(int angleBegin, int angleEnd);//两个角度间的距离(0-360)  (逆时针方向)
int  GetAngle(int x0, int y0, int x1, int y1);//两个点连线与x轴方向的角度
int  DIndex(int angle);//对角度进行量化，得到量化的索引号(0-12)

int  getMinutia(unsigned char *g_lpOrgFinger, unsigned char  *g_lpOrient, FEATURE_CD *feature);//指纹图像提取特征点:端点和叉点


void Draw_Square(unsigned char *lpFinger, int x, int y);//对一点的周围画正方形
void Sign_Square(unsigned char *lpFinger, FEATURE_CD feature, FEATURE_ZS centre, FEATURE_ZS triangle);//对一幅图里面的特征点和奇异点标记正方形