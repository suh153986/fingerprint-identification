
#include "ImageMinutiaExtract.h"
#include "ImageMatch.h"
#include "Stack.h"
#include <math.h>
#include <string.h>



// 为节省时间，短距离的两点间距离采用查表法
// DisTbl[m][n] = (int)(sqrt(m*m+n*n)+0.5)
int  DisTbl[10][10] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	1, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	2, 2, 2, 3, 4, 5, 6, 7, 8, 9,
	3, 3, 3, 4, 5, 5, 6, 7, 8, 9,
	4, 4, 4, 5, 5, 6, 7, 8, 8, 9,
	5, 5, 5, 5, 6, 7, 7, 8, 9, 10,
	6, 6, 6, 6, 7, 7, 8, 9, 10, 10,
	7, 7, 7, 7, 8, 8, 9, 9, 10, 11,
	8, 8, 8, 8, 8, 9, 10, 10, 11, 12,
	9, 9, 9, 9, 9, 10, 10, 11, 12, 12
};

/*
**********align: 将指纹特征按一定的角度和位置偏移进行坐标变换***************
//  lpFeature:	[in] 要变换的指纹特征
//  lpAlignedFeature:	[out] 进行坐标变换后的指纹特征
//  lpFeatureCore:	[in] 旋转变换的中心特征点
//  rotation:	[in] 旋转角度
//  transx:	[in] 水平偏移
//  transy: [in] 垂直偏移
*/
void align(FEATUREPTR lpFeature, FEATUREPTR lpAlignedFeature, MINUTIAPTR lpFeatureCore, int rotation, int transx, int transy)
{
	int  i = 0;
	int  x = 0, y = 0;
	int  cx = 0, cy = 0;
	double  rota = 0.0, sinv = 0.0, cosv = 0.0;

	*lpAlignedFeature = *lpFeature;// 复制整个结构信息

	cx = lpFeatureCore->x;// 坐标转换的中心点坐标
	cy = lpFeatureCore->y;

	rota = rotation / EPI;// 旋转的弧度
	sinv = sin(rota);// 旋转弧度的sin值
	cosv = cos(rota);// 旋转弧度的cos值

	for (i = 0; i < lpFeature->MinutiaNum; i++)
	{
		x = lpFeature->MinutiaArr[i].x;
		y = lpFeature->MinutiaArr[i].y;
		//  坐标转换后的新坐标
		lpAlignedFeature->MinutiaArr[i].x = (int)(cx + cosv*(x - cx) - sinv*(y - cy) + transx + 0.5);
		lpAlignedFeature->MinutiaArr[i].y = (int)(cy + sinv*(x - cx) + cosv*(y - cy) + transy + 0.5);
		// 旋转后特征点的新方向
		lpAlignedFeature->MinutiaArr[i].Direction = (lpFeature->MinutiaArr[i].Direction + rotation) % 360;
	}

}

