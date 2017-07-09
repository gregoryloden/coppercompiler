#ifndef GLOBALS_H
#define GLOBALS_H

class SourceFile;
class Token;
template <class Type> class Array;

#define forEach(Type, t, a, ti) ArrayIterator<Type> ti (a); for (Type t = ti.getFirst(); ti.hasThis(); t = ti.getNext())
#ifdef DEBUG
	#define onlyInDebug(x) x
	#define onlyInDebugWithComma(x) x,
	//*/#define TRACK_OBJ_IDS
#else
	#define onlyInDebug(x)
	#define onlyInDebugWithComma(x)
#endif
#ifdef TRACK_OBJ_IDS
	#define onlyWhenTrackingIDs(x) x
	#define onlyWhenTrackingIDsWithComma(x) x,
#else
	#define onlyWhenTrackingIDs(x)
	#define onlyWhenTrackingIDsWithComma(x)
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

class Math {
public:
	static int min(int a, int b);
	static int max(int a, int b);
};
#ifdef DEBUG
	class ObjCounter {
	public:
		ObjCounter(onlyWhenTrackingIDs(char* pObjType));
		~ObjCounter();

	private:
		#ifdef TRACK_OBJ_IDS
			char* objType;
			int objID;
			ObjCounter* prev;
			ObjCounter* next;
			static ObjCounter* headObjCounter;
			static ObjCounter* tailObjCounter;
		#endif
		static int objCount;
		static int untrackedObjCount;
		static int nextObjID;

	public:
		static void start();
		static void end();
	};
#endif
template <class Type> class Deleter onlyInDebug(: public ObjCounter) {
public:
	Deleter(Type* pToDelete);
	virtual ~Deleter();

private:
	Type* toDelete;

public:
	Type* release();
	Type* retrieve();
};
enum ErrorType: unsigned char {
	General,
	EndOfFileWhileSearching,
	EndOfFileWhileReading
};
class Error {
public:
	static const int SNIPPET_PREFIX_SPACES = 4;
	static const int SNIPPET_CHARS = 0x41;
private:
	static char* snippet;
	static int lastErrorPos;
public:
	static int errorCount;

	static void makeError(ErrorType type, char* message, SourceFile* sourceFile, Token* token);
private:
	static void showSnippet(SourceFile* sourceFile, Token* token);
};
/*
void makeWarning(int type, char* message, size_t loc);
void buildMainFunctions(Array<MainFunction*>* mainfunctions);
*/
#endif
