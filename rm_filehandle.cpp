#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include "rm.h"
#include "rm_error.h"
#include "rm_filehandle.h"
#include "rm_pagehdr.h"
using namespace std;


//
// CalcSlotsSize - ����һ�����ݿ������֧�ֵĴ洢�ļ�¼������
// 
uint static CalcSlotsCapacity(uint len)
{
	uint remain = PF_PAGE_SIZE - sizeof(RMPageHdrEx); // ʣ����õ��ֽ���Ŀ
	// floor - ����ȡ��
	// ÿһ����¼����Ҫ1��bit,Ҳ����1/8�ֽ�����ʾ�Ƿ��Ѿ���¼������
	uint slots = floor((1.0 * remain) / (len + 1 / 8));
	uint hdr_size = sizeof(RMPageHdrEx) + BitMap::bytes(slots);
	// ��������Ҫ���ϵ���
	while ((slots * len + hdr_size) > PF_PAGE_SIZE) {
		slots--;
		hdr_size = sizeof(RMPageHdrEx) + BitMap::bytes(slots);
	}
	return slots;
}


RMFileHandle::RMFileHandle(PFFilePtr file) 
	: changed_(false)
{
	pffile_ = file;
	PFPageHandle page = PFGetPage(file, 0);
	Ptr buff = page.rawPtr();
	memcpy(&rmhdr_, buff, sizeof(RMFileHdr));
	rcdlen_ = rmhdr_.rcdlen;
	capacity_ = CalcSlotsCapacity(rcdlen_);
}

RMFileHandle::~RMFileHandle()
{
	if (changed_) {
		PFPageHandle page = PFGetPage(pffile_, 0);
		Ptr buff = page.rawPtr();
		memcpy(buff, &rmhdr_, sizeof(RMFileHdr));
		page.setDirty();
	}
}


//
// isValidPage - �ж��Ƿ�Ϊ��Ч��ҳ��
// 
bool RMFileHandle::isValidPage(Page num) const
{
	return (num >= 0) && (num < rmhdr_.size);
}

bool RMFileHandle::isValidRID(const RID& rid) const
{
	Slot slot = rid.slot();
	return isValidPage(rid.page()) && slot >= 0;
}


//
// insertRcd - ����һ����¼
// 
RC RMFileHandle::insertRcd(const Ptr addr, RID &rid)
{
	if (addr == nullptr) return RM_NULLRECORD; // ָ�������Ϊ��
	PFPageHandle page;
	Page num;
	Slot slot;
	// ��һ��ӵ�п��е�pos�Ŀ���ҳ���л�ȡһ�����е�pos
	nextFreeSlot(page, num, slot);
	// ��ҳ��ͷ����ȡ����ҳ�����Ϣ
	Ptr ptr = page.rawPtr();
	RMPageHdr hdr(capacity_, ptr);
	uint offset = hdr.lenOfHdr() + slot * rcdlen_;
	ptr = ptr + offset;
	
	rid = RID(num, slot);	// rid���ڼ�¼�洢��λ��
	memcpy(ptr, addr, rcdlen_);
	hdr.map.reset(slot); // �趨Ϊ0,��ʾ�Ѿ���ʹ����
	hdr.setRemain(hdr.remain() - 1);
	if (hdr.remain() == 0) {
		rmhdr_.free = hdr.next();
		hdr.setNext(PAGE_FULLY_USED);
	}
	page.setDirty();
	return 0;
}

//
//  updateRcd ����һ����¼, rcd�м�¼�˼�¼�ڴ����е�λ��
// 
RC RMFileHandle::updateRcd(const RMRecord &rcd)
{
	RID rid;
	rcd.getRid(rid);
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();

	Ptr src;
	PFPageHandle page = PFGetPage(pffile_, num);
	Ptr dst = page.rawPtr();

	RMPageHdr hdr(capacity_, dst);
	if (hdr.map.available(pos)) return RM_NORECATRID;
	rcd.getData(src);
	dst = dst + hdr.lenOfHdr() + pos * rcdlen_;
	memcpy(dst, src, rcdlen_);
	page.setDirty();
	return 0;
}

RC RMFileHandle::deleteRcd(const RID &rid)
{
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();
	PFPageHandle page = PFGetPage(pffile_, num);	// ��ȡ��Ӧ��ҳ��
	Ptr addr = page.rawPtr();
	RMPageHdr hdr(capacity_, addr);

	if (hdr.map.available(pos)) return RM_NORECATRID; // ���Ѿ�Ϊfree
	hdr.map.set(pos); // ����Ӧ��pos����Ϊ0
	int remain = hdr.remain();
	if (hdr.remain() == 0) {
		hdr.setNext(rmhdr_.free);
		rmhdr_.free = num;
	}
	hdr.setRemain(hdr.remain() + 1);
	page.setDirty();
	return 0;
}


//
// getRcd - ��ȡһ����¼
// 
RC RMFileHandle::getRcd(const RID &rid, RMRecord &rcd)
{
	if (!isValidRID(rid)) return RM_BAD_RID;
	Page num = rid.page();
	Slot pos = rid.slot();
	PFPageHandle page = PFGetPage(pffile_, num);
	Ptr addr = page.rawPtr();
	RMPageHdr hdr(capacity_, addr);
	if (hdr.map.available(pos)) return RM_NORECATRID; // ��pos�ض��������
	addr = addr + hdr.lenOfHdr() + pos * rcdlen_;
	rcd.set(addr, rmhdr_.rcdlen, rid);		// �趨��¼
	return 0;
}

RC RMFileHandle::forcePages(Page num /* = ALL_PAGES */)
{
	if (!isValidPage(num) && num != ALL_PAGES)
		return RM_BAD_RID;
	return pffile_->forcePages(num);
}

//
// getNextFreeSlot - ��ȡ��һ�����е�pos,һ��ҳ������ܶ�pos
// 
bool RMFileHandle::nextFreeSlot(PFPageHandle& page, Page &num, Slot& slot)
{
	Ptr addr;
	if (rmhdr_.free > 0) { // ��Ȼ�п��е�ҳ��
		page = PFGetPage(pffile_, rmhdr_.free);
		num = rmhdr_.free;
		addr = page.rawPtr();
	}
	else { // ��Ҫ���·���ҳ��
		page = PFAllocPage(pffile_);
		addr = page.rawPtr();
		num = page.page();
		RMPageHdr hdr(capacity_, addr);
		hdr.setNext(PAGE_LIST_END);
		hdr.setRemain(capacity_);
		int remain = hdr.remain();
		hdr.map.setAll();
		page.setDirty();
		rmhdr_.free = num; // �������ҳ���ҳ����ӵ�����������
		rmhdr_.size++;
		changed_ = true;
	}
	
	RMPageHdr hdr(capacity_, addr);
	for (int i = 0; i < capacity_; i++) {
		if (hdr.map.available(i)) { // ��λ��ǡ�ÿ���
			slot = i;
			return true;
		}
	}
	return false; // unexpected error
}