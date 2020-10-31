#ifndef SM_ATTR_H
#define SM_ATTR_H


struct AttrInfo {
	char* attrname;		// ���Ե�����
	AttrType type;		// ���Ե�����
	int len;			// ���Եĳ���
};

//
// DataAttr - ��Ҫ�����������е�����
//
struct DataAttr {
	char relname[MAXNAME + 1];		// ��ϵ������,����˵��table������
	char attrname[MAXNAME + 1];		// ���Ե�����
	int offset;						// ƫ����
	AttrType type;					// ����
	int len;						// ����
	int idxno;						// ������������Ļ�,�����ı��

public:
	DataAttr()
		: offset(-1)
	{
		memset(relname, 0, MAXNAME + 1);
		memset(attrname, 0, MAXNAME + 1);
	}

	DataAttr(const AttrInfo& info)
		: type(info.type), len(info.len)
		, idxno(-1), offset(-1)
	{
		memcpy(attrname, info.attrname, MAXNAME + 1);
	}

	DataAttr(const DataAttr& rhs)
		: type(rhs.type), len(rhs.len)
		, offset(rhs.offset), idxno(rhs.idxno)
	{
		// tofix
		memcpy(attrname, rhs.attrname, MAXNAME + 1);
		memcpy(relname, rhs.relname, MAXNAME + 1);
	}
	
	DataAttr& operator=(const DataAttr& rhs)
	{
		if (this != &rhs) {
			strcpy(relname, rhs.relname);
			strcpy(attrname, rhs.attrname);
			offset = rhs.offset;
			idxno = rhs.idxno;
			len = rhs.len;
			type = rhs.type;
		}
		return *this;
	}
};

#endif /* SM_ATTR_H */