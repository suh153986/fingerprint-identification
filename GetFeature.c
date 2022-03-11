
#include "GetFeature.h"

FEATURE GetFeature(FEATURE_CD feature_cz, FEATURE_ZS centre_dot, FEATURE_ZS triangle_dot)
{
	int i = 0;
	int j = 0;
	FEATURE feature;

	feature.CoreNum = centre_dot.MinutiaNum;//中心点
	for (i = 0; i < feature.CoreNum; i++)
	{
		feature.CoreArr[i].x = centre_dot.MinutiaArr[i].x;
		feature.CoreArr[i].y = centre_dot.MinutiaArr[i].y;
		feature.CoreArr[i].Direction = centre_dot.MinutiaArr[i].Direction;
		for (j = 0; j < 3; j++)
		{
			feature.CoreArr[i].Triangle[j] = centre_dot.MinutiaArr[i].Triangle[j];
		}
		feature.CoreArr[i].Type = centre_dot.MinutiaArr[i].Type;
	}

	feature.DeltaNum = triangle_dot.MinutiaNum;//三角点
	for (i = 0; i < feature.DeltaNum; i++)
	{
		feature.DeltaArr[i].x = triangle_dot.MinutiaArr[i].x;
		feature.DeltaArr[i].y = triangle_dot.MinutiaArr[i].y;
		feature.DeltaArr[i].Direction = triangle_dot.MinutiaArr[i].Direction;
		for (j = 0; j < 3; j++)
		{
			feature.DeltaArr[i].Triangle[j] = triangle_dot.MinutiaArr[i].Triangle[j];
		}
		feature.DeltaArr[i].Type = triangle_dot.MinutiaArr[i].Type;
	}

	feature.MinutiaNum = feature_cz.MinutiaNum;//特征端点和叉点
	for (i = 0; i < feature.MinutiaNum; i++)
	{
		feature.MinutiaArr[i].x = feature_cz.MinutiaArr[i].x;
		feature.MinutiaArr[i].y = feature_cz.MinutiaArr[i].y;
		feature.MinutiaArr[i].Direction = feature_cz.MinutiaArr[i].Direction;
		for (j = 0; j < 3; j++)
		{
			feature.MinutiaArr[i].Triangle[j] = feature_cz.MinutiaArr[i].Triangle[j];
		}
		feature.MinutiaArr[i].Type = feature_cz.MinutiaArr[i].Type;
	}

	return feature;
}