/*
*****************alignMatch: 两个坐标系对齐的指纹特征进行比对****************
lpAlignFeature表示样板指纹特征数据
lpTemplate表示模板指纹特征数据
lpMatchResult表示匹配结果统计结构
matchMode表示匹配模式：快速对比和正常对比
原理：两个坐标之间的角度差和距离差在一定范围内则说明两点是匹配的
*/
void alignMatch(FEATUREPTR lpAlignFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode)
{
	int	i = 0, j = 0;
	char	flagA[MAXMINUTIANUM];	// 标记lpAlignFeature中特征点是否已经有匹配对象
	char	flagT[MAXMINUTIANUM];	// 标记lpTemplate中特征点是否已经有匹配对象
	int	x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	int  dis = 0, angle = 0;
	int  s = 0;
	int	num1 = 0, num2 = 0;
	int  step = 1;
	num1 = lpAlignFeature->MinutiaNum;
	num2 = lpTemplate->MinutiaNum;

	memset(flagA, 0, MAXMINUTIANUM);// 标记清零
	memset(flagT, 0, MAXMINUTIANUM);

	int score = 0;		// 总分清零
	int matchNum = 0;	// 匹配特征点数清零

						// 相同类型特征点的匹配分数
	for (i = 0; i < lpTemplate->MinutiaNum; i++)
	{
		if (flagT[i])	// 是否已有匹配对象
			continue;
		for (j = 0; j < lpAlignFeature->MinutiaNum; j++)
		{
			if (flagA[j])	// 是否已有匹配对象
				continue;
			// 特征点类型是否相同
			if (lpTemplate->MinutiaArr[i].Type != lpAlignFeature->MinutiaArr[j].Type)
				continue;
			// 特征点方向夹角
			angle = AngleAbs360(lpTemplate->MinutiaArr[i].Direction, lpAlignFeature->MinutiaArr[j].Direction);

			if (angle >= 10)// 夹角>=10则不匹配
				continue;

			x1 = lpTemplate->MinutiaArr[i].x;
			y1 = lpTemplate->MinutiaArr[i].y;
			x2 = lpAlignFeature->MinutiaArr[j].x;
			y2 = lpAlignFeature->MinutiaArr[j].y;

			if (abs(x1 - x2) >= 10)// 水平距离>=10则不匹配
				continue;

			if (abs(y1 - y2) >= 10)// 垂直距离>=10则不匹配
				continue;

			dis = DisTbl[abs(y1 - y2)][abs(x1 - x2)];// 两特征点间的距离

			if (dis >= 10)// 距离>=10则不匹配
				continue;


			flagA[j] = 1;// 对这两个特征点做标记，表示已经有匹配对象
			flagT[i] = 1;

			// 总分加上此两个特征点的匹配分数
			// 此表明dis,angle越大，分数越小
			score += (10 - angle);
			score += (10 - dis);

			matchNum++;// 匹配特征点数加一

					   // 如果是快速比对模式
			if (matchMode == VF_MATCHMODE_IDENTIFY && matchNum >= 8)
			{
				// 计算相似度
				s = 4 * score * matchNum * MAXMINUTIANUM / ((num1 + num2) * (num1 + num2));
				if (s > 100)  // 相似度足够大则返回比对结果
				{
					lpMatchResult->MMCount = matchNum;
					lpMatchResult->Rotation = 0;
					lpMatchResult->Similarity = s;
					lpMatchResult->TransX = 0;
					lpMatchResult->TransX = 0;
					return;
				}
			}

		}
	}

	if (matchMode != VF_MATCHMODE_IDENTIFY)
	{
		// 由于图像处理的误差导致可能端点处理成叉点或叉点处理成端点，假设概率为50%，计算此种情况的分数
		for (i = 0; i < lpTemplate->MinutiaNum; i++)
		{
			if (flagT[i])	// 是否已有匹配对象
				continue;
			for (j = 0; j < lpAlignFeature->MinutiaNum; j++)
			{
				if (flagA[j])	// 是否已有匹配对象
					continue;
				// 是否类型不同
				if (lpTemplate->MinutiaArr[i].Type == lpAlignFeature->MinutiaArr[j].Type)
					continue;
				// 特征点方向夹角
				angle = AngleAbs360(lpTemplate->MinutiaArr[i].Direction,
					lpAlignFeature->MinutiaArr[j].Direction);
				// 夹角>=10则不匹配
				if (angle >= 10)
					continue;
				x1 = lpTemplate->MinutiaArr[i].x;
				y1 = lpTemplate->MinutiaArr[i].y;
				x2 = lpAlignFeature->MinutiaArr[j].x;
				y2 = lpAlignFeature->MinutiaArr[j].y;

				// 水平距离>=10则不匹配
				if (abs(x1 - x2) >= 10)
					continue;
				// 垂直距离>=10则不匹配
				if (abs(y1 - y2) >= 10)
					continue;
				// 两特征点间的距离
				dis = DisTbl[abs(y1 - y2)][abs(x1 - x2)];
				// 距离>=10则不匹配
				if (dis >= 10)
					continue;
				// 对这两个特征点做标记，表示已经有匹配对象
				flagA[j] = 1;
				flagT[i] = 1;
				// 总分加上此两个特征点的匹配分数
				score += ((10 - angle) / 2);
				score += ((10 - dis) / 2);
				// 匹配特征点数加一
				matchNum++;

			}
		}
	}
	// 计算相似度，返回比对结果
	s = 4 * score * matchNum * MAXMINUTIANUM / ((num1 + num2)*(num1 + num2));

	lpMatchResult->MMCount = matchNum;
	lpMatchResult->Rotation = 0;
	lpMatchResult->Similarity = s;
	lpMatchResult->TransX = 0;
	lpMatchResult->TransX = 0;

}


