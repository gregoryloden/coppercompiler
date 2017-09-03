class CDirective onlyInDebug(: public ObjCounter) {
protected:
	CDirective(onlyWhenTrackingIDs(char* pObjType));
public:
	virtual ~CDirective();
};
class CDirectiveReplace: public CDirective {
public:
	CDirectiveReplace(Identifier* pToReplace, Array<string>* pInput, AbstractCodeBlock* pReplacement, SourceFile* pOwningFile);
	virtual ~CDirectiveReplace();

	Identifier* toReplace; //copper: private<readonly Replace>
	Array<string>* input; //copper: private<readonly Replace>
	AbstractCodeBlock* replacement; //copper: private<readonly Replace>
	SourceFile* owningFile; //copper: private<readonly Replace>
	bool inUse; //copper: private<Replace>
};
class CDirectiveInclude: public CDirective {
public:
	CDirectiveInclude(string pFilename, bool pIncludeAll);
	virtual ~CDirectiveInclude();

	string filename; //copper: private<readonly Include>
	bool includeAll; //copper: private<readonly Include>
};
