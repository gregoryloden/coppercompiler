#include "Project.h"

/*
Array<Expression*> tokens;
int tlength;
Array<Function*> functions;
Array<MainFunction*> mainfunctions;

bool warnings = false;
int* rows;
int* cols;

int codesize;
int rdataRVA;
int rdatasize;
int dataRVA;
int datasize;
string data (
	//4 all-purpose variables
	"\0\0\0\0"
	"\0\0\0\0"
	"\0\0\0\0"
	"\0\0\0\0"
	//main process heap
	"\0\0\0\0"
	//current heap location
	"\0\0\0\0"
	//current spot in current heap
	"\0\0\0\0"
	//current heap size
	"\0\0\0\0"
	//new heap location
	"\0\0\0\0"
	//last try statement address
	"\0\0\0\0", 40);
Thunk TExitProcess ("ExitProcess", 0x9B);
Thunk TGetStdHandle ("GetStdHandle", 0x16A);
Thunk TWriteFile ("WriteFile", 0x2F7);
Thunk TGetProcessHeap ("GetProcessHeap", 0x156);
Thunk THeapAlloc ("HeapAlloc", 0x1BD);
Thunk THeapReAlloc ("HeapReAlloc", 0x1C4);
*/

template class Deleter<Separator2>;
template class Deleter<DirectiveTitle>;
template class Deleter<Identifier>;
template class Deleter<Array<string>>;
template class Deleter<LexToken>;

char allPurposeStringBuffer [ALL_PURPOSE_STRING_BUFFER_SIZE];

int Math::min(int a, int b) {
	return a < b ? a : b;
}
int Math::max(int a, int b) {
	return a > b ? a : b;
}
#ifdef DEBUG
	#ifdef TRACK_OBJ_IDS
		ObjCounter* ObjCounter::headObjCounter = nullptr;
		ObjCounter* ObjCounter::tailObjCounter = nullptr;
	#endif
	int ObjCounter::objCount = 0;
	int ObjCounter::untrackedObjCount;
	int ObjCounter::nextObjID = 0;
	ObjCounter::ObjCounter(onlyWhenTrackingIDs(char* pObjType))
	#ifdef TRACK_OBJ_IDS
		: objType(pObjType)
		, objID(nextObjID)
		, prev(tailObjCounter)
		, next(nullptr)
	#endif
	{
		objCount++;
		nextObjID++;

		#ifdef TRACK_OBJ_IDS
			printf("  Added %s\t%d,\tobj count: %d\n", objType, objID, objCount);
			if (tailObjCounter != nullptr)
				tailObjCounter->next = this;
			tailObjCounter = this;
			if (headObjCounter == nullptr)
				headObjCounter = this;
		#endif
	}
	ObjCounter::~ObjCounter() {
		objCount--;
		#ifdef TRACK_OBJ_IDS
			printf("Deleted %s\t%d,\tobj count: %d\n", objType, objID, objCount);
			if (next != nullptr)
				next->prev = prev;
			else if (this == tailObjCounter)
				tailObjCounter = prev;
			if (prev != nullptr)
				prev->next = next;
			else if (this == headObjCounter)
				headObjCounter = next;
		#endif
	}
	//assign the initial object ID to exclude any statically-allocated ObjCounters
	void ObjCounter::start() {
		untrackedObjCount = nextObjID;
		#ifdef TRACK_OBJ_IDS
			//reset the list, it's ok to leave any objects linked
			headObjCounter = nullptr;
			tailObjCounter = nullptr;
		#endif
	}
	//check for any non-deallocated objects
	void ObjCounter::end() {
		#ifdef TRACK_OBJ_IDS
			for (; headObjCounter != nullptr; headObjCounter = headObjCounter->next)
				printf("      Remaining object: %s\t%d\n", headObjCounter->objType, headObjCounter->objID);
		#endif
		printf("Total remaining objects: %d\n", objCount - untrackedObjCount);
		printf("Total objects used: %d + %d untracked\n", (nextObjID - untrackedObjCount), untrackedObjCount);
	}
