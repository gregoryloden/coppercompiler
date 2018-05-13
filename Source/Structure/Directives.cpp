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
	//don't delete the owning file
}
CDirectiveInclude::CDirectiveInclude(StringLiteral* pPathName)
: CDirective(onlyWhenTrackingIDs("DTVINCL"))
, pathName(pPathName)
, path(nullptr) {
	//create a path and then flip it so that we can parse it in reverse
	Path* reversedPath = Path::createPath(pPathName->val);
	Path* forwardPath = nullptr;
	while (reversedPath != nullptr) {
		Path* tempPath = reversedPath;
		reversedPath = reversedPath->parentDirectory;
		tempPath->parentDirectory = forwardPath;
		forwardPath = tempPath;
	}
	path = forwardPath;
}
CDirectiveInclude::~CDirectiveInclude() {
	path->deleteFullPath();
	delete pathName;
}
