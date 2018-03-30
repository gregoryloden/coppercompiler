#include "../General/globals.h"
#include "string"
using namespace std;

class CErrorType;
class CVoid;
class CBool;
class CIntegerPrimitive;
class CFloatingPointPrimitive;
class CGenericFunction;
class CClass;
class CVariableDefinition;
template <class Key, class Value> class PrefixTrie;
template <class Type> class Array;

//superclass types
class CDataType onlyInDebug(: public ObjCounter) {
public:
	static PrefixTrie<char, CDataType*>* globalDataTypes;
	static CErrorType* errorType;
	static CVoid* voidType;
	static CBool* boolType;
	static CIntegerPrimitive* infiniteByteSizeIntType;
	static CIntegerPrimitive* byteType;
	static CIntegerPrimitive* shortType;
	static CIntegerPrimitive* intType;
	static CFloatingPointPrimitive* infinitePrecisionFloatType;
	static CFloatingPointPrimitive* floatType;
	static CGenericFunction* functionType;
	static CDataType* emptyGroupType;
	static CClass* stringType;
	static CClass* mainType;

	string name;

protected:
	CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName);
public:
	virtual ~CDataType();

	static void initializeGlobalDataTypes();
	static void deleteGlobalDataTypes();
	static CDataType* bestCompatibleType(CDataType* type1, CDataType* type2);
};
class CPrimitive: public CDataType {
public:
	short bitSize; //copper: readonly

protected:
	CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize);
public:
	virtual ~CPrimitive();
};
class CNumericPrimitive: public CPrimitive {
protected:
	CNumericPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize);
public:
	virtual ~CNumericPrimitive();
};
class CGenericPointerType: public CDataType {
protected:
	CGenericPointerType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName);
public:
	virtual ~CGenericPointerType();
};

//concrete class types
class CErrorType: public CDataType {
public:
	CErrorType();
	virtual ~CErrorType();
};
class CVoid: public CDataType {
public:
	CVoid();
	virtual ~CVoid();
};
class CBool: public CPrimitive {
public:
	CBool();
	virtual ~CBool();
};
class CIntegerPrimitive: public CNumericPrimitive {
public:
	CIntegerPrimitive(string pName, short pBitSize);
	virtual ~CIntegerPrimitive();
};
class CFloatingPointPrimitive: public CNumericPrimitive {
public:
	CFloatingPointPrimitive(string pName, short pBitSize);
	virtual ~CFloatingPointPrimitive();
};
class CGenericFunction: public CGenericPointerType {
public:
	CGenericFunction();
	virtual ~CGenericFunction();

	static CDataType* typeFor(CDataType* returnType, Array<CDataType*>* parameterTypes);
};
class CSpecificFunction: public CDataType {
public:
	CDataType* returnType; //copper: readonly
	Array<CDataType*>* parameterTypes; //copper: readonly

	CSpecificFunction(string pName, CDataType* pReturnType, Array<CDataType*>* pParameterTypes);
	virtual ~CSpecificFunction();
};
class CClass: public CDataType {
public:
	CClass(string pName);
	virtual ~CClass();
};
class CGenericGroup: public CGenericPointerType {
public:
	CGenericGroup();
	virtual ~CGenericGroup();

	static CDataType* typeFor(Array<CVariableDefinition*>* types);
};
class CSpecificGroup: public CDataType {
public:
	Array<CVariableDefinition*>* types; //copper: readonly
	bool allSameType; //copper: readonly

	CSpecificGroup(string pName, Array<CVariableDefinition*>* pTypes);
	virtual ~CSpecificGroup();
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
