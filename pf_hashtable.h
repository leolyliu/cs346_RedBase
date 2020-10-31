//
// File:		pf_hashtable.h
// Description: PFHashTable class interface
//

#ifndef PF_HASHTABLE_H
#define PF_HASHTABLE_H

#include "pf_internal.h"
#include <list>
#include <vector>
using namespace std;


//
// PFHashTable - һ���ǳ����׵�hash��ʵ��,����������˵,�򵥵���Щ����˼��,
// ���table��¼��һЩʲô��?���������ʵ����һ��map,��¼��fdָ��������ļ��ĵ�num��ҳ��
// �������Ǹ�slot����.
//
class PFHashTable {
	struct Triple {
		int fd;
		Page num;
		int slot;
		Triple(int fd, Page num, int slot) : fd(fd), num(num), slot(slot) {}
	};
public:
	PFHashTable(uint capacity);
	~PFHashTable() {}
public:
	int search(int fd, Page num);
	bool insert(int fd, Page num, int slot);
	bool remove(int fd, Page num);
private:
	int calcHash(int fd, Page num)
	{
		return (fd + num) % capacity_;
	}
private:
	uint capacity_;
	vector<list<Triple>> table_;
};

#endif /* PF_HASHTABLE_H */