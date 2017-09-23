#ifndef GLOBALS_H
#define GLOBALS_H

class Token;
class AbstractCodeBlock;

#define forEach(Type, t, a, ti) ArrayIterator<Type> ti (a); for (Type t = ti.getFirst(); ti.hasThis(); t = ti.getNext())
#define forEachContinued(Type, t, ti) for (Type t = ti->getNext(); ti->hasThis(); t = ti->getNext())
#define COMMA ,
#ifdef DEBUG
	#define onlyInDebug(x) x
	#define assert(x) if (!(x)) { printf("Error: Assertion failure\n"); Debug::crashProgram(); }
	//**/#define TRACK_OBJ_IDS
#else
	#define onlyInDebug(x)
	#define assert(x)
#endif
#ifdef TRACK_OBJ_IDS
	#define onlyWhenTrackingIDs(x) x
#else
	#define onlyWhenTrackingIDs(x)
#endif

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
		virtual ~ObjCounter();

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
class Keyword {
public:
	static const char* rawKeyword;
};
//used to delete objects during a throw
//should always be stack allocated
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
enum class ErrorType: unsigned char {
	General,
	EndOfFileWhileSearching,
	EndOfFileWhileReading,
	Continuation
};
class Error {
public:
	static const int SNIPPET_PREFIX_SPACES = 4;
	static const int SNIPPET_CHARS = 0x41;
private:
	static char* snippet;
public:
	static int errorCount;

	static void makeError(ErrorType type, const char* message, Token* token);
private:
	static void showSnippet(Token* token);
};
#ifdef DEBUG
	class Debug {
	public:
		static void printAbstractCodeBlock(AbstractCodeBlock* codeBlock, int spacesCount);
		static void crashProgram();
	};
#endif
/*
void makeWarning(int type, char* message, size_t loc);
void buildMainFunctions(Array<MainFunction*>* mainfunctions);
*/
#endif
