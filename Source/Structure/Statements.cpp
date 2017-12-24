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
ReturnStatement::ReturnStatement(Token* pExpression)
: Statement(onlyWhenTrackingIDs("RTNSTMT"))
, expression(pExpression) {
}
ReturnStatement::~ReturnStatement() {
	delete expression;
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
