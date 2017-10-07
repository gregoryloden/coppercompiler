#include "Project.h"

Statement::Statement(onlyWhenTrackingIDs(char* pObjType))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
Statement::~Statement() {}
ExpressionStatement::ExpressionStatement(Token* pExpression)
: Statement(onlyWhenTrackingIDs("EXPSTMT"))
, expression(pExpression) {
}
ExpressionStatement::~ExpressionStatement() {
	delete expression;
}
StatementList::StatementList(Array<Statement*>* pStatements, Array<CVariableDefinition*>* pVariables)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("STMTLST")) COMMA)
statements(pStatements)
, variables(pVariables) {
}
StatementList::~StatementList() {
	statements->deleteContents();
	delete statements;
	variables->deleteContents();
	delete variables;
}
