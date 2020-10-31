#include "pf_internal.h"
#include "pf_pagehandle.h"
#include "pf_filehandle.h"
#include "pf_error.h"


PFPageHandle::~PFPageHandle()
{
	// һ�����,file���������ڳ���page
	// PFPageHandle����һ������,����Ҫunpin
	shared_ptr<PFFileHandle> file = pffile_.lock();
	if (file) {
		file->unpin(num_);
	}
}

// 
// operator= - ��ֵ�����
// 
PFPageHandle& PFPageHandle::operator=(const PFPageHandle& rhs)
{
	if (this != &rhs) {
		// ����Ҫ������ԭ��ҳ�������
		PFFilePtr file = pffile_.lock();
		if (file) {
			file->unpin(num_);
		}
		num_ = rhs.num_;
		addr_ = rhs.addr_;
		file = rhs.pffile_.lock();
		// ��ֵ��ʱ�����Ҫ����Ӧ�ü���
		if (file) {
			file->pin(num_);
		}
		pffile_ = file;
	}
	return *this;
}

//
// PFPageHandle - �������캯��
//
PFPageHandle::PFPageHandle(const PFPageHandle& rhs)
	: pffile_(rhs.pffile_)
	, num_(rhs.num_)
	, addr_(rhs.addr_)
{
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->pin(num_);
	}
}

//
// setDirty - ��ҳ������Ϊ��ҳ��
//
void PFPageHandle::setDirty()
{
	if (dirty_) return;
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->markDirty(num_);
	}
}

//
// dispose - �ͷ�ҳ��
//
void PFPageHandle::dispose()
{
	PFFilePtr file = pffile_.lock();
	if (file) {
		file->disposePage(num_);
	}
}
