#include "Project.h"

PrefixTrie<char, CType*>* CType::globalTypes = []() -> PrefixTrie<char, CType*>* {
	PrefixTrie<char, CType*>* val = new PrefixTrie<char, CType*>();
	val->set("void", 4, new CVoid());
	return val;
}();
CType::CType(onlyWhenTrackingIDs(char* pObjType COMMA))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
CType::~CType() {}
CVoid::CVoid()
: CType(onlyWhenTrackingIDs("VOIDTYP")) {
}
CVoid::~CVoid() {}