/*
*****************coreMatch: 中心点配准****************
lpAlignFeature表示样板指纹特征数据
lpTemplate表示模板指纹特征数据
lpMatchResult表示匹配结果统计结构
matchMode表示匹配模式：快速对比和正常对比
*/
void coreMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode, int n, int m)
{
	int i = 0;
	int j = 0;
	MATCHRESULT alignMax;	// 相似度最大的比对结果
	MATCHRESULT globalMatchResult;	// 比对结果
	int  agate = 8;		// 三角拓扑结构角度误差
	int  num = 0;

	// 初始化最好的比对结果
	alignMax.Similarity = 0;
	alignMax.MMCount = 0;
	alignMax.Rotation = 0;
	alignMax.TransX = 0;
	alignMax.TransY = 0;

	FEATURE	alignFeature;	// 对齐后的指纹特征

							// 位置偏移
	int transx = (lpTemplate->MinutiaArr[n].x - lpFeature->MinutiaArr[m].x);
	int transy = (lpTemplate->MinutiaArr[n].y - lpFeature->MinutiaArr[m].y);
	int rotation = 0;//角度偏移


	for (i = 0; i < lpFeature->MinutiaNum; i++)
	{
		for (j = 0; j < lpTemplate->MinutiaNum; j++)
		{
			alignFeature.MinutiaNum = 0;

			if (lpFeature->MinutiaArr[i].Type == VF_MINUTIA_CORE || lpTemplate->MinutiaArr[j].Type == VF_MINUTIA_CORE)
				continue;
			if (lpFeature->MinutiaArr[i].Type == VF_MINUTIA_DELTA || lpTemplate->MinutiaArr[j].Type == VF_MINUTIA_DELTA)
				continue;

			int rotation = GetAngle(lpTemplate->MinutiaArr[j].x, lpTemplate->MinutiaArr[j].y,
				lpFeature->MinutiaArr[i].x, lpFeature->MinutiaArr[i].y);

			//结构拓扑对齐
			align(lpFeature, &alignFeature, &lpFeature->MinutiaArr[i], rotation, transx, transy);


			// 将两个对齐的指纹特征进行比对
			alignMatch(&alignFeature, lpTemplate, &globalMatchResult, matchMode);
			// 如果比对结果比最好的比对结果更好，则更新alignMax
			if (globalMatchResult.Similarity > alignMax.Similarity)
			{
				alignMax.MMCount = globalMatchResult.MMCount;
				alignMax.Similarity = globalMatchResult.Similarity;
				alignMax.Rotation = rotation;
				alignMax.TransX = transx;
				alignMax.TransY = transy;

				// 如果是快速比对模式，则相似度达到一定程度则退出
				if (matchMode == VF_MATCHMODE_IDENTIFY && alignMax.MMCount >= 8)
				{
					if (alignMax.Similarity > 100)
					{
						*lpMatchResult = alignMax;
						return;
					}
				}
			}
		}
	}
	//最终比对结果
	*lpMatchResult = alignMax;
	return;
}

