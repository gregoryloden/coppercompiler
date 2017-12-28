#include "Project.h"

//classes defined within other classes have owning class

CFunctionType* CDataType::functionType = new CFunctionType();
CVoid* CDataType::voidType = new CVoid();
PrefixTrie<char, CDataType*>* CDataType::globalDataTypes = []() -> PrefixTrie<char, CDataType*>* {
	CDataType* typesArray[] = {
		voidType,
		new CIntegerPrimitive("bool", 1),
		new CIntegerPrimitive("byte", 8),
		new CIntegerPrimitive("short", 16),
		new CIntegerPrimitive("int", 32),
		new CFloatingPointPrimitive("float", 32),
		functionType,
		new CClass("String")
	};
	const int typesArrayCount = sizeof(typesArray) / sizeof(typesArray[0]);
	PrefixTrie<char, CDataType*>* val = new PrefixTrie<char, CDataType*>();
	for (int i = 0; i < typesArrayCount; i++) {
		CDataType* cdt = typesArray[i];
		val->set(cdt->name.c_str(), cdt->name.length(), cdt);
	}
	return val;
}();
CDataType::CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
name(pName) {
}
CDataType::~CDataType() {}
CVoid::CVoid()
: CDataType(onlyWhenTrackingIDs("VOIDTYP" COMMA) "void") {
}
CVoid::~CVoid() {}
CPrimitive::CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize)
: CDataType(onlyWhenTrackingIDs(pObjType COMMA) pName)
, bitSize(pBitSize) {
}
CPrimitive::~CPrimitive() {}
CIntegerPrimitive::CIntegerPrimitive(string pName, int pBitSize)
: CPrimitive(onlyWhenTrackingIDs("INTPMTP" COMMA) pName, pBitSize) {
}
CIntegerPrimitive::~CIntegerPrimitive() {}
CFloatingPointPrimitive::CFloatingPointPrimitive(string pName, int pBitSize)
: CPrimitive(onlyWhenTrackingIDs("FLTPMTP" COMMA) pName, pBitSize) {
}
CFloatingPointPrimitive::~CFloatingPointPrimitive() {}
CFunctionType::CFunctionType()
: CDataType(onlyWhenTrackingIDs("FUNCTYP" COMMA) "Function") {
}
CFunctionType::~CFunctionType() {}
CClass::CClass(string pName)
: CDataType(onlyWhenTrackingIDs("CLSSTYP" COMMA) pName) {
}
CClass::~CClass() {}
