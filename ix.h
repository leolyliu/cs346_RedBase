#ifndef IX_H
#define IX_H

#include "redbase.h"
#include "pf.h"

using KeyPtr = void *;

//
// IXFileHdr - �����ļ���ͷ��
// 
struct IXFileHdr {
	uint size;		// ҳ����Ŀ
	uint limitSpace; // ÿһҳ���ֻ�ܹ�ʹ�õĿռ��С
	Page root;		// ��һҳ��λ��
	Page leaf;		// ��һ��Ҷ�����ڵ�ҳ����
	uint pairSize;  // ��ֵ�ԵĴ�С
	uint height;	// �������ĸ߶�
	AttrType type;  // ��������
	uint len;		// ���ĳ���
	uint capacity;  // ��¼��һ��node������Է���key����Ŀ
};

#endif /* IX_H */