/*
*****************deltaMatch: 三角点配准****************
lpAlignFeature表示样板指纹特征数据
lpTemplate表示模板指纹特征数据
lpMatchResult表示匹配结果统计结构
matchMode表示匹配模式：快速对比和正常对比
*/
void deltaMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate,
	PMATCHRESULT lpMatchResult, VF_FLAG matchMode, PSTACK n_delta, PSTACK m_delta)
{
	int nn = 0;
	int mm = 0;
	int i = 0;
	int j = 0;
	MATCHRESULT alignMax;	// 相似度最大的比对结果
	MATCHRESULT globalMatchResult;	// 比对结果
	int  agate = 8;		// 三角拓扑结构角度误差
	int  num = 0;
	// 初始化最好的比对结果
	alignMax.Similarity = 0;
	alignMax.MMCount = 0;
	alignMax.Rotation = 0;
	alignMax.TransX = 0;
	alignMax.TransY = 0;

	FEATURE	alignFeature;	// 对齐后的指纹特征
	int n, m;

	int transx = 0;
	int transy = 0;
	int rotation = 0;
	for (nn = 0; nn < (n_delta->number); nn++)
		for (mm = 0; mm < (m_delta->number); mm++)
		{

			n = findnode(n_delta, nn, n_delta->number);
			m = findnode(m_delta, mm, m_delta->number);

			// 位置偏移
			transx = (lpTemplate->MinutiaArr[n].x - lpFeature->MinutiaArr[m].x);
			transy = (lpTemplate->MinutiaArr[n].y - lpFeature->MinutiaArr[m].y);


			for (i = 0; i < lpFeature->MinutiaNum; i++)
			{
				for (j = 0; j < lpTemplate->MinutiaNum; j++)
				{
					alignFeature.MinutiaNum = 0;

					if (lpFeature->MinutiaArr[i].Type == VF_MINUTIA_CORE || lpTemplate->MinutiaArr[j].Type == VF_MINUTIA_CORE)
						continue;
					if (lpFeature->MinutiaArr[i].Type == VF_MINUTIA_DELTA || lpTemplate->MinutiaArr[j].Type == VF_MINUTIA_DELTA)
						continue;


					rotation = GetAngle(lpFeature->MinutiaArr[i].x, lpFeature->MinutiaArr[i].y,
						lpTemplate->MinutiaArr[j].x, lpTemplate->MinutiaArr[j].y);

					align(lpFeature, &alignFeature, &lpFeature->MinutiaArr[i], rotation, transx, transy);


					// 将两个对齐的指纹特征进行比对
					alignMatch(&alignFeature, lpTemplate, &globalMatchResult, matchMode);
					// 如果比对结果比最好的比对结果更好，则更新alignMax
					if (globalMatchResult.Similarity > alignMax.Similarity)
					{
						alignMax.MMCount = globalMatchResult.MMCount;
						alignMax.Similarity = globalMatchResult.Similarity;
						alignMax.Rotation = rotation;
						alignMax.TransX = transx;
						alignMax.TransY = transy;

						// 如果是快速比对模式，则相似度达到一定程度则退出
						if (matchMode == VF_MATCHMODE_IDENTIFY && alignMax.MMCount >= 8)
						{
							if (alignMax.Similarity > 100)
							{
								*lpMatchResult = alignMax;
								return;
							}
						}
					}
				}
			}
		}


	//最终比对结果
	*lpMatchResult = alignMax;

}

//求两个坐标之间的距离
int dist(int x0, int y0, int x1, int y1)
{
	return (int)sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
}

