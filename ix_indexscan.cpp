#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include "ix_indexscan.h"
#include "ix_indexhandle.h"
#include "bplus_node.h"
using namespace std;


IXIndexScan::~IXIndexScan()
{
	if (comp_) delete(comp_);
}

RC IXIndexScan::openScan(const IXIndexPtr &index, Operator op, const void* val)
{
	if (opened_) return IX_HANDLEOPEN;
	if (index_ == nullptr) return IX_FCREATEFAIL;
	op_ = op;
	val_ = val;
	index_ = index;
	opened_ = true;
	comp_ = make_comp(index->type(), index_->hdr_.len);
	/* ���ﲢ�����趨pCurr_��currPos_ */
	return 0;
}

RC IXIndexScan::getNextEntry(RID &rid)
{
	void* key = nullptr;
	return getNextEntry(key, rid);
}

//
// getNextEntry ���ڲ�����ɵ�İ취,�Ȳ����κ��Ż�����,ֱ�ӱ�������
//
RC IXIndexScan::getNextEntry(KeyPtr &key_, RID& rid)
{
	if (!opened_) return IX_FNOTOPEN;
	if (eof_) return IX_EOF;
	/* ��һ�ε���getNextEntry, curr_��pos_��û�б���ʼ���� */
	if ((curr_ == nullptr) && (pos_ == 0)) {
		curr_ = index_->loadNode(index_->leaf_);
	}

	while (curr_ != nullptr) {
		int size = curr_->size_;
		KeyPtr key = nullptr;
		for (int i = pos_; i < size; i++)
		{
			key = curr_->keyAt(i);
			if (comp_->eval(key, op_, val_)) {
				rid = curr_->ridAt(i);
				key_ = key;
				pos_ = i + 1;
				return 0;
			}
		}
		curr_ = index_->loadNode(curr_->right());
		pos_ = 0;
	}
	eof_ = true;
	return IX_EOF;
}

RC IXIndexScan::closeScan()
{
	if (!opened_) return IX_FNOTOPEN;
	opened_ = false;
	if (comp_ != nullptr) delete comp_;
	comp_ = nullptr;
	curr_ = nullptr;
	pos_ = -1;
	eof_ = false;
	return 0;
}

RC IXIndexScan::rewind()
{
	if (!opened_) return IX_FNOTOPEN;
	pos_ = 0;
	curr_ = nullptr;
	eof_ = false;
	return 0;
}

