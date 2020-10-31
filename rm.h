// 
//	Record Manager component interface
//	

#ifndef RM_H
#define RM_H

#include "comp.h"

//
// RMFileHdr - �ļ�ͷ��
// 
struct RMFileHdr {
	int free;				// �����е�һ������ҳ
	int size;				// �ļ����Ѿ������˵�ҳ����Ŀ
	int rcdlen;			// ��¼�Ĵ�С
} __attribute__((packed));

struct RMPageHdrEx {
	int free;
	int slots;
	int remain;
	char slotMap[];
} __attribute__((packed));

/*
 
		|-----------------------|
		|	free				|
		|-----------------------|
		|	free				|
		|-----------------------|
		|	slots				|
		|-----------------------|
		|	remain				|
		|-----------------------|
		|	slotMap				|
		|						|
		|						|
		|						|
		|-----------------------|

 */





#endif /* RM_H */