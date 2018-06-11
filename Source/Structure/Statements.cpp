#include "Project.h"

Statement::Statement(onlyWhenTrackingIDs(char* pObjType))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
Statement::~Statement() {}
EmptyStatement::EmptyStatement()
: Statement(onlyWhenTrackingIDs("EMPSTMT")) {
}
EmptyStatement::~EmptyStatement() {}
ExpressionStatement::ExpressionStatement(Token* pExpression)
: Statement(onlyWhenTrackingIDs("EXPSTMT"))
, expression(pExpression) {
}
ExpressionStatement::~ExpressionStatement() {
	delete expression;
}
ReturnStatement::ReturnStatement(Token* pExpression, Token* pReturnKeywordToken)
: Statement(onlyWhenTrackingIDs("RTNSTMT"))
, expression(pExpression)
, returnKeywordToken(pReturnKeywordToken) {
}
ReturnStatement::~ReturnStatement() {
	delete expression;
	delete returnKeywordToken;
}
IfStatement::IfStatement(Token* pCondition, Array<Statement*>* pThenBody, Array<Statement*>* pElseBody)
: Statement(onlyWhenTrackingIDs("IFSTMT"))
, condition(pCondition)
, thenBody(pThenBody)
, elseBody(pElseBody) {
}
IfStatement::~IfStatement() {
	delete condition;
	if (thenBody != nullptr) {
		thenBody->deleteContents();
		delete thenBody;
	}
	if (elseBody != nullptr) {
		elseBody->deleteContents();
		delete elseBody;
	}
}
IfStatement::ConditionVisitor::ConditionVisitor(
	PrefixTrie<char, CVariableDefinition*>* pVariables, TokenVisitor* pSecondaryVisitor)
: TokenVisitor(onlyWhenTrackingIDs("IFCNVTR"))
, variables(pVariables)
, secondaryVisitor(pSecondaryVisitor)
, conditionBooleanType(OperatorType::None) {
}
IfStatement::ConditionVisitor::~ConditionVisitor() {
	delete secondaryVisitor;
	//don't delete the variables, they're owned by something else
}
//search through all tokens that will definitely happen for either the then-body or the else-body
void IfStatement::ConditionVisitor::handleExpression(Token* t) {
	Operator* o;
	ParenthesizedExpression* p;
	if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
		if (conditionBooleanType == OperatorType::None) {
			if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
				conditionBooleanType = o->operatorType;
				t->visitSubtokens(this);
			}
			return;
		} else if (conditionBooleanType == o->operatorType) {
			t->visitSubtokens(this);
			return;
		}
	} else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr) {
		handleExpression(p->expression);
		return;
	} else if (conditionBooleanType == OperatorType::None)
		return;
	//we found a regular token within a boolean AND or OR, pass it on to the other visitor
	secondaryVisitor->handleExpression(t);
}
LoopStatement::LoopStatement(
	ExpressionStatement* pInitialization,
	Token* pCondition,
	Token* pIncrement,
	Array<Statement*>* pBody,
	bool pInitialConditionCheck)
: Statement(onlyWhenTrackingIDs("LOPSTMT"))
, initialization(pInitialization)
, condition(pCondition)
, increment(pIncrement)
, body(pBody)
, initialConditionCheck(pInitialConditionCheck) {
}
LoopStatement::~LoopStatement() {
	delete initialization;
	delete condition;
	delete increment;
	body->deleteContents();
	delete body;
}
LoopControlFlowStatement::LoopControlFlowStatement(bool pContinueLoop, IntConstant* pLevels)
: Statement(onlyWhenTrackingIDs("CFLSTMT"))
, continueLoop(pContinueLoop)
, levels(pLevels) {
}
LoopControlFlowStatement::~LoopControlFlowStatement() {
	delete levels;
}