/*
*****************centralMatch: 中间区域配准****************
lpAlignFeature表示样板指纹特征数据
lpTemplate表示模板指纹特征数据
lpMatchResult表示匹配结果统计结构
matchMode表示匹配模式：快速对比和正常对比
*/
void centralMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode)
{
	int	m, n, a1, a2;
	int	rotation;		// 旋转角度
	int	transx, transy;	// 位置偏移
	FEATURE	alignFeature;	// 对齐后的指纹特征
	MATCHRESULT alignMax;	// 相似度最大的比对结果
	MATCHRESULT globalMatchResult;	// 比对结果
	int  agate = 8;		// 三角拓扑结构角度误差
	int  num = 0;
	// 初始化最好的比对结果
	alignMax.Similarity = 0;
	alignMax.MMCount = 0;
	alignMax.Rotation = 0;
	alignMax.TransX = 0;
	alignMax.TransY = 0;

	int nx = 0, ny = 0;
	for (n = 0; n < lpTemplate->MinutiaNum; n++)
	{
		nx += lpTemplate->MinutiaArr[n].x;
		ny += lpTemplate->MinutiaArr[n].y;
	}

	nx = nx / lpTemplate->MinutiaNum;
	ny = ny / lpTemplate->MinutiaNum;


	int mx = 0, my = 0;
	for (m = 0; m < lpFeature->MinutiaNum; m++)
	{
		mx += lpFeature->MinutiaArr[m].x;
		my += lpFeature->MinutiaArr[m].y;
	}

	mx = mx / lpFeature->MinutiaNum;
	my = my / lpFeature->MinutiaNum;

	int Counter = 0;


	// 对相同类型的指纹特征两两作为同一个指纹特征进行对齐比对
	for (n = 0; n < lpTemplate->MinutiaNum; n++)
	{
		if (dist(nx, ny, lpTemplate->MinutiaArr[n].x, lpTemplate->MinutiaArr[n].y) > CENTRALRADIUS)
			continue;

		for (m = 0; m < lpFeature->MinutiaNum; m++)
		{
			// 不同类型则不比对
			if (lpFeature->MinutiaArr[m].Type != lpTemplate->MinutiaArr[n].Type)
				continue;
			if (dist(mx, my, lpFeature->MinutiaArr[m].x, lpFeature->MinutiaArr[m].y) > CENTRALRADIUS)
				continue;

			Counter++;

			if (matchMode == VF_MATCHMODE_IDENTIFY)
			{
				// 特征点三角拓扑结构比对，相似则进行配准
				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Direction % 180);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Direction % 180);
					if (abs(a1 - a2)>agate)
						continue;
				}
				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[1] != 255 && lpTemplate->MinutiaArr[n].Triangle[1] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Triangle[1]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Triangle[1]);
					if (abs(a1 - a2)>agate)
						continue;
				}
				if (lpFeature->MinutiaArr[m].Triangle[2] != 255 && lpTemplate->MinutiaArr[n].Triangle[2] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[1] != 255 && lpTemplate->MinutiaArr[n].Triangle[1] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[1], lpFeature->MinutiaArr[m].Triangle[2]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[1], lpTemplate->MinutiaArr[n].Triangle[2]);
					if (abs(a1 - a2)>agate)
						continue;
				}
				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[2] != 255 && lpTemplate->MinutiaArr[n].Triangle[2] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Triangle[2]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Triangle[2]);
					if (abs(a1 - a2)>agate)
						continue;
				}
			}
			alignFeature.MinutiaNum = 0;
			// 旋转角度
			rotation = GetAngleDis(lpFeature->MinutiaArr[m].Direction,
				lpTemplate->MinutiaArr[n].Direction);
			// 位置偏移
			transx = (lpTemplate->MinutiaArr[n].x - lpFeature->MinutiaArr[m].x);
			transy = (lpTemplate->MinutiaArr[n].y - lpFeature->MinutiaArr[m].y);

			// 将lpFeature与lpTemplate对齐
			align(lpFeature, &alignFeature, &lpFeature->MinutiaArr[m], rotation, transx, transy);
			// 将两个对齐的指纹特征进行比对
			alignMatch(&alignFeature, lpTemplate, &globalMatchResult, matchMode);
			// 如果比对结果比最好的比对结果更好，则更新alignMax
			if (globalMatchResult.Similarity > alignMax.Similarity)
			{
				alignMax.MMCount = globalMatchResult.MMCount;
				alignMax.Similarity = globalMatchResult.Similarity;
				alignMax.Rotation = rotation;
				alignMax.TransX = transx;
				alignMax.TransY = transy;

				// 如果是快速比对模式，则相似度达到一定程度则退出
				if (matchMode == VF_MATCHMODE_IDENTIFY && alignMax.MMCount >= 8)
				{
					if (alignMax.Similarity > 100)
					{
						*lpMatchResult = alignMax;
						return;
					}
				}
			}
		}
	}

	//最终比对结果
	*lpMatchResult = alignMax;
}

