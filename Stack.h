
#pragma once

/*
**定义堆栈：用来存储数据：
初始化一个头结点，不存储数据，每次增加一个数据相当于进栈，数据加一
*/


typedef struct Node
{
	int data;
	struct Node * pNext;
} NODE, *PNODE;


typedef struct Stack
{
	PNODE pTop;
	PNODE pBottom;
	int number;
} STACK, *PSTACK;

void InitStack(PSTACK pStack);
void PushStack(PSTACK pStack, int val);
int findnode(PSTACK pStack, int index, int length);
int isEmpty(PSTACK pStack);
void  ClearStack(PSTACK pStack);