#endif
template <class Type> Deleter<Type>::Deleter(Type* pToDelete)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("DELETER")))
toDelete(pToDelete) {
}
template <class Type> Deleter<Type>::~Deleter() {
	delete toDelete;
}
//return the held object without releasing it
template <class Type> Type* Deleter<Type>::retrieve() {
	return toDelete;
}
//return the held object and release it so it won't be deleted
template <class Type> Type* Deleter<Type>::release() {
	Type* val = toDelete;
	toDelete = nullptr;
	return val;
}
char* Error::snippet = []() -> char* {
	char* val = new char[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2 + 2];
	memset(val, ' ', SNIPPET_PREFIX_SPACES);
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS] = '\n';
	memset(val + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1, ' ', SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2);
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2] = '^';
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2 + 1] = '\0';
	return val;
}();
int Error::lastErrorPos = -1;
int Error::errorCount = 0;
//print an error and throw
void Error::makeError(ErrorType type, char* message, SourceFile* sourceFile, Token* token) {
	if (token->contentPos != lastErrorPos) {
		lastErrorPos = token->contentPos;
		int* rowStarts = sourceFile->rowStarts->inner;
		//find the right row
		int rowLo = 0;
		int rowHi = sourceFile->rowStarts->length;
		int row = rowHi / 2;
		while (row > rowLo) {
			if (lastErrorPos >= rowStarts[row])
				rowLo = row;
			else
				rowHi = row;
			row = (rowLo + rowHi) / 2;
		}
		//print the error
		printf("Error in \"%s\" at line %d column %d: ",
			sourceFile->filename.c_str(), row + 1, lastErrorPos - rowStarts[row] + 1);
		switch (type) {
			case General: puts(message); break;
			case EndOfFileWhileSearching: printf("reached the end of the file while searching for %s\n", message); break;
			case EndOfFileWhileReading: printf("reached the end of the file while reading %s\n", message); break;
		}
		showSnippet(sourceFile, token);
		errorCount++;
	}
	throw 0;
}
//show the snippet where the error/warning is
void Error::showSnippet(SourceFile* sourceFile, Token* token) {
	int targetStart = token->contentPos - SNIPPET_CHARS / 2;
	int targetEnd = targetStart + SNIPPET_CHARS;
	int start = Math::max(targetStart, 0);
	int end = Math::min(targetEnd, sourceFile->contentsLength);
	memset(snippet + SNIPPET_PREFIX_SPACES, ' ', start - targetStart);
	memset(snippet + SNIPPET_PREFIX_SPACES + end - targetStart, ' ', targetEnd - end);
	for (int i = start; i < end; i++) {
		char c = sourceFile->contents[i];
		//make sure c is in the printable character range
		snippet[i - targetStart + SNIPPET_PREFIX_SPACES] = (c >= '!' && c <= '~') ? c : ' ';
	}
	puts(snippet);
}
/*
//make a warning
void makeWarning(int type, char* message, size_t loc) {
	printf("Warning at line %d column %d: ", rows[loc], cols[loc]);
	if (type == 0)
		printf("%s\n", message);
	warnings = true;
	showSnippet(loc);
}
//build all of the Main. functions
void buildMainFunctions(Array<MainFunction*>* mainfunctions) {
	//Main.exit(int);
	Function* f = new Function(TVOID);
	f->params.add(new VariableData("", TINT, new MEMPTR(ESP, 0, -1, 4, false), 0));
	AssemblySequence* as = new AssemblySequence(new PUSH(TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new CALL(&TExitProcess));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.exit", 0));
	//Main.print(String);
	f = new Function(TVOID);
	f->params.add(new VariableData("", TSTRING, new MEMPTR(ESP, 0, -1, 4, false), 0));
	as = new AssemblySequence(new MOV(TREG32, EAX, TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new PUSH(TCONSTANT, 0))
		->add(new PUSH(TDATAADDRESS, VDATAGENERAL))
		//string size
		->add(new PUSH(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_LENGTH, true))))
		//string address
		->add(new PUSH(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_ADDR, true))))
		//push STD_OUTPUT_HANDLE
		->add(new PUSH(TCONSTANT, -0x0B))
		->add(new CALL(&TGetStdHandle))
		->add(new PUSH(TREG32, EAX))
		->add(new CALL(&TWriteFile))
		->add(new RET(4));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.print", 0));
	//Main.str(int)
	f = new Function(TSTRING);
	f->params.add(new VariableData("", TINT, new MEMPTR(ESP, 0, -1, 4, false), 0));
	as = new AssemblySequence(new MOV(TREG32, EAX, TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new MOV(TREG32, ESI, TREG32, ESP))
		//load 10 in the divisor
		->add(new MOV(TREG32, ECX, TCONSTANT, 10))
		//check if the number is negative
		->add(new CMP(TREG32, EAX, TCONSTANT, 0))
		->add(new JGE(TINSTRUCTIONRELATIVE, 7))
		//indicate negative
		->add(new MOV(TREG8, BL, TCONSTANT, 1))
		//get the lowest digit, negate everything, then jump to regular processing
		->add(new CDQ())
		->add(new IDIV(TREG32, ECX))
		->add(new NEG(TREG32, EAX))
		->add(new NEG(TREG8, DL))
		->add(new JMP(TINSTRUCTIONRELATIVE, 4))
		//indicate positive
		->add(new MOV(TREG8, BL, TCONSTANT, 0))
		//begin converting
		->add(new CDQ())
		->add(new IDIV(TREG32, ECX))
		->add(new ADD(TREG8, DL, TCONSTANT, 48))
		->add(new DEC(TREG32, ESI))
		->add(new MOV(TBYTEPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, 0, true)), TREG8, DL))
		//jump back if it's greater than 0
		->add(new CMP(TREG32, EAX, TCONSTANT, 0))
		->add(new JG(TINSTRUCTIONRELATIVE, -6))
		//add a minus sign if it's negative
		->add(new CMP(TREG8, BL, TCONSTANT, 0))
		->add(new JE(TINSTRUCTIONRELATIVE, 3))
		->add(new DEC(TREG32, ESI))
		->add(new MOV(TBYTEPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, 0, true)), TCONSTANT, 45))
		//begin moving the string
		//get the heap address
		->add(new MOV(TREG32, EAX, TDATADWORDPTR, VDATACURRENTHEAPSPOT))
		//load the byte array address onto the heap
		->add(new LEA(TREG32, EDI, TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, 8, true))))
		->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_ADDR, true)), TREG32, EDI))
		//load the string size
		->add(new MOV(TREG32, ECX, TREG32, ESP))
		->add(new SUB(TREG32, ECX, TREG32, ESI))
		->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_LENGTH, true)), TREG32, ECX))
		//bump up the heap
		->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TCONSTANT, 8))
		->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TREG32, ECX))
		//move the string
		->add(new REPMOVSB())
		->add(new RET(4));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.str", 0));
}
*/
