#include "Project.h"

PrefixTrie<char, CType*>* CType::globalTypes = []() -> PrefixTrie<char, CType*>* {
	CType* typesArray[] = {
		new CVoid(),
		new CIntegerPrimitive("bool", 1),
		new CIntegerPrimitive("byte", 8),
		new CIntegerPrimitive("short", 16),
		new CIntegerPrimitive("int", 32),
		new CFunctionType(),
		new CClass("String")
	};
	PrefixTrie<char, CType*>* val = new PrefixTrie<char, CType*>();
	for (int i = 0; i < 7; i++) {
		CType* ct = typesArray[i];
		val->set(ct->name.c_str(), ct->name.length(), ct);
	}
	return val;
}();
CType::CType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
name(pName) {
}
CType::~CType() {}
CVoid::CVoid()
: CType(onlyWhenTrackingIDs("VOIDTYP" COMMA) "void") {
}
CVoid::~CVoid() {}
CPrimitive::CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize)
: CType(onlyWhenTrackingIDs(pObjType COMMA) pName)
, bitSize(pBitSize) {
}
CPrimitive::~CPrimitive() {}
CIntegerPrimitive::CIntegerPrimitive(string pName, int pByteSize)
: CPrimitive(onlyWhenTrackingIDs("INTPMTP" COMMA) pName, pByteSize) {
}
CIntegerPrimitive::~CIntegerPrimitive() {}
CFloatingPointPrimitive::CFloatingPointPrimitive(string pName, int pByteSize)
: CPrimitive(onlyWhenTrackingIDs("FLTPMTP" COMMA) pName, pByteSize) {
}
CFloatingPointPrimitive::~CFloatingPointPrimitive() {}
CFunctionType::CFunctionType()
: CType(onlyWhenTrackingIDs("FUNCTYP" COMMA) "Function") {
}
CFunctionType::~CFunctionType() {}
CClass::CClass(string pName)
: CType(onlyWhenTrackingIDs("CLSSTYP" COMMA) pName) {
}
CClass::~CClass() {}
