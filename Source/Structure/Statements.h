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
	ExpressionStatement(Token* pExpression);
	virtual ~ExpressionStatement();

	Token* expression;
};
class ReturnStatement: public Statement {
public:
	ReturnStatement(Token* pExpression);
	virtual ~ReturnStatement();

	Token* expression;
};
class IfStatement: public Statement {
public:
	IfStatement(Token* pCondition, Array<Statement*>* pThenBody, Array<Statement*>* pElseBody);
	virtual ~IfStatement();

	Token* condition;
	Array<Statement*>* thenBody;
	Array<Statement*>* elseBody;
};
class LoopStatement: public Statement {
public:
	LoopStatement(
		ExpressionStatement* pInitialization,
		Token* pCondition,
		Token* pIncrement,
		Array<Statement*>* pBody,
		bool pInitialConditionCheck);
	virtual ~LoopStatement();

	ExpressionStatement* initialization;
	Token* condition;
	Token* increment;
	Array<Statement*>* body;
	bool initialConditionCheck;
};
class LoopControlFlowStatement: public Statement {
public:
	LoopControlFlowStatement(bool pContinueLoop, IntConstant* pLevels);
	virtual ~LoopControlFlowStatement();

	bool continueLoop;
	IntConstant* levels;
};
