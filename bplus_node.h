#ifndef BTREE_NODE_H
#define BTREE_NODE_H
#include <tr1/memory>
#include "redbase.h"
#include "pf.h"
#include "rm_rid.h"
#include "ix_error.h"
#include "pf_pagehandle.h"
using namespace std;

struct IXFileHdr;
class BPlusNode;
using NodePtr = shared_ptr<BPlusNode>;

class BPlusNode {
	friend class BtreeNodeTest;
	friend class IXIndexHandle;
	friend class IXIndexScan;
	using KeyPtr = void *;
public:
	BPlusNode(IXFileHdr &hdr, PFPageHandle& page, bool load);
	~BPlusNode() {};
public:
	Page left();
	Page right();
	void setLeft(Page num);
	void setRight(Page num);

	RID ridAt(int pos);
	int keyComp(const KeyPtr lhs, const KeyPtr rhs);
public:
	int lowerBound(KeyPtr key);
	int leftmostInsertPos(KeyPtr key);
	int leftmostSearchPos(KeyPtr key);
	int search(KeyPtr key, const RID &rid);
public:
	bool erase(int idx);
	KeyPtr keyAt(int pos) const;
	bool updateKey(int idx, KeyPtr src);
	void copyKey(int idx, KeyPtr key);
	bool updateRid(int idx, RID rid);
	bool insert(const KeyPtr dst, const RID& rid);
	bool insert(int idx, KeyPtr key, const RID& rid);
private:
	int size();
	void setSize(int size);
private:
	// for test
	Page left_;
	Page right_;

	PFPageHandle page_;
	char *keys_;
	RID *rids_;
	uint size_;		/* �Ѿ�ʹ���˵ļ�����Ŀ */
	uint capacity_; /* ���� */
	uint len_;		/* ÿһ��key�ĳ��� */
	AttrType type_; /* key������ */
	Page num_;		/* ���ڼ�¼ҳ�� */
};

#endif /* BTREE_NODE_H */