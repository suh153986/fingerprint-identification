﻿
#include <stdio.h>
#include <math.h>
#include "ImageMinutiaExtract.h"
#include <memory.h>



/*
***********getOriChange:计算方向场变化**********
//angle1第一个角度
//angle2第二个角度
//flag标志位：-1表示提取中心点，1表示提取三角点
*/
int getOriChange(int angle1, int angle2, char flag)
{
	int  d = angle2 - angle1;

	if (flag >= 1)
	{
		if (d < 0)
		{
			d += 10;
		}

	}
	else if (flag <= -1)
	{
		if (d > 0)
		{
			d -= 10;
		}
	}

	return d;
}

/*
***********getSingular:获取奇异点：中心点和三角点**********
//lpOrient图像方向场指针
//lpNum奇异点个数指针
//lpArr奇异点存储数组
//flag标志位(-1表示提取中心点，1表示提取三角点)
*/
int getSingular(unsigned char * lpOrient, unsigned char *lpSrc, FEATUREPTR_ZS feature_zs, char flag)
{

	int Width = ImageWidth;
	int Height = ImageHeight;
	unsigned char * lpNow = NULL;
	unsigned char * lpDis = NULL;
	unsigned char * tempPtr[8] = { NULL };
	unsigned char * disPtr[3] = { NULL };

	int	SiteU8[8] = { Width - 1, Width, Width + 1, 1, -Width + 1, -Width, -Width - 1, -1 };

	int    x, y, i;
	int    num;
	int    sum1, sum2;
	int    d, n, k, k1;
	unsigned char *pOriMap;

	int    oriV;
	int    a1, a2;
	MINUTIA  singuArr[30];

	int    value;
	double dis;
	bool   bFound = false;
	bool   fg = false;
	int    D3[8][2] = {
		-1,-1, -1,0,  -1,1,  0,1,  1,1,  1,0,  1,-1,  0,-1
	};
	int    D5[12][2] = {
		-2,-1, -2,0,  -2,1, -1,2,  0,2,  1,2, 2,1,  2,0,  2,-1, 1,-2, 0,-2, -1,-2
	};
	int    D7[24][2] = {
		-3,-3, -3,-2, -3,-1, -3,0, -3,1, -3,2, -3,3, -2,3, -1,3, 0,3, 1,3, 2,3, 3,3, 3,2, 3,1, 3,0,
		3,-1, 3,-2, 3,-3, 2,-3, 1,-3, 0,-3, -1,-3, -2,-3
	};

	int angle1, angle2, angle3;
	int a, x1, y1;
	int trilen = 16;

	num = 0;

	memset(singuArr, 0, sizeof(singuArr));				//初始化奇异点数组，清零

	for (y = 3; y < Height - 3; y++)
	{
		for (x = 3; x < Width - 3; x++)					//遍历整幅图
		{
			pOriMap = lpOrient + y*Width + x;			//获得某点的方向场指针
			oriV = *pOriMap;							//获取某点的方向场的值

			if (oriV == 255)								//若是背景区域，则跳入下一个循环
			{
				continue;
			}

			fg = false;
			for (i = 0; i < 24; i++)
			{
				if (pOriMap[D7[i][1] * Width + D7[i][0]] == 255)
				{
					fg = true;
					break;
				}
			}
			if (fg)
			{
				continue;
			}

			sum1 = 0;
			for (i = 0; i < 8; i++)
			{
				a1 = pOriMap[D3[i][1] * Width + D3[i][0]] / 24;
				a2 = pOriMap[D3[(i + 1) % 8][1] * Width + D3[(i + 1) % 8][0]] / 24;
				d = getOriChange(a1, a2, flag);

				if (abs(d) > 5)
				{
					break;
				}

				sum1 += d;
			}
			sum2 = 0;
			for (i = 0; i < 12; i++)
			{
				a1 = pOriMap[D5[i][1] * Width + D5[i][0]] / 24;
				a2 = pOriMap[D5[(i + 1) % 12][1] * Width + D5[(i + 1) % 12][0]] / 24;
				d = getOriChange(a1, a2, flag);

				if (abs(d) > 5)
				{
					break;
				}

				sum2 += d;
			}

			if (flag == -1)									//中心点
			{
				value = -10;
			}
			else if (flag == 1)								//三角点
			{
				value = 10;
			}
			//确定其为奇异点，里外两圈的方向场差相同
			if (sum2 == value && sum1 == value)
			{
				bFound = false;
				for (i = 0; i < num; i++)
				{
					dis = sqrt((double)((x - singuArr[i].x)*(x - singuArr[i].x) +
						(y - singuArr[i].y)*(y - singuArr[i].y)));
					if (dis < 4)
					{
						singuArr[i].x = (singuArr[i].x + x) / 2.0;
						singuArr[i].y = (singuArr[i].y + y) / 2.0;
						bFound = true;
						break;
					}

				}
				if (!bFound)
				{
					singuArr[num].x = x;
					singuArr[num].y = y;
					num++;
					if (num >= MAX_SINGULARYNUM)				//若奇异点数量超过最大限制，则停止寻找奇异点
					{
						feature_zs->MinutiaNum = num;//奇异点的个数
						for (i = 0; i < num; i++)								//将找到的奇异点存入输入数组
						{
							if (singuArr[i].x == 0 || singuArr[i].y == 0)
							{
								continue;
							}
							lpNow = lpSrc + (long)singuArr[i].y*Width + (long)singuArr[i].x;

							tempPtr[0] = lpNow;
							*tempPtr[0] = 255;
							n = 0;
							for (k = 0; k < 8; k++)
							{
								if (*(lpNow + SiteU8[k]) == 0)//找出该点相邻周围的黑点
								{
									tempPtr[n + 1] = lpNow + SiteU8[k];
									*tempPtr[n + 1] = 255;
									n++;
								}
							}

							if (flag == -1 && n < 3 && n >6)//至少有三个黑点,至多8个黑点,一般为3个点
							{
								continue;
							}
							if (flag == 1 && n < 2 && n > 6)//三角点周围只有两个黑点
							{
								continue;
							}

							n = n > 3 ? 3 : n;
							for (k = 0; k < n; k++)
							{
								GetByDis(tempPtr[k + 1], &disPtr[k], 8);
								for (k1 = 0; k1 < n; k1++)
								{
									*tempPtr[k1] = 0;
								}

								angle1 = GetAngle(singuArr[i].x, singuArr[i].y, PX(disPtr[k]), PY(disPtr[k]));
								angle2 = *(lpOrient + singuArr[i].y*Width + singuArr[i].x);
								angle3 = GetAngleDis(angle1, angle2);
								if (angle3 > 90 && angle3 < 270)
								{
									feature_zs->MinutiaArr[i].Direction = angle2 + 180;
								}
								else // 如果成锐角，则表示该特征点方向与该点方向场方向相同
								{
									feature_zs->MinutiaArr[i].Direction = angle2;
								}
								a = feature_zs->MinutiaArr[i].Direction / EPI + PI / 4.0;
								x1 = singuArr[i].x + (int)(trilen*cos(a) + 0.5);
								y1 = singuArr[i].y + (int)(trilen*sin(a) + 0.5);
								feature_zs->MinutiaArr[i].Triangle[k] = *(lpOrient + y1*Width + x1);

								if (k == 1 && n == 2)//只找到两个黑点，寻找第三个黑点
								{
									a = a + (PI * 2) / 3;
									x1 = singuArr[i].x + (int)(trilen*cos(a) + 0.5);
									y1 = singuArr[i].y + (int)(trilen*sin(a) + 0.5);
									feature_zs->MinutiaArr[i].Triangle[k] = *(lpOrient + y1*Width + x1);
								}
							}

							if (flag == -1)
							{
								feature_zs->MinutiaArr[i].Type = VF_MINUTIA_CORE;
							}
							else if (flag == 1)
							{
								feature_zs->MinutiaArr[i].Type = VF_MINUTIA_DELTA;
							}


							feature_zs->MinutiaArr[i].x = singuArr[i].x;
							feature_zs->MinutiaArr[i].y = singuArr[i].y;
						}

						return 0;
					}
				}

			}
		}
	}



	feature_zs->MinutiaNum = num;//奇异点的个数
	for (i = 0; i < num; i++)								//将找到的奇异点存入输入数组
	{
		if (singuArr[i].x == 0 || singuArr[i].y == 0)
		{
			continue;
		}
		lpNow = lpSrc + (long)singuArr[i].y*Width + (long)singuArr[i].x;

		tempPtr[0] = lpNow;
		*tempPtr[0] = 255;
		n = 0;
		for (k = 0; k < 8; k++)
		{
			if (*(lpNow + SiteU8[k]) == 0)//找出该点相邻周围的黑点
			{
				tempPtr[n + 1] = lpNow + SiteU8[k];
				*tempPtr[n + 1] = 255;
				n++;
			}
		}

		if (flag == -1 && n < 3 && n >6)//至少有三个黑点,至多8个黑点,一般为3个点
		{
			continue;
		}
		if (flag == 1 && n < 2 && n > 6)//三角点周围只有两个黑点
		{
			continue;
		}

		n = n >= 3 ? 3 : n;//限定一个特征点的方向为三个
		for (k = 0; k < n; k++)
		{
			GetByDis(tempPtr[k + 1], &disPtr[k], 8);

			angle1 = GetAngle(singuArr[i].x, singuArr[i].y, PX(disPtr[k]), PY(disPtr[k]));
			angle2 = *(lpOrient + singuArr[i].y*Width + singuArr[i].x);
			angle3 = GetAngleDis(angle1, angle2);
			if (angle3 > 90 && angle3 < 270)
			{
				feature_zs->MinutiaArr[i].Direction = angle2 + 180;
			}
			else // 如果成锐角，则表示该特征点方向与该点方向场方向相同
			{
				feature_zs->MinutiaArr[i].Direction = angle2;
			}
			a = feature_zs->MinutiaArr[i].Direction / EPI + PI / 4.0;
			x1 = singuArr[i].x + (int)(trilen*cos(a) + 0.5);
			y1 = singuArr[i].y + (int)(trilen*sin(a) + 0.5);
			feature_zs->MinutiaArr[i].Triangle[k] = *(lpOrient + y1*Width + x1);

			if (k == 1 && n == 2)//只找到两个黑点，寻找第三个黑点
			{
				a = a + (PI * 2) / 3;
				x1 = singuArr[i].x + (int)(trilen*cos(a) + 0.5);
				y1 = singuArr[i].y + (int)(trilen*sin(a) + 0.5);
				feature_zs->MinutiaArr[i].Triangle[k] = *(lpOrient + y1*Width + x1);
			}
		}
		for (k1 = 0; k1 < n + 1; k1++)
		{
			*(tempPtr[k1]) = 0;
		}

		if (flag == -1)
		{
			feature_zs->MinutiaArr[i].Type = VF_MINUTIA_CORE;
		}
		else if (flag == 1)
		{
			feature_zs->MinutiaArr[i].Type = VF_MINUTIA_DELTA;
		}
		feature_zs->MinutiaArr[i].x = singuArr[i].x;
		feature_zs->MinutiaArr[i].y = singuArr[i].y;
	}


	return 0;
}



