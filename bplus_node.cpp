#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include "bplus_node.h"
#include "ix.h"
#include "pf.h"
#include "pf.h"


int BPlusNode::search(KeyPtr key, const RID &rid)
{
	int low = lowerBound(key);
	if (low < 0) return -1;
	for (int i = low; i < size_; i++) {
		if (keyComp(keyAt(i), key) != 0) break;
		if (rids_[i] == rid) return i;
	}
	return -1;
}

//
// leftmostSearchPos - ���������һ����keyС�������±�,���������key�Ļ�,����-1
//
int BPlusNode::leftmostSearchPos(KeyPtr key)
{
	int pos = 0;
	int begin = 0, end = size_ - 1;
	while (begin <= end) {
		int mid = (begin + end) / 2;
		KeyPtr k = keyAt(mid);
		int res = keyComp(k, key);
		if (res < 0) begin = mid + 1;
		else if (res > 0) end = mid - 1;
		else end = mid - 1;
	}
	if (begin >= size_) return size_ - 1;
	if (begin == 0) return keyComp(keyAt(0), key) > 0 ? -1 : 0;
	return begin - 1;
}

BPlusNode::BPlusNode(IXFileHdr &hdr, PFPageHandle &page, bool load)
	: len_(hdr.len)
	, type_(hdr.type)
	, capacity_(hdr.capacity)
	, page_(page)
{
	// allocPage ���ڱ�ʾ���ҳ���Ƿ�Ϊ�·����
	// ��������
	//  һ��n��key,ռ�� n * len_�ֽڵĴ�С
	//  һ��n��rid,ռ�� n * sizeof(RID) �ֽڵĴ�С
	//  һ���ֽ����ڼ�¼һ���ж��ٸ�key,ռ��sizeof(numOfKeys_)�ֽڵĴ�С
	//  һ��left�ֶ�,ռ��sizeof(Page)�ֽڵĴ�С
	//  һ��right�ֶ�,ռ����sizeof(Page)�ֽڵĴ�С
	Ptr addr = page.rawPtr();
	num_ = page.page();

	keys_ = addr;
	rids_ = reinterpret_cast<RID *>(addr + len_ * capacity_);
	if (load) { // ˵�����ҳ��ʵ�����Ѿ���ʼ������
		size_ = size();
		left_ = left();
		right_ = right();
	}
	else { // ������Ҫ���³�ʼ��
		size_ = 0;
		setSize(0);
		setLeft(-1);
		setRight(-1);
		page.setDirty();
	}
}


//
// numberOfKeys - ��ȡ������Ŀ
// 
int BPlusNode::size()
{
	int *size = reinterpret_cast<int*>(reinterpret_cast<char *>(rids_) + 
		sizeof(RID) * capacity_);
	return *size;
}

//
// setNumOfKeys - �趨������Ŀ
// 
void BPlusNode::setSize(int size)
{
	int *dst = reinterpret_cast<int*>(reinterpret_cast<char*>(rids_) + 
		sizeof(RID) * capacity_);
	*dst = size;
	page_.setDirty();
}

//
// leftPage - ��ȡ��������ҳ��
// 
Page BPlusNode::left()
{
	Page *dst = reinterpret_cast<Page *>(reinterpret_cast<char *>(rids_) + 
		sizeof(RID) * capacity_ + sizeof(int));
	return *dst;
}

//
// rightPage - ��ȡ��������ҳ��
// 
Page BPlusNode::right()
{
	Page *dst = reinterpret_cast<Page *>(reinterpret_cast<char *>(rids_) + 
		sizeof(RID) * capacity_ + sizeof(int) + sizeof(Page));
	return *dst;
}

void BPlusNode::setLeft(Page num)
{
	Page *dst = reinterpret_cast<Page *>(reinterpret_cast<char *>(rids_) + 
		sizeof(RID) * capacity_ + sizeof(int));
	*dst = num;
	left_ = num;
	page_.setDirty();
}

void BPlusNode::setRight(Page num)
{
	Page *dst = reinterpret_cast<Page *>(reinterpret_cast<char *>(rids_) + 
		sizeof(RID) * capacity_ + sizeof(int) + sizeof(Page));
	*dst = num;
	right_ = num;
	page_.setDirty();
}

//
// keyAt - ��ȡrhs�ĵ�ַ
// 
KeyPtr BPlusNode::keyAt(int idx) const
{
	if ((idx < 0) || (idx >= capacity_)) return nullptr;
	return keys_ + len_ * idx;
}


