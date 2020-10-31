#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pf_buffer.h"
#include "util.h"

using namespace RedBase;
using namespace std;

PFBuffer* PFBuffer::instance_ = nullptr;

//
// getPage - ��ȡһ��ָ�򻺴�ռ��ָ�룬���ҳ���Ѿ��ڻ���֮�еĻ���(re)pin ҳ�棬Ȼ�󷵻�һ��
//	ָ������ָ�롣���ҳ��û���ڻ����У�������ļ��ж�������pin it��Ȼ�󷵻�һ��ָ������ָ�롣���
//  ���������Ļ����滻��һ��ҳ��
//	
RC PFBuffer::getPage(int fd, Page num, Ptr &addr)
{
	int idx = table_.search(fd, num);
	if (idx >= 0) {
		// ҳ���Ѿ����ڴ����ˣ�����ֻ��Ҫ�������ü���
		nodes_[idx].count++;
	}
	else {
		// ����һ���յ�ҳ��,ҳ�����buff�е�λ����idx��¼
		idx = searchAvaliableNode();
		if (idx < 0) return PF_NOBUF;
		readPage(fd, num, nodes_[idx].buffer);
		nodes_[idx].fd = fd;
		nodes_[idx].num = num;
		nodes_[idx].count = 1;
		nodes_[idx].dirty = false;
		table_.insert(fd, num, idx);
	}
	// dstָ��ֱ��ָ��ҳ���ײ�
	addr = nodes_[idx].buffer;
	return 0;
}

//
// pin - ��ҳ�����ڻ�������,�������̺�getPage����
//
RC PFBuffer::pin(int fd, Page num)
{
	int idx = table_.search(fd, num);
	if (idx >= 0) {
		// ҳ���Ѿ����ڴ����ˣ�����ֻ��Ҫ�������ü���
		nodes_[idx].count++;
	}
	else {
		// ����һ���յ�ҳ��,ҳ�����buff�е�λ����idx��¼
		idx = searchAvaliableNode();
		if (idx < 0) return PF_NOBUF;
		readPage(fd, num, nodes_[idx].buffer);
		nodes_[idx].fd = fd;
		nodes_[idx].num = num;
		nodes_[idx].count = 1;
		nodes_[idx].dirty = false;
		table_.insert(fd, num, idx);
	}
	return 0;
}

//
// allocatePage - ����һ��ҳ��
// 
RC PFBuffer::allocPage(int fd, Page num, Ptr &addr)
{
	int idx = table_.search(fd, num);
	if (idx > 0) return PF_PAGEINBUF;

	// ����յ�ҳ
	idx = searchAvaliableNode();
	if (idx < 0) return PF_NOBUF;

	table_.insert(fd, num, idx);
	nodes_[idx].fd = fd;
	nodes_[idx].num = num;
	nodes_[idx].dirty = false;
	nodes_[idx].count = 1;
	addr = nodes_[idx].buffer;
	return 0;
}

//
// markDirty - ��һ��ҳ����Ϊ��,������bufffer�ж�����ʱ��,���ᱻ�ص��ļ���
// 
RC PFBuffer::markDirty(int fd, Page num)
{
	int idx = table_.search(fd, num);
	// ҳ���������ڻ�����
	if (idx < 0)  return PF_PAGENOTINBUF;

	if (nodes_[idx].count == 0) return PF_PAGEUNPINNED;
	// ���Ϊ��ҳ
	nodes_[idx].dirty = true;
	return 0;
}

//
// unpin - ����һ��������
//	
RC PFBuffer::unpin(int fd, Page num)
{
	int idx = table_.search(fd, num);
	if (idx < 0) return PF_PAGENOTINBUF;
	nodes_[idx].count -= 1;
	if (nodes_[idx].count == 0) {
		//printf("num = %d, count = 0, dirty = %d\n", num, nodes_[idx].count, nodes_[idx].dirty);
		if (nodes_[idx].dirty) {
			//printf("write, back\n");
			writeBack(nodes_[idx].fd, nodes_[idx].num, nodes_[idx].buffer);
			nodes_[idx].dirty = false;
		}
	}
	return 0;
}

//
// flush - �����ҳ��ȫ����ˢ��������
// 
RC PFBuffer::flush(int fd)
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].fd == fd) {
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
		}
	}
	return 0;
}

//
// forcePages
// 
RC PFBuffer::forcePages(int fd, Page num)
{
	for (int i = 0; i < capacity; i++) {
		if ((nodes_[i].fd == fd) && (nodes_[i].num == num)) {
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
		}
	}
	return 0;
}

//
// clearFilePages - һ�����ļ�ǰ����,�����л����ҳ��ȫ���Ƴ�
//
void PFBuffer::clearFilePages(int fd)
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].fd == fd) {
			table_.remove(nodes_[i].fd, nodes_[i].num);
			if (nodes_[i].dirty) {
				writeBack(fd, nodes_[i].num, nodes_[i].buffer);
				nodes_[i].dirty = false;
			}
			nodes_[i].fd = -1;
			nodes_[i].num = -1;
			nodes_[i].count = 0;
		}
	}
}

//
// searchAvaliableNode - �ڲ�ʹ�õĺ���
// 
int PFBuffer::searchAvaliableNode()
{
	for (int i = 0; i < capacity; i++) {
		if (nodes_[i].count == 0) {
			table_.remove(nodes_[i].fd, nodes_[i].num);
			return i;
		}
	}
	return -1;
}

//
// readPage - �Ӵ����ж�ȡһ��ҳ��
//	
RC PFBuffer::readPage(int fd, Page num, Ptr dst)
{
	long offset = num * (long)pagesize + PF_FILE_HDR_SIZE;
	Lseek(fd, offset, L_SET);
	// ��ȡ����
	int n = Read(fd, dst, pagesize);
	if (n != pagesize) return PF_INCOMPLETEREAD;
	return 0; // һ������
}


//
// writeBack - ��src��������д�ص�����
// 
RC PFBuffer::writeBack(int fd, Page num, Ptr src)
{
	// �����ҳ�����,���ļ�ͷ��֮��ʼ
	long offset = num * pagesize + PF_FILE_HDR_SIZE;
	Lseek(fd, offset, L_SET);

	int n = Write(fd, src, pagesize);
	if (n != pagesize) return PF_INCOMPLETEWRITE;
	return 0;
}


