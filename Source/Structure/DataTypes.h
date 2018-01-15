#include "../General/globals.h"
#include "string"
using namespace std;

template <class Key, class Value> class PrefixTrie;
template <class Type> class Array;

class CDataType onlyInDebug(: public ObjCounter) {
public:
	static PrefixTrie<char, CDataType*>* globalDataTypes;
private:
	static Array<CDataType*>* typesToDelete;

public:
	string name;

protected:
	CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName);
public:
	virtual ~CDataType();

	static void initializeGlobalDataTypes();
	static void deleteGlobalDataTypes();
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
class CGenericFunction: public CDataType {
public:
	CGenericFunction();
	virtual ~CGenericFunction();
};
class CSpecificFunction: public CDataType {
private:
	CDataType* returnType;
	Array<CDataType*>* parameterTypes;

public:
	CSpecificFunction(string pName, CDataType* pReturnType, Array<CDataType*>* pParameterTypes);
	virtual ~CSpecificFunction();
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