/*
*************IsFork: 判断某点是否为叉点***********
lpNow : [in] 当前点的地址
//
*/

bool   IsFork(unsigned char * lpNow)
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
*************IsEnd: 判断某点是否为端点***********
lpNow : [in] 当前点的地址
*/

bool   IsEnd(unsigned char * lpNow)
{
	int IMGW = ImageWidth;
	int i = 0, sum = 0;
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };// 某点周围8个点的地址偏移

																					 // 8个点所有相邻两个点的差的绝对值的和如果为2*255则为端点
	for (i = 0; i < 8; i++)
	{
		sum += abs(*(lpNow + SiteD8[(i + 1) % 8]) - *(lpNow + SiteD8[i]));
	}
	if (sum == 255 * 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
*************GetNext: 在纹线上根据当前点和前驱点地址找到下一个点的地址***********
//	lpNow : [in] 当前点的地址
//	lpLast : [in] 前驱点的地址
//	lppNext : [out] 下一个点的地址的指针
*/
int  GetNext(unsigned char *lpNow, unsigned char  *lpLast, unsigned char **lppNext)
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


int  GetByDis(unsigned char *lpEnd, unsigned char **lppPos, int d)
{
	int IMGW = ImageWidth;
	int  i = 0, n = 0;
	unsigned char   *lpNow = NULL;
	unsigned char   *lpLast = NULL;
	unsigned char   *lpNext = NULL;
	int	SiteD8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };

	*lppPos = NULL;

	// 找到当前点周围的是黑点并且不是前驱点的黑点，保存到数组
	for (i = 0; i < 8; i++)
	{
		if (*(lpEnd + SiteD8[i]) == 0)
		{
			lpNow = lpEnd + SiteD8[i];
			n++;
		}
	}
	if (n != 1) // 数目如果不是1则返回错误
	{
		return 1;
	}
	// 找到的黑点地址为lpNow
	// 端点作为前驱点,地址保存到lpLast
	lpLast = lpEnd;
	// 循环找d次后继点
	for (i = 0; i < d; i++)
	{
		// 如果是叉点则跳出循环
		if (IsFork(lpNow))
		{
			break;
		}
		// 成功找到下一个点则更改当前点和前驱点
		if (GetNext(lpNow, lpLast, &lpNext) == 0)
		{
			lpLast = lpNow;
			lpNow = lpNext;
		}
		else
		{
			break;
		}

	}
	// 如果找到的点为叉点，则返回其前驱点
	if (IsFork(lpNow))
	{
		*lppPos = lpLast;
	}
	else	// 否则返回当前点
	{
		*lppPos = lpNow;
	}

	return 0;
}

