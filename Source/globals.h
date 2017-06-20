#ifndef GLOBALS_H
#define GLOBALS_H
//#include "string"
//using namespace std;

class SourceFile;
class Token;
template <class Type> class Array;

#ifdef DEBUG
#define onlyInDebug(x) x
#define onlyInDebugWithComma(x) x,
#else
#define onlyInDebug(x)
#define onlyInDebugWithComma(x)
#endif

const int ALL_PURPOSE_STRING_BUFFER_SIZE = 0x100;

/*
#define IMAGEBASE 0x400000
#define VDATAGENERAL 0
#define VDATAGENERAL2 4
#define VDATAGENERAL3 8
#define VDATAGENERAL4 12
#define VDATAMAINHEAP 16
#define VDATACURRENTHEAP 20
#define VDATACURRENTHEAPSPOT 24
#define VDATACURRENTHEAPSIZE 28
#define VDATANEWHEAP 32
#define VDATALASTTRYADDRESS 36
#define STRING_LENGTH 0
#define STRING_ADDR 4
#define FUNCTION_ADDR 0

class Thunk;
template <class Type> class Array;
class Expression;
class Function;
class MainFunction;

extern Array<Expression*> tokens;
extern int tlength;
extern Array<Function*> functions;
extern Array<MainFunction*> mainfunctions;
*/
extern char allPurposeStringBuffer[];
/*
extern bool warnings;
extern int* rows;
extern int* cols;

extern int codesize;
extern int rdataRVA;
extern int rdatasize;
extern int dataRVA;
extern int datasize;
extern string data;
extern Thunk TExitProcess;
extern Thunk TGetStdHandle;
extern Thunk TWriteFile;
extern Thunk TGetProcessHeap;
extern Thunk THeapAlloc;
extern Thunk THeapReAlloc;
*/

enum ErrorType: unsigned char {
	General,
	EndOfFileWhileSearching,
	EndOfFileWhileReading
};
#ifdef DEBUG
class ObjCounter {
public:
	ObjCounter(char* pObjType);
	~ObjCounter();
private:
	char* objType;
	int objID;

	static const int OBJ_IDS_COUNT = 4096;
	static int objCount;
	static int nextObjID;
	static int untrackedObjCount;
	static bool objIDs[OBJ_IDS_COUNT];
public:
	static void start();
	static void end();
};
#endif
class Memory {
public:
	template <class Type> static void deleteArrayAndContents(Array<Type>* a);
};
class Error {
public:
	static void makeError(ErrorType type, char* message, SourceFile* sourceFile, Token* token);
private:
	static const int SNIPPET_CHARS = 0x41;
	static char* buildSnippet();
	static char* snippet;
	static int errors;
	static int lastErrorPos;

	static void showSnippet(SourceFile* sourceFile, Token* token);
};
/*
void makeWarning(int type, char* message, size_t loc);
void buildMainFunctions(Array<MainFunction*>* mainfunctions);
*/
#endif
