#include "../General/globals.h"
#include "string"
using namespace std;

template <class Key, class Value> class PrefixTrie;

class CType onlyInDebug(: public ObjCounter) {
protected:
	CType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName);
public:
	virtual ~CType();

	static PrefixTrie<char, CType*>* globalTypes;
	string name;
};
class CVoid: public CType {
public:
	CVoid();
	virtual ~CVoid();
};
class CPrimitive: public CType {
protected:
	CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize);
public:
	virtual ~CPrimitive();

	short bitSize; //copper: readonly
};
class CIntegerPrimitive: public CPrimitive {
public:
	CIntegerPrimitive(string pName, int pByteSize);
	virtual ~CIntegerPrimitive();
};
class CFloatingPointPrimitive: public CPrimitive {
public:
	CFloatingPointPrimitive(string pName, int pByteSize);
	virtual ~CFloatingPointPrimitive();
};
class CFunctionType: public CType {
public:
	CFunctionType();
	virtual ~CFunctionType();
};
class CClass: public CType {
public:
	CClass(string pName);
	virtual ~CClass();
};
/*
class CEnum: public CType {
public:
	CEnum();
	virtual ~CEnum();
};
class CGenericPrimitive: public CPrimitive {
public:
	CGenericPrimitive();
	virtual ~CGenericPrimitive();
};
class CGenericClass: public CClass {
public:
	CGenericClass();
	virtual ~CGenericClass();
};
*/
