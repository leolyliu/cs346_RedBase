#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pf_internal.h"
#include "pf_buffer.h"
#include "pf_filehandle.h"
#include "util.h"
#include "pf_manager.h"
#include "pf_error.h"
using namespace RedBase;


RC PFManager::createFile(const char* pathname)
{
	int fd, n;
	// O_CREAT ����򿪵��ļ������ڵĻ������Զ��������ļ�
	// O_EXCL ���O_CREATҲ������, ��ָ���ȥ����ļ��Ƿ����. �ļ����������������ļ�, 
	// ���򽫵��´��ļ�����.
	// O_WRONLY ��ֻд�ķ�ʽ���ļ�
	fd = Open(pathname, O_CREAT | O_EXCL | O_WRONLY, CREATION_MASK);
	char hdrBuf[PF_FILE_HDR_SIZE];
	memset(hdrBuf, 0, PF_FILE_HDR_SIZE);

	PFFileHdr *hdr_ = (PFFileHdr*)hdrBuf;
	hdr_->free = PF_PAGE_LIST_END;
	hdr_->size = 0;

	// ��ͷ��д�뵽�ļ���
	n = Write(fd, hdrBuf, PF_FILE_HDR_SIZE);
	if (n != PF_FILE_HDR_SIZE) {
		Close(fd);
		Unlink(pathname); // ɾ���ļ�
		if (n < 0) return PF_UNIX;
		else return PF_HDRWRITE;
	}
	Close(fd);
	return 0; // һ�ж�OK
}

//
// destroyFile - ɾ�����ļ�
// 
RC PFManager::destroyFile(const char *pathname)
{
	Unlink(pathname);
	return 0;
}

//
// openFile - ��ĳ���ļ�,�����ļ���ͷ����Ϣ����,д�뵽PFFileHandle���ʵ����
// 
RC PFManager::openFile(const char* pathname, PFFilePtr &file)
{
	RC rc;
	file = make_shared<PFFileHandle>();
	file->fd_ = Open(pathname, O_RDWR);

	int n = Read(file->fd_, (Ptr)&file->hdr_, sizeof(PFFileHdr));
	if (n != sizeof(PFFileHdr)) {
		Close(file->fd_);
		return PF_HDRREAD;
	}
	file->changed_ = false;
	file->opened_ = true;
	return 0;
}

//
// closeFile - �رյ��ļ�
// 
RC PFManager::closeFile(PFFilePtr &file)
{
	// ��buffer�еĶ���ˢ�µ�������
	int fd = file->fd_;
	file.reset();
	// �ر��ļ�
	Close(fd);
	return 0;
}


