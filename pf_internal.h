//
// File:		pf_internal.h
// Description:	Declarations internal to the paged file component
//

#ifndef PF_INTERNAL_H
#define PF_INTERNAL_H

#include <cstdlib>
#include <cstring>
#include "pf.h"

const int PF_BUFFER_SIZE = 40;     // Number of pages in the buffer
const int PF_HASH_TBL_SIZE = 20;   // Size of hash table

// Constants and defines
#define CREATION_MASK		0600		// r/w privileges to owner only
#define PF_PAGE_LIST_END	-1			// end of list of free_ pages
#define PF_PAGE_USED		-2			// Page is being used


#ifndef L_SET
#define L_SET              0
#endif

//
// PageHdr: Header structure for pages
//
struct PFPageHdr {
	int free;		// free���������¼���ȡֵ:
					// - ��һ������ҳ�ı��,��ʱ��ҳ��Ҳ�ǿ��е�
					// - PF_PAGE_LIST_END => ҳ�������һ������ҳ��
					// - PF_PAGE_USED => ҳ�沢���ǿ��е�
};

// Justify the file header to the length of one Page
const int PF_FILE_HDR_SIZE = PF_PAGE_SIZE + sizeof(PFPageHdr);

#endif /* PF_INTERNAL_H */