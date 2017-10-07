#include "../General/globals.h"

class Token;
class CVariableDefinition;
template <class Type> class Array;

class Statement onlyInDebug(: public ObjCounter) {
protected:
	Statement(onlyWhenTrackingIDs(char* pObjType));
public:
	virtual ~Statement();
};
class ExpressionStatement: public Statement {
public:
	ExpressionStatement(Token* pExpression);
	virtual ~ExpressionStatement();

	Token* expression;
};

//For function bodies, if statements, etc. to include variable definitions
class StatementList onlyInDebug(: public ObjCounter) {
public:
	StatementList(Array<Statement*>* pStatements, Array<CVariableDefinition*>* pVariables);
	~StatementList();

	Array<Statement*>* statements;
	Array<CVariableDefinition*>* variables;
};
