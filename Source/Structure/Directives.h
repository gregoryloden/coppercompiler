#include "../General/globals.h"

class Path;
class AbstractCodeBlock;
class Identifier;
class StringLiteral;
class SourceFile;
template <class Type> class Array;

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
	StringLiteral* pathName; //copper: private<readonly Include>
	Path* path; //copper: private<readonly Include>

	CDirectiveInclude(StringLiteral* pPathName);
	virtual ~CDirectiveInclude();
};