//
// updateKey - ����key������,ָ��idx��ֵ
// 
bool BPlusNode::updateKey(int idx, KeyPtr key)
{
	if ((idx < 0) || (idx >= capacity_)) return false;
	KeyPtr dst = keys_ + len_ * idx;
	memcpy(dst, key, len_);
	page_.setDirty();
	return true;
}

//
// updateRid - ����rid������ָ��idx��ֵ
// 
bool BPlusNode::updateRid(int idx, RID rid)
{
	if ((idx < 0) || (idx >= capacity_)) return false;
	rids_[idx] = rid;
	page_.setDirty();
	return true;
}

void BPlusNode::copyKey(int idx, KeyPtr key)
{
	KeyPtr src = keyAt(idx);
	memcpy(key, src, len_);
}

//
// insert - ��һ��������itemװ��Node����
// 
bool BPlusNode::insert(const KeyPtr key, const RID& rid)
{
	int i = -1;
	KeyPtr prev = nullptr, curr = nullptr;
	// �Ӻ���ǰɨ��
	for (i = size_ - 1; i >= 0; i--)
	{
		prev = curr;
		curr = keyAt(i);
		if (keyComp(key, curr) > 0) break;
		rids_[i + 1] = rids_[i];
		updateKey(i + 1, curr);
	}
	rids_[i + 1] = rid;
	updateKey(i + 1, key);
	size_++;
	setSize(size_);
	page_.setDirty();
	return true;
}

//
// insert
//
bool BPlusNode::insert(int idx, KeyPtr key, const RID& rid)
{
	if ((idx < 0) || (idx > size_)) return false;
	int count = size_  - idx;
	memmove(keyAt(idx + 1), keyAt(idx), count * len_);
	memmove(&rids_[idx + 1], &rids_[idx], count * sizeof(RID));
	memcpy(keyAt(idx), key, len_);
	updateRid(idx, rid);
	size_++;
	setSize(size_);
	page_.setDirty();
	return true;
}

bool BPlusNode::erase(int idx)
{
	if ((idx < 0) || (idx >= size_)) return false;
	int size = size_ - idx - 1;
	if (idx != size_ - 1) {
		memmove(keyAt(idx), keyAt(idx + 1), size * len_);
		memmove(&rids_[idx], &rids_[idx + 1], size * sizeof(RID));
	}
	size_--;
	setSize(size_);
	page_.setDirty();
	return true;
}


RID BPlusNode::ridAt(int idx)
{
	if ((idx < 0) || (idx >= size_)) return RID(-1, -1);
	return rids_[idx];
}

int BPlusNode::keyComp(const KeyPtr lhs, const KeyPtr rhs)
{
	if (type_ == STRING) return memcmp(lhs, rhs, size_);
	if (type_ == FLOAT) {
		float f1 = *reinterpret_cast<float *>(const_cast<void *>(lhs));
		float f2 = *reinterpret_cast<float *>(const_cast<void *>(rhs));
		if (f1 > f2) return 1;
		if (f1 < f2) return -1;
		return 0;
	}

	if (type_ == INT) {
		int i1 = *reinterpret_cast<int *>(const_cast<void *>(lhs));
		int i2 = *reinterpret_cast<int *>(const_cast<void *>(rhs));
		if (i1 > i2) return 1;
		if (i1 < i2) return -1;
		return 0;
	}
	assert(0);
	return 0;
}


//
// lowerBound - �ڱ�node�ڵ��Ѱ�Һ�keyһ�µ�����ߵļ����±�
//
int BPlusNode::lowerBound(KeyPtr key)
{
	KeyPtr k;
	int begin = 0, end = size_ - 1;
	while (begin <= end) {
		int mid = (begin + end) / 2;
		k = keyAt(mid);
		int res = keyComp(k, key);
		if (res < 0) {
			begin = mid + 1;
		}
		else if (res > 0) {
			end = mid - 1;
		} 
		else { // k == key
			end = mid - 1;
		}
	}
	if (begin >= size_) return -1;
	return keyComp(keyAt(begin), key) == 0 ? begin : -1;
}

//
// leftmostInsertPos - ����key�������Ĳ���λ��
//
int BPlusNode::leftmostInsertPos(KeyPtr key)
{
	int pos = 0;
	KeyPtr k;
	int begin = 0, end = size_ - 1;
	while (begin <= end) {
		int mid = (begin + end) / 2;
		k = keyAt(mid);
		int res = keyComp(k, key);
		if (res < 0) {
			begin = mid + 1;
		}
		else if (res > 0) {
			end = mid - 1;
		}
		else { // k == key
			end = mid - 1;
		}
	}
	return end + 1;
}