/*
*****************globalMatch: 全局区配准****************
lpAlignFeature表示样板指纹特征数据
lpTemplate表示模板指纹特征数据
lpMatchResult表示匹配结果统计结构
matchMode表示匹配模式：快速对比和正常对比
*/
void  globalMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode)
{
	int	m, n, a1, a2;
	int	rotation;		// 旋转角度
	int	transx, transy;	// 位置偏移
	FEATURE	alignFeature;	// 对齐后的指纹特征
	MATCHRESULT alignMax;	// 相似度最大的比对结果
	MATCHRESULT globalMatchResult;	// 比对结果
	int  agate = 8;		// 三角拓扑结构角度误差
	int  num = 0;
	// 初始化最好的比对结果
	alignMax.Similarity = 0;
	alignMax.MMCount = 0;
	alignMax.Rotation = 0;
	alignMax.TransX = 0;
	alignMax.TransY = 0;


	// 对相同类型的指纹特征两两作为同一个指纹特征进行对齐比对
	for (n = 0; n < lpTemplate->MinutiaNum; n++)
	{
		for (m = 0; m < lpFeature->MinutiaNum; m++)
		{
			// 不同类型则不比对
			if (lpFeature->MinutiaArr[m].Type != lpTemplate->MinutiaArr[n].Type)
				continue;
			if (matchMode == VF_MATCHMODE_IDENTIFY)
			{
				// 特征点三角拓扑结构比对，相似则进行配准
				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Direction % 180);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Direction % 180);
					if (abs(a1 - a2)>agate)
						continue;
				}

				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[1] != 255 && lpTemplate->MinutiaArr[n].Triangle[1] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Triangle[1]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Triangle[1]);
					if (abs(a1 - a2)>agate)
						continue;
				}

				if (lpFeature->MinutiaArr[m].Triangle[2] != 255 && lpTemplate->MinutiaArr[n].Triangle[2] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[1] != 255 && lpTemplate->MinutiaArr[n].Triangle[1] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[1], lpFeature->MinutiaArr[m].Triangle[2]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[1], lpTemplate->MinutiaArr[n].Triangle[2]);
					if (abs(a1 - a2)>agate)
						continue;
				}

				if (lpFeature->MinutiaArr[m].Triangle[0] != 255 && lpTemplate->MinutiaArr[n].Triangle[0] != 255 &&
					lpFeature->MinutiaArr[m].Triangle[2] != 255 && lpTemplate->MinutiaArr[n].Triangle[2] != 255)
				{
					a1 = GetJiajiao(lpFeature->MinutiaArr[m].Triangle[0], lpFeature->MinutiaArr[m].Triangle[2]);
					a2 = GetJiajiao(lpTemplate->MinutiaArr[n].Triangle[0], lpTemplate->MinutiaArr[n].Triangle[2]);
					if (abs(a1 - a2)>agate)
						continue;
				}
			}
			alignFeature.MinutiaNum = 0;
			// 旋转角度
			rotation = GetAngleDis(lpFeature->MinutiaArr[m].Direction,
				lpTemplate->MinutiaArr[n].Direction);
			// 位置偏移
			transx = (lpTemplate->MinutiaArr[n].x - lpFeature->MinutiaArr[m].x);
			transy = (lpTemplate->MinutiaArr[n].y - lpFeature->MinutiaArr[m].y);

			// 将lpFeature与lpTemplate对齐
			align(lpFeature, &alignFeature, &lpFeature->MinutiaArr[m],
				rotation, transx, transy);
			// 将两个对齐的指纹特征进行比对
			alignMatch(&alignFeature, lpTemplate, &globalMatchResult, matchMode);
			// 如果比对结果比最好的比对结果更好，则更新alignMax
			if (globalMatchResult.Similarity > alignMax.Similarity)
			{
				alignMax.MMCount = globalMatchResult.MMCount;
				alignMax.Similarity = globalMatchResult.Similarity;
				alignMax.Rotation = rotation;
				alignMax.TransX = transx;
				alignMax.TransY = transy;

				// 如果是快速比对模式，则相似度达到一定程度则退出
				if (matchMode == VF_MATCHMODE_IDENTIFY && alignMax.MMCount >= 8)
				{
					if (alignMax.Similarity > 100)
					{
						*lpMatchResult = alignMax;
						return;
					}
				}
			}
		}
	}

	//最终比对结果
	*lpMatchResult = alignMax;
}