/*
**************PX: 根据地址得到当前点的横坐标****************
lpPos: [in] 当前点地址
*/

int  PX(unsigned char *lpPos)
{
	int IMGW = ImageWidth;
	return ((lpPos - MinutiaSourceImag) % IMGW);
}

/*
**************PY: 根据地址得到当前点的纵坐标****************
lpPos: [in] 当前点地址
*/
int  PY(unsigned char *lpPos)
{
	int IMGW = ImageWidth;
	return ((lpPos - MinutiaSourceImag) / IMGW);
}

/*
**************GetJiajiao: 求两个角度的夹角(0 - 90)****************
//	angle1: [in] 角度一 （0 - 180）
//	angle2: [in] 角度二 （0 - 180）
*/
int  GetJiajiao(int angle1, int angle2)
{
	int  a = abs(angle1 - angle2);

	if (a > 90)
	{
		a = 180 - a;
	}
	return a;
}

/*
**************AngleAbs360: 求两个角度的夹角(0 - 180)****************
//	angle1: [in] 角度一 （0 - 360）
//	angle2: [in] 角度二 （0 - 360）
*/
int  AngleAbs360(int angle1, int angle2)
{
	int a = abs(angle1 - angle2);

	if (a > 180)
	{
		return (360 - a);
	}
	else
	{
		return a;
	}
}

