#include "../General/globals.h"
#include "string"
using namespace std;

/*
#define EEXPRESSION 0
#define EOPERATION 1
#define EBOOLEANOPER 2
#define EVAR 3
#define EINTCONSTANT 4
#define EFLOATCONSTANT 5
#define EOBJECTCONSTANT 6
#define ESTRINGCONCAT 7
#define ECAST 8
#define EFUNCTIONCALL 9
#define EASSEMBLYSEQUENCE 10
#define EUNKNOWNVALUE 11
#define ESEPARATOR 12
#define ERETURN 13
#define EIFBRANCH 14
#define ECONTROLFLOW 15
#define EMULTILOOP 16
#define TNONE -1
#define TVOID 0
#define TBOOLEAN 1
#define TBYTE 2
#define TSHORT 3
#define TINT 4
#define TFLOAT 5
#define TDOUBLE 6
#define TSTRING 7
#define TFUNCTION 8
#define OINC 1
#define ODEC 2
#define OVARLNOT 3
#define OVARBNOT 4
#define OVARNEG 5
#define OLNOT 6
#define OBNOT 7
#define ONEG 8
#define OMUL 9
#define ODIV 10
#define OMOD 11
#define OADD 12
#define OSUB 13
#define OSHL 14
#define OSHR 15
#define OSAR 16
#define OROL 17
#define OROR 18
#define OAND 19
#define OXOR 20
#define OOR 21
#define OEQ 22
#define ONEQ 23
#define OLEQ 24
#define OGEQ 25
#define OLT 26
#define OGT 27
#define OBAND 28
#define OBXOR 29
#define OBOR 30
#define OQMARK 31
#define OCOLON 32
#define OASSIGN 33
#define OASSIGNADD 34
#define OASSIGNSUB 35
#define OASSIGNMUL 36
#define OASSIGNDIV 37
#define OASSIGNMOD 38
#define OASSIGNSHL 39
#define OASSIGNSHR 40
#define OASSIGNSAR 41
#define OASSIGNROL 42
#define OASSIGNROR 43
#define OASSIGNAND 44
#define OASSIGNXOR 45
#define OASSIGNOR 46
#define OASSIGNBAND 47
#define OASSIGNBXOR 48
#define OASSIGNBOR 49
#define PASSIGN 1
#define PQMARK 2
#define PBOR 3
#define PBXOR 4
#define PBAND 5
#define PCOMPARE 6
#define POR 7
#define PXOR 8
#define PAND 9
#define PSHIFT 10
#define PADDSUB 11
#define PMULDIV 12
#define PUNARYPRE 13
#define PUNARYPOST 14
#define SLPAREN 1
#define SRPAREN 2
#define SCOMMA 3
#define SSEMICOLON 4
#define CBREAK 1
#define CCONTINUE 2
#define CCASE 3
#define CDEFAULT 4

class MEMPTR;
class AssemblyInstruction;
class Expression;
*/
class CDataType;
template <class Type> class Array;
class AbstractCodeBlock;
class StatementList;
class Token;
class VariableInitialization;
template <class Key, class Value> class AVLTree;
/*

int divhighreg(int rtype);
bool isNumerical(int context, bool includeboolean);
int getrtype(int context);
int getrtypestack(int context);
int getpopcount(int context);
int getbytesize(int context);
bool isSeparatorType(Expression* e, int type);
bool isBooleanOperationType(Expression* e, int type);
bool isCompareOperation(Expression* e);
bool isLogicalNotOperation(Expression* e);
bool isSpecificUnknownValue(Expression* e, char* c);
AssemblyInstruction* comparisonInstruction(int oper, int type, int val, bool reverse, bool jump);
Array<AssemblyInstruction*>* formConditional(Expression* condition,
	Array<AssemblyInstruction*>* aif, Array<AssemblyInstruction*>* aelse, bool deletable);
Array<AssemblyInstruction*>* getCondition(Expression* condition, bool swap, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse);
void setJumps(Array<AssemblyInstruction*>* jumpslist, int truejmpdest, int falsejmpdest);
Array<AssemblyInstruction*>* getAllAssembly(Array<AssemblyInstruction*>* to, Array<Expression*>* from);

class Expression
: public ObjCounter
 {
public:
	Expression(int theetype, bool thevalue, int thecontext, size_t thecontentpos);
	virtual ~Expression();

	virtual Array<AssemblyInstruction*>* getAssembly(bool push, int reg) = 0;

	int etype;
	size_t contentpos;
	bool value;
	int context;
	bool toplevel;
};
class VariableData
: public ObjCounter
 {
public:
	VariableData(string thename, int thecontext, MEMPTR* theptr, int thetpos);
	VariableData(string thename, int thecontext, int datapos, int thetpos);
	VariableData(VariableData* v);
	virtual ~VariableData();

	string name;
	int ptype;
	int ptypestack;
	intptr_t ptr;
	int context;
	int tpos;
	bool fixedfunction;
};
class Function
: public ObjCounter
 {
public:
	Function(int thecontext);
	~Function();

	Array<AssemblyInstruction*>* getInstructions();

	Array<Expression*> statements;
	Array<VariableData*> variables;
	Array<VariableData*> params;
	size_t starttpos;
	int startinstruction;
	int context;
	int vnum;
};
class VariableStack
: public ObjCounter
 {
public:
	VariableStack(VariableData* v, Function* f, VariableStack* n);
	~VariableStack();

	VariableData* val;
	VariableStack* next;
	int num;
};
class BlockStack
: public ObjCounter
 {
public:
	BlockStack(Expression* e, BlockStack* n);
	~BlockStack();

	Expression* val;
	BlockStack* next;
};
class Thunk
: public ObjCounter
 {
public:
	Thunk(const char* thename, int thethunk);
	~Thunk();

	bool used;
	string thunk;
	string rva;
	int offset;
};
class Operation: public Expression {
public:
	Operation(string s, size_t thecontentpos, int thetpos);
	~Operation();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);
	Array<AssemblyInstruction*>* getAssemblyTernary(bool push, int reg);
	Array<AssemblyInstruction*>* getAssemblyBinary(bool push, int reg);
	Array<AssemblyInstruction*>* getAssemblyUnary(bool push, int reg);

	int oper;
	int prec;
	Expression* left;
	Expression* right;
	Expression* testpart;
	int tpos;
};
class BooleanOperation: public Expression {
public:
	BooleanOperation(int theoper, size_t thecontentpos);
	~BooleanOperation();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);
	Array<AssemblyInstruction*>* buildConditional(bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse);
	void addConditionalBody(Array<AssemblyInstruction*>* a, bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse);
	void handleBodyExpression(Expression* e, bool swap, Array<AssemblyInstruction*>* a, bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse);

	int oper;
	Array<Expression*> inner;
	AssemblyInstruction** jumps;
};
class Variable: public Expression {
public:
	Variable(VariableData* v, size_t thecontentpos);
	~Variable();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	VariableData* inner;
};
class IntConstant: public Expression {
public:
	IntConstant(int i, size_t thecontentpos);
	IntConstant(bool b, size_t thecontentpos);
	~IntConstant();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	int ival;
};
class FloatConstant: public Expression {
public:
	FloatConstant(int e, BigInt* b, size_t thecontentpos);
	FloatConstant(size_t thecontentpos);
	~FloatConstant();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	BigInt bits;
	int exp;
	bool doubleprec;
	bool sign;
};
class ObjectConstant: public Expression {
public:
	ObjectConstant(string s, size_t thecontentpos);
	ObjectConstant(Function* f, string s, size_t thecontentpos);
	virtual ~ObjectConstant();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	string sval;
	Function* fval;
};
class StringConcatenation: public Expression {
public:
	StringConcatenation(size_t thecontentpos);
	~StringConcatenation();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Array<Expression*> strings;
};
class Cast: public Expression {
public:
	Cast(int to, Expression* from);
	Cast(int to, size_t thecontentpos);
	~Cast();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);
	bool validCast();

	Expression* inner;
};
class FunctionCall: public Expression {
public:
	FunctionCall(Expression* e);
	~FunctionCall();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Expression* einner;
	Function* finner;
	Array<Expression*> params;
};
class AssemblySequence: public Expression {
public:
	AssemblySequence();
	AssemblySequence(AssemblyInstruction* a);
	~AssemblySequence();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Array<AssemblyInstruction*> inner;
};
class UnknownValue: public Expression {
public:
	UnknownValue(string s, size_t thecontentpos);
	~UnknownValue();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	string val;
};
class Separator: public Expression {
public:
	Separator(char c, size_t thecontentpos);
	~Separator();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	int val;
};
class Return: public Expression {
public:
	Return(Expression* e, Function* f);
	~Return();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Expression* einner;
	Function* finner;
};
class IfBranch: public Expression {
public:
	IfBranch(size_t thecontentpos);
	~IfBranch();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Expression* condition;
	Array<Expression*> ifpart;
	Array<Expression*> elsepart;
};
class ControlFlow: public Expression {
public:
	ControlFlow(string s, size_t thecontentpos);
	~ControlFlow();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	int val;
	AssemblyInstruction* jump;
	Expression* code;
};
class MultiLoop: public Expression {
public:
	MultiLoop(size_t thecontentpos, bool thedowhile);
	~MultiLoop();

	Array<AssemblyInstruction*>* getAssembly(bool push, int reg);

	Expression* initialization;
	Expression* condition;
	Expression* increment;
	Array<Expression*> statements;
	Array<ControlFlow*> controlflows;
	bool dowhile;
};
class FixedFunction: public VariableData {
public:
	FixedFunction(VariableData* it);
	~FixedFunction();

	Function* inner;
};
class MainFunction: public ObjectConstant {
public:
	MainFunction(Function* f, string s, size_t thecontentpos);
	~MainFunction();

	bool added;
};
*/
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
class SourceFile onlyInDebug(: public ObjCounter) {
public:
	SourceFile(string pFilename);
	virtual ~SourceFile();

	string filename; //copper: private<readonly Include>
	char* contents; //copper: private<readonly Lex>
	int contentsLength; //copper: private<readonly(Lex, ParseDirectives)>
	Array<int>* rowStarts; //copper: private<readonly Lex>
	AbstractCodeBlock* abstractContents; //copper: private<writeonly ParseDirectives>
	AVLTree<SourceFile*, bool>* includedFiles; //copper: private<readonly Include>
	Array<SourceFile*>* inclusionListeners; //copper: private<readonly Include>
	Array<Token*>* replacedTokens; //copper: private<readonly Replace>
	Array<VariableInitialization*>* globalVariables; //copper: private<ParseExpressions>
	//Array<??????????> typesDefined;
//	Array<CClass*>* classes;

	int getRow(int contentPos);
};
class CVariableDefinition onlyInDebug(: public ObjCounter) {
public:
	CVariableDefinition(CDataType* pType, Identifier* pName);
	virtual ~CVariableDefinition();

	CDataType* type;
	Identifier* name;
};
