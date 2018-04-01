#include "Project.h"

CDirective::CDirective(onlyWhenTrackingIDs(char* pObjType))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
CDirective::~CDirective() {}
CDirectiveReplace::CDirectiveReplace(
	Identifier* pToReplace, Array<string>* pInput, AbstractCodeBlock* pReplacement, SourceFile* pOwningFile)
: CDirective(onlyWhenTrackingIDs("DTVRPLC"))
, toReplace(pToReplace)
, input(pInput)
, replacement(pReplacement)
, owningFile(pOwningFile)
, inUse(false) {
}
CDirectiveReplace::~CDirectiveReplace() {
	delete toReplace;
	delete input;
	delete replacement;
	//don't delete the parent file
}
CDirectiveInclude::CDirectiveInclude(string fullPath)
: CDirective(onlyWhenTrackingIDs("DTVINCL"))
, path(new IncludedPath(fullPath.c_str())) {
}
CDirectiveInclude::~CDirectiveInclude() {
	delete path;
}
