#include "Project.h"

PrefixTrie<char, CType*>* CType::globalTypes = []() -> PrefixTrie<char, CType*>* {
	PrefixTrie<char, CType*>* val = new PrefixTrie<char, CType*>();
	val->set("void", 4, new CVoid());
	val->set("bool", 4, new CIntegerPrimitive(1));
	val->set("byte", 4, new CIntegerPrimitive(8));
	val->set("short", 5, new CIntegerPrimitive(16));
	val->set("int", 3, new CIntegerPrimitive(32));
	val->set("Function", 8, new CFunctionType());
	val->set("String", 6, new CClass("String"));
	return val;
}();
CType::CType(onlyWhenTrackingIDs(char* pObjType))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
CType::~CType() {}
CVoid::CVoid()
: CType(onlyWhenTrackingIDs("VOIDTYP")) {
}
CVoid::~CVoid() {}
CPrimitive::CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) short pBitSize)
: CType(onlyWhenTrackingIDs(pObjType))
, bitSize(pBitSize) {
}
CPrimitive::~CPrimitive() {}
CIntegerPrimitive::CIntegerPrimitive(int pByteSize)
: CPrimitive(onlyWhenTrackingIDs("INTPMTP" COMMA) pByteSize) {
}
CIntegerPrimitive::~CIntegerPrimitive() {}
CFloatingPointPrimitive::CFloatingPointPrimitive(int pByteSize)
: CPrimitive(onlyWhenTrackingIDs("FLTPMTP" COMMA) pByteSize) {
}
CFloatingPointPrimitive::~CFloatingPointPrimitive() {}
CFunctionType::CFunctionType()
: CType(onlyWhenTrackingIDs("FUNCTYP")) {
}
CFunctionType::~CFunctionType() {}
CClass::CClass(string pName)
: CType(onlyWhenTrackingIDs("CLSSTYP"))
, name(pName) {
}
CClass::~CClass() {}
