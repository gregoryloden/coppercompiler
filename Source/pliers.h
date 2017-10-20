#include "General/globals.h"
#include "string"
using namespace std;

class SourceFile;
class ErrorMessage;
template <class Type> class Array;

class Pliers {
public:
	Pliers(const char* pBaseFileName, bool pPrintProgress onlyInDebug(COMMA bool printContents));
	~Pliers();

	const char* baseFileName;
	bool printProgress;
	Array<SourceFile*>* allFiles;
	Array<ErrorMessage*>* errorMessages;
};
/*
class Function;
class Expression;
template <class Type> class Array;
class Operation;
class IntConstant;
class VariableData;
class MainFunction;
class VariableStack;
class BigInt;
class BlockStack;

char* getFile(char* filename);
void setRowsAndColumns();
void parseCode();
VariableData* newVariable(Function* owner, bool param, VariableStack* st);
int varType(string s);
Expression* replace(int loc, Expression* val);
void advanceToSemicolon(size_t start, bool track, bool includecomma);
Expression* getInitialization(VariableData* v, Function* owner, VariableStack* st);
Expression* getExpression(Function* owner, bool toplevel, bool semicolon, VariableStack* st, BlockStack* est);
void addStatements(Function* f);
void getStatementBlock(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements);
VariableStack* getSingleStatement(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements);
void keywordErrors(string s, bool nottoplevel, bool notfirst, size_t loc);
void getBranchStatements(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements, size_t start, char* message);
Expression* createExpression(Array<Expression*>* e, int start, int end, int prec);
Expression* implicitCast(Expression* e, int context);
bool ensureSameContexts(Operation* o, bool forcenumerical, bool forceboolean, bool cancastleft);
void setTightLoose();
void buildAssembly();
void buildSections();
void buildrdata();
void buildExecutable();
void cleanup();
template <class Type> void empty(Array<type>* a);
void emptyV(Array<VariableData*>* a);
*/
