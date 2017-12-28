#include "../General/globals.h"

class Token;
class CVariableDefinition;
class StatementList;
class IntConstant;
template <class Type> class Array;

class Statement onlyInDebug(: public ObjCounter) {
protected:
	Statement(onlyWhenTrackingIDs(char* pObjType));
public:
	virtual ~Statement();
};
class EmptyStatement: public Statement {
public:
	EmptyStatement();
	virtual ~EmptyStatement();
};
class ExpressionStatement: public Statement {
public:
	Token* expression;

	ExpressionStatement(Token* pExpression);
	virtual ~ExpressionStatement();
};
class ReturnStatement: public Statement {
public:
	Token* expression;

	ReturnStatement(Token* pExpression);
	virtual ~ReturnStatement();
};
class IfStatement: public Statement {
public:
	Token* condition;
	Array<Statement*>* thenBody;
	Array<Statement*>* elseBody;

	IfStatement(Token* pCondition, Array<Statement*>* pThenBody, Array<Statement*>* pElseBody);
	virtual ~IfStatement();
};
class LoopStatement: public Statement {
public:
	ExpressionStatement* initialization;
	Token* condition;
	Token* increment;
	Array<Statement*>* body;
	bool initialConditionCheck;

	LoopStatement(
		ExpressionStatement* pInitialization,
		Token* pCondition,
		Token* pIncrement,
		Array<Statement*>* pBody,
		bool pInitialConditionCheck);
	virtual ~LoopStatement();
};
class LoopControlFlowStatement: public Statement {
public:
	bool continueLoop;
	IntConstant* levels;

	LoopControlFlowStatement(bool pContinueLoop, IntConstant* pLevels);
	virtual ~LoopControlFlowStatement();
};