/*
*****patternMatch:指纹全部比对**********
lpFeature表示样板指纹特征数据指针
pTemplate表示模板指针特征数据指针
lpMatchResult表示比对统计结果数据指针
matchMode表示比对模式（快读比对和正常比对）
*/
void  patternMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode)
{
	int i = 0;
	int j = 0;
	STACK n_core1;//样板中心点堆栈
	PSTACK n_core = &n_core1;
	InitStack(n_core);

	STACK m_core1;//模板中心点堆栈
	PSTACK m_core = &m_core1;
	InitStack(m_core);

	STACK n_delta1;//样板三角点堆栈
	PSTACK n_delta = &n_delta1;
	InitStack(n_delta);

	STACK m_delta1;//模板三角点堆栈
	PSTACK m_delta = &m_delta1;
	InitStack(m_delta);


	int n = 0, m = 0;

	for (n = 0; n < lpFeature->CoreNum; n++)
	{
		if (lpFeature->CoreArr[n].Type == VF_MINUTIA_CORE) //找出样板中心点
		{
			PushStack(n_core, n);
		}

		/*if (lpFeature->MinutiaArr[n].Type == VF_MINUTIA_DELTA) {
		n_delta.push_back(n);
		}*/
	}

	for (m = 0; m < lpTemplate->CoreNum; m++)
	{
		if (lpTemplate->CoreArr[m].Type == VF_MINUTIA_CORE)//找出模板中心点
		{
			PushStack(m_core, m);
		}

		/*if (lpTemplate->MinutiaArr[m].Type == VF_MINUTIA_DELTA) {
		m_delta.push_back(m);
		}*/
	}

	//两者都有中心点则尝试中心点配准比对
	if ((n_core->number) > 0 && (m_core->number) > 0)
	{
		for (i = 0; i < (n_core->number); i++)
			for (j = 0; j < (m_core->number); j++)
			{
				//n = (int)n_core[i];
				n = findnode(n_core, i, n_core->number);

				//m = (int)m_core[j];
				m = findnode(m_core, j, m_core->number);

				coreMatch(lpFeature, lpTemplate, lpMatchResult, matchMode, n, m);

				// 如果是快速比对模式，则相似度达到一定程度则退出
				if (matchMode == VF_MATCHMODE_IDENTIFY && lpMatchResult->MMCount >= 8)
				{
					if (lpMatchResult->Similarity > 100)
					{
						return;
					}
				}
			}
	}

	for (n = 0; n < lpFeature->DeltaNum; n++)
	{
		if (lpFeature->DeltaArr[n].Type == VF_MINUTIA_DELTA) //找出样板三角点
		{
			PushStack(n_delta, n);
		}
	}

	for (m = 0; m < lpTemplate->DeltaNum; m++)
	{
		if (lpTemplate->DeltaArr[m].Type == VF_MINUTIA_DELTA)//找出模板三角点
		{
			PushStack(m_delta, m);
		}
	}

	//两者都有三角点，则尝试三角点配准比对
	if ((n_delta->number) > 0 && (m_delta->number) > 0)
	{
		deltaMatch(lpFeature, lpTemplate, lpMatchResult, matchMode, n_delta, m_delta);

		// 如果是快速比对模式，则相似度达到一定程度则退出
		if (matchMode == VF_MATCHMODE_IDENTIFY && lpMatchResult->MMCount >= 8)
		{
			if (lpMatchResult->Similarity > 100)
			{
				return;
			}
		}
	}

	//两者不能满足中心点和三角点配准条件，则进行中心区域配准比对
	centralMatch(lpFeature, lpTemplate, lpMatchResult, matchMode);
	if (matchMode == VF_MATCHMODE_IDENTIFY && lpMatchResult->MMCount >= 8)
	{
		if (lpMatchResult->Similarity > 100)
		{
			return;
		}
	}

	//如果中心区域配准比对失败，则尝试全局配准比对
	globalMatch(lpFeature, lpTemplate, lpMatchResult, matchMode);

	//释放堆栈里面的内存
	ClearStack(n_core);
	ClearStack(m_core);
	ClearStack(n_delta);
	ClearStack(m_delta);

	return;

}
