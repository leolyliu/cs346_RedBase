#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "rm_manager.h"
#include "pf_pagehandle.h"
#include "rm_filehandle.h"

extern PFManager pfManager;

RC RMManager::createFile(const char* pathname, uint rcdlen)
{
	if (rcdlen == 0) return RM_BADRECSIZE;
	if (rcdlen >= (PF_PAGE_SIZE - sizeof(RMPageHdr))) return RM_SIZETOOBIG;
	pfManager.createFile(pathname);
	PFFilePtr file;
	pfManager.openFile(pathname, file);
	PFPageHandle page = PFAllocPage(file);
	Ptr addr = page.rawPtr();
	
	// �ļ��ĵ�һ����ͷ����Ϣ
	RMFileHdr *hdr = (RMFileHdr *)addr;
	hdr->free = PAGE_LIST_END;	// ��0ҳ��������RM�ļ�ͷ,�������ʹ��.
	hdr->size = 1;				// �Ѿ������˵�ҳ����Ŀ
	hdr->rcdlen = rcdlen;		// ��¼�Ĵ�С
	page.setDirty();
	pfManager.closeFile(file);
	return 0;
}

RC RMManager::destroyFile(const char* pathname)
{
	return pfManager.destroyFile(pathname);
}

//
// openFile - ��ĳ���ļ�
// 
RC RMManager::openFile(const char* pathname, RMFilePtr &rmFile)
{
	PFFilePtr pfFile;
	RC errval;
	errval = pfManager.openFile(pathname, pfFile);
	if (errval != 0) return errval;
	rmFile = make_shared<RMFileHandle>(pfFile);
	return 0;
}

//
// closeFile ���ļ��ر�
// 
RC RMManager::closeFile(RMFilePtr& file)
{
	if (file == nullptr) return RM_FNOTOPEN;
	PFFilePtr pffile = file->pffile_;
	file.reset();
	pfManager.closeFile(pffile);
	return 0;
}