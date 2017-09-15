#include "Project.h"

Token::Token(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
contentPos(pContentPos)
, endContentPos(pEndContentPos)
, owningFile(pOwningFile) {
}
Token::~Token() {
	//don't delete owningFile
}
int Token::getRow() {
	Array<int>* rowStarts = owningFile->rowStarts;
	int rowLo = 0;
	int rowHi = owningFile->rowStarts->length;
	int row = rowHi / 2;
	while (row > rowLo) {
		if (contentPos >= rowStarts->get(row))
			rowLo = row;
		else
			rowHi = row;
		row = (rowLo + rowHi) / 2;
	}
	return row;
}
Token* Token::getResultingToken(Token* t) {
	SubstitutedToken* s;
	while ((s = dynamic_cast<SubstitutedToken*>(t)) != nullptr)
		t = s->resultingToken;
	return t;
}
EmptyToken::EmptyToken(int pContentPos, SourceFile* pOwningFile)
: Token(onlyWhenTrackingIDs("EMPTTKN" COMMA) pContentPos, pContentPos, pOwningFile) {
}
EmptyToken::~EmptyToken() {}
LexToken::LexToken(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: Token(onlyWhenTrackingIDs(pObjType COMMA) pContentPos, pEndContentPos, pOwningFile) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("IDNTFR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant::IntConstant(int pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
}
IntConstant::~IntConstant() {}
FloatConstant::FloatConstant(
	BigInt* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, significand(pSignificand)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant::~FloatConstant() {
	delete significand;
}
StringLiteral::StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("STRING" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator::Separator(SeparatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("SEPRATR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, type(pType) {
}
Separator::~Separator() {}
string Separator::separatorName(SeparatorType s) {
	switch (s) {
		case Semicolon: return "semicolon";
		case LeftParenthesis: return "left parenthesis";
		case RightParenthesis: return "right parenthesis";
		default: return "comma";
	}
}
Operator::Operator(OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("OPERATR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, type(pType)
, left(nullptr)
, right(nullptr) {
	switch (pType) {
//		case None:
		case Dot:
		case ObjectMemberAccess:
			precedence = PrecedenceObjectMember;
		case Increment:
		case Decrement:
		case VariableLogicalNot:
		case VariableBitwiseNot:
		case VariableNegate:
			precedence = PrecedencePostfix;
		case LogicalNot:
		case BitwiseNot:
		case Negate:
			precedence = PrecedencePrefix;
		case Multiply:
		case Divide:
		case Modulus:
			precedence = PrecedenceMultiplication;
		case Add:
		case Subtract:
			precedence = PrecedenceAddition;
		case ShiftLeft:
		case ShiftRight:
		case ShiftArithmeticRight:
//		case RotateLeft:
//		case RotateRight:
			precedence = PrecedenceBitShift;
		case BitwiseAnd:
			precedence = PrecedenceBitwiseAnd;
		case BitwiseXor:
			precedence = PrecedenceBitwiseXor;
		case BitwiseOr:
			precedence = PrecedenceBitwiseOr;
		case Equal:
		case NotEqual:
		case LessOrEqual:
		case GreaterOrEqual:
		case LessThan:
		case GreaterThan:
			precedence = PrecedenceComparison;
		case BooleanAnd:
			precedence = PrecedenceBooleanAnd;
		case BooleanOr:
			precedence = PrecedenceBooleanOr;
		case Colon:
			precedence = PrecedenceTernaryColon;
		case QuestionMark:
			precedence = PrecedenceTernaryQuestionMark;
		case Assign:
		case AssignAdd:
		case AssignSubtract:
		case AssignMultiply:
		case AssignDivide:
		case AssignModulus:
		case AssignShiftLeft:
		case AssignShiftRight:
		case AssignShiftArithmeticRight:
//		case AssignRotateLeft:
//		case AssignRotateRight:
		case AssignBitwiseAnd:
		case AssignBitwiseXor:
		case AssignBitwiseOr:
//		case AssignBooleanAnd:
//		case AssignBooleanOr:
			precedence = PrecedenceAssignment;
	}
}
Operator::~Operator() {
	//do not delete left or right because they are maintained by the abstract code block
}
DirectiveTitle::DirectiveTitle(string pTitle, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("DCTVTTL" COMMA) pContentPos, pEndContentPos, pOwningFile)
, title(pTitle)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {
	//don't delete the directive, the SourceFile owns it
}
AbstractCodeBlock::AbstractCodeBlock(
	Array<Token*>* pTokens, Array<CDirective*>* pDirectives, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: Token(onlyWhenTrackingIDs("ABCDBLK" COMMA) pContentPos, pEndContentPos, pOwningFile)
, tokens(pTokens)
, directives(pDirectives) {
}
AbstractCodeBlock::~AbstractCodeBlock() {
	tokens->deleteSelfAndContents();
	if (directives != nullptr)
		directives->deleteSelfAndContents();
}
SubstitutedToken::SubstitutedToken(Token* pResultingToken, bool pShouldDelete, Token* tokenBeingReplaced)
: Token(onlyWhenTrackingIDs("SBSTKN" COMMA) tokenBeingReplaced->contentPos, tokenBeingReplaced->endContentPos,
	tokenBeingReplaced->owningFile)
, resultingToken(pResultingToken)
, shouldDelete(pShouldDelete) {
}
SubstitutedToken::~SubstitutedToken() {
	if (shouldDelete)
		delete resultingToken;
}
ParenthesizedExpression::ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* pSource)
: Token(onlyWhenTrackingIDs("PNTHEXP" COMMA) pSource->contentPos, pSource->endContentPos, pSource->owningFile)
, expression(pExpression)
, source(pSource) {
}
ParenthesizedExpression::~ParenthesizedExpression() {
	//don't delete the expression since all the tokens are in the source code block
	delete source;
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDFRLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
