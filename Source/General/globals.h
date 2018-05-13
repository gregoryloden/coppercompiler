#ifndef GLOBALS_H
#define GLOBALS_H
#include "string"
using namespace std;

class Token;
class AbstractCodeBlock;
class Statement;
class VariableInitialization;
class SourceFile;
class ErrorMessage;
template <class Type> class Array;

#define forEach(Type, t, a, ti) ArrayIterator<Type> ti (a); for (Type t = ti.getThis(); ti.hasThis(); t = ti.getNext())
#define forEachContinued(Type, t, ti) for (Type t = (ti)->getThis(); (ti)->hasThis(); t = (ti)->getNext())
#define COMMA ,
#ifdef DEBUG
	#define onlyInDebug(x) x
	#define assert(x) if (!(x)) { printf("Error: Assertion failure\n"); Debug::crashProgram(); }
	/**/#define TRACK_OBJ_IDS
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
	static const short SHORT_MAX = 0x7FFF;

	static int min(int a, int b);
	static int max(int a, int b);
};
class StringUtils {
public:
	static Array<string>* split(string s, char delimiter);
	static bool stringMatchesWildcard(string s, Array<string>* wildcardMatchSubstrings);
};
#ifdef DEBUG
	class ObjCounter {
	private:
		static int objCount;
		static int untrackedObjCount;
		static int nextObjID;
		#ifdef TRACK_OBJ_IDS
			static ObjCounter* headObjCounter;
			static ObjCounter* tailObjCounter;

			char* objType;
			int objID;
			ObjCounter* prev;
			ObjCounter* next;
		#endif

	public:
		ObjCounter(onlyWhenTrackingIDs(char* pObjType));
		#ifdef TRACK_OBJ_IDS
			ObjCounter(ObjCounter* cloneSource): ObjCounter(cloneSource->objType) {}
		#endif
		virtual ~ObjCounter();

		static void start();
		static void end();
	};
#endif
//used to delete objects during a throw
//should always be stack allocated
template <class Type> class Deleter onlyInDebug(: public ObjCounter) {
protected:
	Type* toDelete;

public:
	Deleter(Type* pToDelete);
	virtual ~Deleter();

	Type* release();
	Type* retrieve();
};
template <class Type> class ArrayContentDeleter: public Deleter<Array<Type*>> {
public:
	ArrayContentDeleter(Array<Type*>* pToDelete);
	virtual ~ArrayContentDeleter();
};
enum class ErrorType: unsigned char {
	General,
	EndOfFileWhileSearching,
	EndOfFileWhileReading,
	Expected,
	ExpectedToFollow,
	Continuation,
	CompilerIssue
};
class Error {
public:
	static void makeError(ErrorType type, const char* message, Token* token);
	static void logError(ErrorType type, const char* message, Token* token);
	static void logErrorWithErrorSourceAndOriginFile(
		ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile);
private:
	static ErrorMessage* buildErrorMessage(
		ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile);
};
class ErrorMessage onlyInDebug(: public ObjCounter) {
public:
	static const int SNIPPET_PREFIX_SPACES = 4;
	static const int SNIPPET_CHARS = 0x41;
private:
	static char* snippet;

	ErrorType type;
	const char* message;
	SourceFile* owningFile;
	int contentPos;
	ErrorMessage* continuation;
	SourceFile* errorSourceOwningFile;
	int errorSourceContentPos;
	ErrorMessage* errorSourceContinuation;
	SourceFile* errorOriginFile;

public:
	ErrorMessage(
		ErrorType pType,
		const char* pMessage,
		SourceFile* pOwningFile,
		int pContentPos,
		ErrorMessage* pContinuation,
		SourceFile* pErrorSourceOwningFile,
		int pErrorSourceContentPos,
		ErrorMessage* pErrorSourceContinuation,
		SourceFile* pErrorOriginFile);
	~ErrorMessage();

	int getRow();
	void printError();
private:
	void showSnippet(SourceFile* snippetFile, int snippetContentPos);
};
#ifdef DEBUG
	class Debug {
	public:
		static void printAbstractCodeBlock(AbstractCodeBlock* codeBlock, int tabsCount);
		static void printTokenTree(Token* t, int tabsCount, bool printOperatorParentheses);
		static void printStatementList(Array<Statement*>* a, int tabsCount, bool statementContinuationFollows);
		static void printStatement(Statement* s, int tabsCount);
		static void printLexToken(Token* t);
		static void crashProgram();
	};
#endif
/*
void makeWarning(int type, char* message, size_t loc);
void buildMainFunctions(Array<MainFunction*>* mainfunctions);
*/
#endif
