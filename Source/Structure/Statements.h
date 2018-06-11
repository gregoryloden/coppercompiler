#include "../Structure/Tokens.h"

class Token;
class CVariableDefinition;
class StatementList;
class IntConstant;
template <class KeyElement, class Value> class PrefixTrie;

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
	Token* returnKeywordToken;

	ReturnStatement(Token* pExpression, Token* pReturnKeywordToken);
	virtual ~ReturnStatement();
};
class IfStatement: public Statement {
public:
	class ConditionVisitor: public TokenVisitor {
	private:
		PrefixTrie<char, CVariableDefinition*>* variables;
		TokenVisitor* secondaryVisitor;
	public:
		OperatorType conditionBooleanType; //copper: readonly

		ConditionVisitor(PrefixTrie<char, CVariableDefinition*>* pVariables, TokenVisitor* pSecondaryVisitor);
		virtual ~ConditionVisitor();

		void handleExpression(Token* t);
	};

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
