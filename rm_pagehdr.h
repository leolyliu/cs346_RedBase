#ifndef RM_PAGEHDR_H
#define RM_PAGEHDR_H
#include "pf.h"
#include "redbase.h"

class BitMap
{
public:
	BitMap(uint slots, Ptr addr)
		: buffer_(addr)
		, bytes_(bytes(slots))
		, capacity_(slots)
	{}
	~BitMap() {}
public:
	//
	// reset - ��posλ���ϵ�bit��Ϊ0
	// 
	bool reset(uint pos)
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		buffer_[byte] &= ~(1 << offset);
		return true;
	}

	//
	// set - ����λ��,����Ӧ��bitΪ�趨Ϊ1
	// 
	bool set(uint pos)
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		buffer_[byte] |= (1 << offset);
		return true;
	}

	//
	// setAll - �����еı��ض�����Ϊ1
	//
	void setAll()
	{
		for (uint i = 0; i < bytes_; i++) {
			buffer_[i] = 0xff;
		}
	}

	//
	// resetAll - �����еı��ض�����Ϊ0
	// 
	void resetAll()
	{
		for (uint i = 0; i < bytes_; i++)
		{
			buffer_[i] = 0x00;
		}
	}

	//
	// available - ���Ը�����λ���Ƿ�Ϊ1,Ҳ����λ���Ƿ����
	// 
	bool available(uint pos) const
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		return buffer_[byte] & (1 << offset);
	}
public:
	//
	// bytes - ����Ϊslots������slot,��Ҫ���ĵ��ֽ���Ŀ
	// 
	uint static bytes(uint slots)
	{
		uint n = slots / 8;
		if (slots % 8 != 0) n++;
		return n;
	}

	uint size()
	{
		return bytes_;
	}
private:
	Ptr buffer_;
	uint capacity_;
	uint bytes_;
};

//
// RMPageHdr - ҳ���ײ�,�����ǵ�ʵ����,ÿһ��ҳ���ײ�����������������Ϣ
// 
class RMPageHdr {
public:
	RMPageHdr(uint slots, Ptr addr)
		: buffer_(addr)
		, slots_(slots)
		, map(slots, addr + sizeof(int) + 2 * sizeof(uint))
	{}
	~RMPageHdr() {}
public:
	int lenOfHdr()
	{
		return sizeof(int) + 2 * sizeof(uint) + map.size();
	}

	int next()
	{
		return *reinterpret_cast<int *>(buffer_);
	}

	uint slots()
	{
		return *reinterpret_cast<uint *>(buffer_ + sizeof(int));
	}

	uint remain()
	{
		return *reinterpret_cast<uint *>(buffer_ + sizeof(int) + sizeof(uint));
	}

	void setNext(int val)
	{
		*reinterpret_cast<int *>(buffer_) = val;
	}

	void setSlots(uint val)
	{
		*reinterpret_cast<uint *>(buffer_ + sizeof(int)) = val;
	}

	void setRemain(uint val)
	{
		*reinterpret_cast<uint *>(buffer_ + sizeof(int) + sizeof(uint)) = val;
	}
public:
	BitMap map;
private:
	uint slots_;
	Ptr buffer_;
};


#endif /* RM_PAGEHDR_H */