#pragma once


#include "Stack.h"
#define     MAXMINUTIANUM			60			// 最大特征点数
#define PI					3.1415926
#define EPI					57.29578

#define VF_MINUTIA_CORE		  3
#define VF_MINUTIA_DELTA	  4

#define CENTRALRADIUS 60

/*
// 特征点结构
typedef struct tagMinutiae {
int    x;				// 横坐标
int    y;				// 纵坐标
int    Direction;		// 方向
int	Triangle[3];	// 特征点为中心外接圆半径为定值的正三角形三个顶点的方向
int    Type;			// 类型
} MINUTIA, *MINUTIAPTR;

*/
// 指纹特征(模板)结构:特征端点和叉点，中心点、三角点
typedef struct tagALLFeature {
	int		MinutiaNum;					    // 特征点数
	MINUTIA		MinutiaArr[MAXMINUTIANUM];	// 特征点数组
	int     CoreNum;                        //中心点数
	MINUTIA     CoreArr[MAXMINUTIANUM];     //中心点数组
	int     DeltaNum;                       //三角点数
	MINUTIA     DeltaArr[MAXMINUTIANUM];    //三角点数组
} FEATURE, *FEATUREPTR;

//匹配结果
typedef struct tagMatchResult {
	int    Similarity;
	int    Rotation;
	int    TransX;
	int    TransY;
	int	MMCount;
} MATCHRESULT, *PMATCHRESULT;

typedef char  VF_FLAG;

// 精确比对模式，主要用在少量比对次数场合
#define	VF_MATCHMODE_VERIFY			1
// 快速比对模式，主要用在大量比对次数场合
#define	VF_MATCHMODE_IDENTIFY		2

//配准点
void align(FEATUREPTR lpFeature, FEATUREPTR lpAlignedFeature, MINUTIAPTR lpFeatureCore, int rotation, int transx, int transy);

//配准后进行匹配
void alignMatch(FEATUREPTR lpAlignFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode);

//中心点匹配
void coreMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode, int n, int m);

//三角点匹配
void deltaMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate,
	PMATCHRESULT lpMatchResult, VF_FLAG matchMode, PSTACK n_delta, PSTACK m_delta);

//求两个坐标之间的距离
int dist(int x0, int y0, int x1, int y1);

//中间区域匹配
void centralMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode);

//全部匹配
void  globalMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode);

//匹配对准
void  patternMatch(FEATUREPTR lpFeature, FEATUREPTR lpTemplate, PMATCHRESULT lpMatchResult, VF_FLAG matchMode);