/*
**************GetAngleDis： 两个角度间的距离(0-360)  (逆时针方向)****************
//	angleBegin: [in] 起始角度 （0 - 360）
//	angleEnd: [in] 终结角度 （0 - 360）
*/
int  GetAngleDis(int angleBegin, int angleEnd)
{
	int  a = angleEnd - angleBegin;

	if (a < 0)
	{
		a += 360;
	}
	return a;
}

/*
**************GetAngle： 两个点连线与x轴方向的角度****************
//	x0: [in] 第一个点横坐标
//	y0: [in] 第一个点纵坐标
//	x1: [in] 第二个点横坐标
//	y1: [in] 第二个点纵坐标
*/
int  GetAngle(int x0, int y0, int x1, int y1)
{
	double angle = atan2((double)(y1 - y0), (double)(x1 - x0));

	if (angle < 0)// 弧度转化到0 - 2*PI
	{
		angle += 2 * PI;
	}

	return (int)(angle*EPI + 0.5);// 弧度转化为角度
}

/*
**************DIndex: 对角度进行量化，得到量化的索引号(0-12)****************
angle: [in] 角度 （0 - 180）
*/
int  DIndex(int angle)
{
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
***************getMinutia：指纹图像提取特征点:端点和叉点*****************
//g_lpOrgFinger输入图像指针
//g_lpOrient输入方向场指针
//feature输出特征点:端点和叉点
*/
int  getMinutia(unsigned char *g_lpOrgFinger, unsigned char  *g_lpOrient, FEATURE_CD *feature)
{
	int IMGW = ImageWidth;
	int IMGH = ImageHeight;

	FEATURE_CD *g_Feature = feature;

	int	x = 0, y = 0, i = 0, j = 0, k = 0, n = 0;
	int  temp = 0;
	int  sum = 0;
	unsigned char * lpNow = NULL;
	unsigned char * lpDis = NULL;
	int  r = 16;		// 允许特征点靠近前景边缘的最近距离
	double  d = 0.0, d1 = 0.0, d2 = 0.0, d0 = 0.0, a = 0.0;
	int  dGate = 16;
	bool    flag = false;
	int  x11, y11;
	int  x21, y21;
	int  angle1, angle2, angle3, angle4;
	unsigned char * tempPtr[8] = { NULL };
	unsigned char * disPtr[3] = { NULL };
	unsigned char * lpOri = NULL;
	int  tempForkNum = 0;
	int  tempEndNum = 0;
	int  ForkNum = 0;
	int  EndNum = 0;
	// 临时特征点数组
	MINUTIA     tempFork[MAXRAWMINUTIANUM];
	MINUTIA     tempEnd[MAXRAWMINUTIANUM];
	MINUTIA     ForkArr[MAXRAWMINUTIANUM];
	MINUTIA     EndArr[MAXRAWMINUTIANUM];
	// 某点为圆心，半径为5的圆上所有点的地址偏移
	int	SiteR5[28] = {
		-5, IMGW - 5, 2 * IMGW - 5, 3 * IMGW - 4, 4 * IMGW - 3, 5 * IMGW - 2, 5 * IMGW - 1, 5 * IMGW,
		5 * IMGW + 1, 5 * IMGW + 2, 4 * IMGW + 3, 3 * IMGW + 4, 2 * IMGW + 5, IMGW + 5, 5, -IMGW + 5,
		-2 * IMGW + 5, -3 * IMGW + 4, -4 * IMGW + 3, -5 * IMGW + 2, -5 * IMGW + 1, -5 * IMGW,
		-5 * IMGW - 1, -5 * IMGW - 2, -4 * IMGW - 3, -3 * IMGW - 4, -2 * IMGW - 5, -IMGW - 5
	};
	// 某点周围8个点的地址偏移
	int	SiteU8[8] = { IMGW - 1, IMGW, IMGW + 1, 1, -IMGW + 1, -IMGW, -IMGW - 1, -1 };

	// 初始化临时特征点数组
	memset((void *)&tempFork[0], 0, MAXRAWMINUTIANUM * sizeof(MINUTIA));
	memset((void *)&tempEnd[0], 0, MAXRAWMINUTIANUM * sizeof(MINUTIA));
	memset((void *)&ForkArr[0], 0, MAXRAWMINUTIANUM * sizeof(MINUTIA));
	memset((void *)&EndArr[0], 0, MAXRAWMINUTIANUM * sizeof(MINUTIA));


	bool    bGood = false;	// 标记阈值是否调整好，使特征点数目在允许范围内
	int  densD = 7;	// 允许两个特征点间最近的距离
	int  loopnum;	// 循环次数
	int  trilen = 16;
	// 第一遍寻找所有端点和叉点	
	ForkNum = 0;
	EndNum = 0;
	temp = 17 * IMGW;
	for (y = 17; y < IMGH - 17; y++)
	{
		for (x = 17; x < IMGW - 17; x++)
		{
			lpNow = g_lpOrgFinger + temp + x;
			lpOri = g_lpOrient + y*IMGW + x;
			// 不是黑点则不考虑
			if (*lpNow != 0)
			{
				continue;
			}
			// 是叉点
			if (IsFork(lpNow))
			{
				// 检查是否靠近边缘, 靠近边缘则不考虑
				flag = true;
				for (i = -r; i <= r && flag; i++)
				{
					for (j = -r; j <= r && flag; j++)
					{
						if (y + i<0 || y + i >= IMGH || x + j<0 || x + j >= IMGW)
						{
							continue;
						}

						if (*(g_lpOrient + temp + i*IMGW + x + j) == 255)
						{
							flag = false;
							break;
						}
					}
				}
				// 检查该点周围方向场是否变化剧烈，变化剧烈则不考虑，因为很可能是虚假点
				sum = 0;
				for (i = 0; i < 28; i++)
				{
					sum += GetJiajiao(*(lpOri + SiteR5[(i + 1) % 28]), *(lpOri + SiteR5[i]));
				}
				if (sum > 96)
					flag = false;

				// 达到所有要求则记录下来该点
				if (flag)
				{
					ForkArr[ForkNum].x = x;
					ForkArr[ForkNum].y = y;
					ForkNum++;
					// 如果总数已经超过允许最大数目，则返回错误
					if (ForkNum >= MAXRAWMINUTIANUM)
					{
						ForkNum = 0;
						return 1;
					}

				}

			}
			else if (IsEnd(lpNow))	// 如果是端点
			{
				// 检查是否靠近边缘, 靠近边缘则不考虑
				flag = true;
				for (i = -r; i <= r && flag; i++)
				{
					for (j = -r; j <= r && flag; j++)
					{
						if (y + i<0 || y + i >= IMGH || x + j<0 || x + j >= IMGW)
						{
							continue;
						}

						if (*(g_lpOrient + temp + i*IMGW + x + j) == 255)
						{
							flag = false;
							break;
						}
					}
				}

				// 检查该点周围方向场是否变化剧烈，变化剧烈则不考虑，因为很可能是虚假点
				sum = 0;
				for (i = 0; i < 28; i++)
				{
					sum += GetJiajiao(*(lpOri + SiteR5[(i + 1) % 28]), *(lpOri + SiteR5[i]));
				}
				if (sum > 96)
					flag = false;

				// 达到所有要求则记录下来该点
				if (flag)
				{
					EndArr[EndNum].x = x;
					EndArr[EndNum].y = y;
					EndNum++;
					// 如果总数已经超过允许最大数目，则返回错误
					if (EndNum >= MAXRAWMINUTIANUM)
					{
						EndNum = 0;
						return 1;
					}

				}

			}
		}
		temp += IMGW;
	}

	// 初始化临时缓冲区，准备循环调整两个特征点间允许的最小距离，使之增大到一个合适的值
	// 因为如果特征点非常密集的地方很有可能是质量很差的地方
	for (i = 0; i < MAXRAWMINUTIANUM; i++)
	{
		tempEnd[i] = EndArr[i];
	}
	for (i = 0; i < MAXRAWMINUTIANUM; i++)
	{
		tempFork[i] = ForkArr[i];
	}
	tempForkNum = ForkNum;
	tempEndNum = EndNum;

	// 循环调整
	bGood = false;
	loopnum = 0;	// 调整的次数
	while (!bGood && loopnum < 32) // 最多调整32次
	{
		loopnum++;	// 调整次数加一
					// 得到新的特征点数组
		for (i = 0; i < MAXRAWMINUTIANUM; i++)
		{
			EndArr[i] = tempEnd[i];
		}
		for (i = 0; i < MAXRAWMINUTIANUM; i++)
		{
			ForkArr[i] = tempFork[i];
		}
		// 新的特征点数
		ForkNum = tempForkNum;	// 叉点
		EndNum = tempEndNum;	// 端点

								// 去掉虚假的端点
		bGood = true;
		for (i = 0; i < EndNum - 1; i++)
		{
			flag = false;
			for (j = i + 1; j < EndNum; j++)
			{
				// 求两个端点的距离
				d = sqrt((double)((EndArr[i].x - EndArr[j].x)*(EndArr[i].x - EndArr[j].x) +
					(EndArr[i].y - EndArr[j].y)*(EndArr[i].y - EndArr[j].y)));
				// 距离足够大则检查下一个端点
				if (d > dGate && d > densD)
				{
					continue;
				}
				// 距离太小，则将其坐标置为（0，0）
				if (d <= densD)
				{
					EndArr[j].x = 0;
					EndArr[j].y = 0;
					flag = true;
					continue;
				}
				// 求第一个端点所在纹线的角度
				lpNow = g_lpOrgFinger + EndArr[i].y*IMGW + EndArr[i].x;
				if (GetByDis(lpNow, &lpDis, 8) != 0)
				{
					break;
				}
				angle1 = GetAngle(EndArr[i].x, EndArr[i].y, PX(lpDis), PY(lpDis));

				// 求第二个端点所在纹线的角度
				lpNow = g_lpOrgFinger + EndArr[j].y*IMGW + EndArr[j].x;
				if (GetByDis(lpNow, &lpDis, 8) != 0)
				{
					continue;
				}
				angle2 = GetAngle(EndArr[j].x, EndArr[j].y, PX(lpDis), PY(lpDis));

				// 求两个角度间的距离
				angle3 = GetAngleDis(angle1, angle2);

				// 如果两个角度间成锐角，则不是虚假特征点
				if (angle3 > 270 || angle3 < 90)
				{
					continue;
				}

				// 求两个端点连线的角度
				angle3 = GetAngle(EndArr[i].x, EndArr[i].y, EndArr[j].x, EndArr[j].y);
				// 求第一个端点纹线与连线的夹角
				angle3 = GetAngleDis(angle1, angle3);
				// 如果夹角较大则不是虚假点
				if (angle3 < 150 || angle3 > 210)
				{
					continue;
				}

				// 求第二个端点纹线与连线的夹角
				angle4 = GetAngle(EndArr[j].x, EndArr[j].y, EndArr[i].x, EndArr[i].y);
				angle4 = GetAngleDis(angle2, angle4);

				// 如果夹角较大则不是虚假点
				if (angle4 < 150 || angle4 > 210)
				{
					continue;
				}
				// 否则表示这两个点是同一条纹线上的断裂处的两个端点,坐标置原点
				EndArr[j].x = 0;
				EndArr[j].y = 0;
				flag = true;
			}
			if (flag)// 表示这两个点是同一条纹线上的断裂处的两个端点,坐标置原点
			{
				EndArr[i].x = 0;
				EndArr[i].y = 0;
			}
		}

		// 统计新的端点数目
		j = 0;
		for (i = 0; i < EndNum; i++)
		{
			if (EndArr[i].x == 0 || EndArr[i].y == 0)
			{
				continue;
			}
			lpNow = g_lpOrgFinger + EndArr[i].y*IMGW + EndArr[i].x;
			if (GetByDis(lpNow, &lpDis, 8) != 0)
			{
				continue;
			}

			j++;
			// 如果端点数目还是过多，则需要调整densD
			if (j >= MAXMINUTIANUM)
			{
				bGood = false;
				break;
			}

		}
		// 调整densD
		if (!bGood)
		{
			densD++; // 加一
			continue;
		}

		// 将端点装配到g_Feature
		j = 0;
		for (i = 0; i < EndNum; i++)
		{
			if (EndArr[i].x == 0 || EndArr[i].y == 0)
			{
				continue;
			}
			// 沿纹线找到第8个点
			lpNow = g_lpOrgFinger + EndArr[i].y*IMGW + EndArr[i].x;
			if (GetByDis(lpNow, &lpDis, 8) != 0)
			{
				continue;
			}
			// 求该点与端点连线的角度
			angle1 = GetAngle(EndArr[i].x, EndArr[i].y, PX(lpDis), PY(lpDis));
			angle2 = *(g_lpOrient + EndArr[i].y*IMGW + EndArr[i].x);
			angle3 = GetAngleDis(angle1, angle2);
			// 如果成钝角，则表示该特征点方向与该点方向场方向相反
			if (angle3 > 90 && angle3 < 270)
			{
				g_Feature->MinutiaArr[j].Direction = angle2 + 180;
			}
			else // 如果成锐角，则表示该特征点方向与该点方向场方向相同
			{
				g_Feature->MinutiaArr[j].Direction = angle2;
			}
			a = g_Feature->MinutiaArr[j].Direction / EPI + PI / 4.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[j].Triangle[0] = *(g_lpOrient + y11*IMGW + x11);
			a += PI * 2 / 3.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[j].Triangle[1] = *(g_lpOrient + y11*IMGW + x11);
			a += PI * 2 / 3.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[j].Triangle[2] = *(g_lpOrient + y11*IMGW + x11);

			g_Feature->MinutiaArr[j].x = EndArr[i].x;	//横坐标
			g_Feature->MinutiaArr[j].y = EndArr[i].y;	//纵坐标
			g_Feature->MinutiaArr[j].Type = VF_MINUTIA_END;	//类型

			j++;

		}

		EndNum = j;	// 端点数目


					// 去掉距离太近的叉点
		for (i = 0; i < ForkNum - 1; i++)
		{
			flag = false;
			for (j = i + 1; j < ForkNum; j++)
			{
				d = sqrt((double)((ForkArr[i].x - ForkArr[j].x)*(ForkArr[i].x - ForkArr[j].x) +
					(ForkArr[i].y - ForkArr[j].y)*(ForkArr[i].y - ForkArr[j].y)));

				if (d <= densD - 2)
				{
					ForkArr[j].x = 0;
					ForkArr[j].y = 0;
					flag = true;
				}
			}
			if (flag)
			{
				ForkArr[i].x = 0;
				ForkArr[i].y = 0;
			}
		}

		// 统计新的真正的叉点
		j = 0;
		for (i = 0; i < ForkNum; i++)
		{
			if (ForkArr[i].x == 0 || ForkArr[i].y == 0)
			{
				continue;
			}
			lpNow = g_lpOrgFinger + ForkArr[i].y*IMGW + ForkArr[i].x;

			tempPtr[0] = lpNow;
			*tempPtr[0] = 255;
			// 检查是否真正叉点
			n = 0;
			for (k = 0; k < 8; k++)
			{
				if (*(lpNow + SiteU8[k]) == 0)
				{
					tempPtr[n + 1] = lpNow + SiteU8[k];
					*tempPtr[n + 1] = 255;
					n++;
				}
			}
			for (k = 0; k < 4; k++)
			{
				*tempPtr[k] = 0;
			}

			if (n != 3)
			{
				continue;
			}
			j++;
			// 特征点总数大于允许最大数目则要调整densD
			if (EndNum + j >= MAXMINUTIANUM)
			{
				densD++;
				bGood = false;
				break;
			}

		}

		if (!bGood)
		{
			continue;
		}

		// 装配叉点到g_Feature
		j = 0;
		for (i = 0; i < ForkNum; i++)
		{
			if (ForkArr[i].x == 0 || ForkArr[i].y == 0)
			{
				continue;
			}
			lpNow = g_lpOrgFinger + ForkArr[i].y*IMGW + ForkArr[i].x;

			tempPtr[0] = lpNow;
			*tempPtr[0] = 255;
			n = 0;
			for (k = 0; k < 8; k++)
			{
				if (*(lpNow + SiteU8[k]) == 0)
				{
					tempPtr[n + 1] = lpNow + SiteU8[k];
					*tempPtr[n + 1] = 255;
					n++;
				}
			}
			if (n != 3)
			{
				continue;
			}
			// 找到叉点三个方向纹线上距离为8的点
			for (k = 0; k < 3; k++)
			{
				GetByDis(tempPtr[k + 1], &disPtr[k], 8);
			}
			for (k = 0; k < 4; k++)
			{
				*tempPtr[k] = 0;
			}
			// 求该三个点两两间的距离
			d0 = sqrt((double)((PX(disPtr[1]) - PX(disPtr[2]))*(PX(disPtr[1]) - PX(disPtr[2])) +
				(PY(disPtr[1]) - PY(disPtr[2]))*(PY(disPtr[1]) - PY(disPtr[2]))));
			d1 = sqrt((double)((PX(disPtr[0]) - PX(disPtr[2]))*(PX(disPtr[0]) - PX(disPtr[2])) +
				(PY(disPtr[0]) - PY(disPtr[2]))*(PY(disPtr[0]) - PY(disPtr[2]))));
			d2 = sqrt((double)((PX(disPtr[1]) - PX(disPtr[0]))*(PX(disPtr[1]) - PX(disPtr[0])) +
				(PY(disPtr[1]) - PY(disPtr[0]))*(PY(disPtr[1]) - PY(disPtr[0]))));

			// 距离最短的边对应的是叉线方向
			if (d0 < d1 && d0 < d2)
			{
				x11 = PX(disPtr[1]);
				y11 = PY(disPtr[1]);
				x21 = PX(disPtr[2]);
				y21 = PY(disPtr[2]);
				lpDis = disPtr[0];
			}
			else if (d1 < d0 && d1 < d2)
			{
				x11 = PX(disPtr[0]);
				y11 = PY(disPtr[0]);
				x21 = PX(disPtr[2]);
				y21 = PY(disPtr[2]);
				lpDis = disPtr[1];
			}
			else
			{
				x11 = PX(disPtr[0]);
				y11 = PY(disPtr[0]);
				x21 = PX(disPtr[1]);
				y21 = PY(disPtr[1]);
				lpDis = disPtr[2];
			}
			// 求最短边中点与叉点连线的方向与该点方向场方向的夹角
			angle1 = GetAngle(ForkArr[i].x, ForkArr[i].y, (x11 + x21) / 2, (y11 + y21) / 2);
			angle2 = *(g_lpOrient + ForkArr[i].y*IMGW + ForkArr[i].x);
			angle3 = GetAngleDis(angle1, angle2);
			// 如果成钝角，则表示该特征点方向与该点方向场方向相反
			if (angle3 > 90 && angle3 < 270)
			{
				g_Feature->MinutiaArr[EndNum + j].Direction = angle2 + 180;
			}
			else // 如果成锐角，则表示该特征点方向与该点方向场方向相同
			{
				g_Feature->MinutiaArr[EndNum + j].Direction = angle2;
			}
			// 纪录以特征点为中心外接圆半径为trilen的正三角形三个顶点的方向
			a = g_Feature->MinutiaArr[EndNum + j].Direction / EPI + PI / 4.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[EndNum + j].Triangle[0] = *(g_lpOrient + y11*IMGW + x11);
			a += PI * 2 / 3.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[EndNum + j].Triangle[1] = *(g_lpOrient + y11*IMGW + x11);
			a += PI * 2 / 3.0;
			x11 = EndArr[i].x + (int)(trilen*cos(a) + 0.5);
			y11 = EndArr[i].y + (int)(trilen*sin(a) + 0.5);
			g_Feature->MinutiaArr[EndNum + j].Triangle[2] = *(g_lpOrient + y11*IMGW + x11);

			g_Feature->MinutiaArr[EndNum + j].x = ForkArr[i].x; //横坐标
			g_Feature->MinutiaArr[EndNum + j].y = ForkArr[i].y; //纵坐标
			g_Feature->MinutiaArr[EndNum + j].Type = VF_MINUTIA_FORK; // 类型


			j++;
		}
		ForkNum = j; // 叉点数目


	}

	// 如果循环32次都不能得到理想结果，则表示图像质量差，返回错误
	if (loopnum >= 32)
	{
		EndNum = 0;
		ForkNum = 0;
		return 1;
	}
	// 特征点数目等于端点数加叉点数
	g_Feature->MinutiaNum = (unsigned char)(EndNum + ForkNum);

	//  特征点数目太少则返回错误
	if (ForkNum + EndNum < 8)
	{
		EndNum = 0;
		ForkNum = 0;
		return 1;
	}

	return 0;
}

/*
**********Draw_Square:对一点的周围画正方形*********
//lpFinger输入图像指针，
//x输入横坐标
//y输入纵坐标
*/
void Draw_Square(unsigned char *lpFinger, int x, int y)
{
	int W = ImageWidth;
	int H = ImageHeight;
	int i = 0;
	int j = 0;

	for (i = 0; i<9; i++)
	{
		j = (y - 5 + i)*W + x + 5;//east
		lpFinger[j] = 0;

		j = (y - 5 + i)*W + x - 5;//west
		lpFinger[j] = 0;

		j = (y - 5)*W + x - 5 + i;//north
		lpFinger[j] = 0;

		j = (y + 5)*W + x - 5 + i;//south
		lpFinger[j] = 0;
	}
	return;
}

/*
**********Sign_Square:对一幅图里面的特征点和奇异点标记正方形*********
//lpFinger输入图像指针，
//feature输入特征点
//centre输入中心点
//c_num输入中心点个数
//triangle输入三角点
//t_num输入三角点个数
*/

void Sign_Square(unsigned char *lpFinger, FEATURE_CD feature_cd, FEATURE_ZS centre, FEATURE_ZS triangle)
{

	int i = 0;
	int index = 0;
	int x = 0, y = 0;
	int east = 0;
	int west = 0;
	int north = 0;
	int south = 0;


	for (i = 0; i < feature_cd.MinutiaNum; i++)
	{
		x = feature_cd.MinutiaArr[i].x;
		y = feature_cd.MinutiaArr[i].y;
		Draw_Square(lpFinger, x, y);
	}

	for (i = 0; i<centre.MinutiaNum; i++)
	{
		x = centre.MinutiaArr[i].x;
		y = centre.MinutiaArr[i].y;
		Draw_Square(lpFinger, x, y);
	}

	for (i = 0; i<triangle.MinutiaNum; i++)
	{
		x = triangle.MinutiaArr[i].x;
		y = triangle.MinutiaArr[i].y;
		Draw_Square(lpFinger, x, y);
	}

	return;
}


