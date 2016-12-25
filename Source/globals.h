#include "string"
using namespace std;

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

#define SNIPPETCHARS 33
//check if pos is within the code
#define inbounds() (pos < clength)
#define outofbounds() (pos >= clength)

class Thunk;
template <class type> class Array;
class Expression;
class Function;
class MainFunction;

extern size_t pos;
extern char* contents;
extern size_t clength;
extern Array<Expression*> tokens;
extern int tlength;
extern Array<Function*> functions;
extern Array<MainFunction*> mainfunctions;
extern int errors;
extern char snippet[];
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

void makeError(int type, char* message, size_t loc);
void showSnippet(size_t loc);
void makeWarning(int type, char* message, size_t loc);
void buildMainFunctions(Array<MainFunction*>* mainfunctions);
