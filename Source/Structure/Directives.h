#include "../General/globals.h"

class IncludedPath;

class CDirective onlyInDebug(: public ObjCounter) {
protected:
	CDirective(onlyWhenTrackingIDs(char* pObjType));
public:
	virtual ~CDirective();
};
class CDirectiveReplace: public CDirective {
public:
	Identifier* toReplace; //copper: private<readonly Replace>
	Array<string>* input; //copper: private<readonly Replace>
	AbstractCodeBlock* replacement; //copper: private<readonly Replace>
	SourceFile* owningFile; //copper: private<readonly Replace>
	bool inUse; //copper: private<Replace>

	CDirectiveReplace(Identifier* pToReplace, Array<string>* pInput, AbstractCodeBlock* pReplacement, SourceFile* pOwningFile);
	virtual ~CDirectiveReplace();
};
class CDirectiveInclude: public CDirective {
public:
	IncludedPath* path; //copper: private<readonly Include>

	CDirectiveInclude(string fullPath);
	virtual ~CDirectiveInclude();
};
