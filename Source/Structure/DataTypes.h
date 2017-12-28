#include "../General/globals.h"
#include "string"
using namespace std;

class CFunctionType;
class CVoid;
template <class Key, class Value> class PrefixTrie;

class CDataType onlyInDebug(: public ObjCounter) {
public:
	static CFunctionType* functionType;
	static CVoid* voidType;
	static PrefixTrie<char, CDataType*>* globalDataTypes;

	string name;

protected:
	CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName);
public:
	virtual ~CDataType();
};
class CVoid: public CDataType {
public:
	CVoid();
	virtual ~CVoid();
};
class CPrimitive: public CDataType {
public:
	short bitSize; //copper: readonly

protected:
	CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize);
public:
	virtual ~CPrimitive();
};
class CIntegerPrimitive: public CPrimitive {
public:
	CIntegerPrimitive(string pName, int pBitSize);
	virtual ~CIntegerPrimitive();
};
class CFloatingPointPrimitive: public CPrimitive {
public:
	CFloatingPointPrimitive(string pName, int pBitSize);
	virtual ~CFloatingPointPrimitive();
};
class CFunctionType: public CDataType {
public:
	CFunctionType();
	virtual ~CFunctionType();
};
class CClass: public CDataType {
public:
	CClass(string pName);
	virtual ~CClass();
};
/*
class CEnum: public CDataType {
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
