
#include "Stack.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

void InitStack(PSTACK pStack)
{
	pStack->pTop = (PNODE)malloc(sizeof(NODE));

	if (NULL != pStack->pTop)
	{
		pStack->pBottom = pStack->pTop;
		pStack->pTop->pNext = NULL;
	}
	else
	{
		printf("error:distribute failure!\n");
		exit(-1);
	}
	pStack->number = 0;
	return;
}

void PushStack(PSTACK pStack, int val)
{
	PNODE pNew = (PNODE)malloc(sizeof(NODE));

	pNew->data = val;
	pNew->pNext = pStack->pTop;
	pStack->pTop = pNew;

	pStack->number++;
	return;
}

int findnode(PSTACK pStack, int index, int length)
{
	int data = 0;
	PNODE p = pStack->pTop;
	int times = length - index - 1;
	while (times)
	{
		p = p->pNext;

		if (p == pStack->pBottom)//全部遍历完没找到就返回0退出
		{
			return 0;
		}

		times--;
	}
	data = p->data;

	return data;
}

int isEmpty(PSTACK pStack)
{
	if (pStack->pTop == pStack->pBottom)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void  ClearStack(PSTACK pStack)
{
	PNODE p = pStack->pTop;
	PNODE q = NULL;

	if (isEmpty(pStack))
		return;

	//释放内存
	while (p != pStack->pBottom)
	{
		q = p->pNext;
		free(p);
		p = q;
	}

	pStack->pTop = pStack->pBottom;
	return;
}

