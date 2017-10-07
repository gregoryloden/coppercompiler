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
		case SeparatorType::Semicolon: return "semicolon";
		case SeparatorType::LeftParenthesis: return "left parenthesis";
		case SeparatorType::RightParenthesis: return "right parenthesis";
		default: return "comma";
	}
}
#ifdef TRACK_OBJ_IDS
	Operator::Operator(OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
	: Operator("OPERATR", pType, pContentPos, pEndContentPos, pOwningFile) {
	}
#endif
Operator::Operator(
	onlyWhenTrackingIDs(char* pObjType COMMA) OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs(pObjType COMMA) pContentPos, pEndContentPos, pOwningFile)
, type(pType)
, left(nullptr)
, right(nullptr) {
	switch (pType) {
//		case OperatorType::None:
		case OperatorType::Dot:
		case OperatorType::ObjectMemberAccess:
			precedence = OperatorTypePrecedence::ObjectMember;
			break;
		case OperatorType::Increment:
		case OperatorType::Decrement:
		case OperatorType::VariableLogicalNot:
		case OperatorType::VariableBitwiseNot:
		case OperatorType::VariableNegate:
			precedence = OperatorTypePrecedence::Postfix;
			break;
		case OperatorType::Cast:
		case OperatorType::LogicalNot:
		case OperatorType::BitwiseNot:
		case OperatorType::Negate:
			precedence = OperatorTypePrecedence::Prefix;
			break;
		case OperatorType::Multiply:
		case OperatorType::Divide:
		case OperatorType::Modulus:
			precedence = OperatorTypePrecedence::Multiplication;
			break;
		case OperatorType::Add:
		case OperatorType::Subtract:
			precedence = OperatorTypePrecedence::Addition;
			break;
		case OperatorType::ShiftLeft:
		case OperatorType::ShiftRight:
		case OperatorType::ShiftArithmeticRight:
//		case OperatorType::RotateLeft:
//		case OperatorType::RotateRight:
			precedence = OperatorTypePrecedence::BitShift;
			break;
		case OperatorType::BitwiseAnd:
			precedence = OperatorTypePrecedence::BitwiseAnd;
			break;
		case OperatorType::BitwiseXor:
			precedence = OperatorTypePrecedence::BitwiseXor;
			break;
		case OperatorType::BitwiseOr:
			precedence = OperatorTypePrecedence::BitwiseOr;
			break;
		case OperatorType::Equal:
		case OperatorType::NotEqual:
		case OperatorType::LessOrEqual:
		case OperatorType::GreaterOrEqual:
		case OperatorType::LessThan:
		case OperatorType::GreaterThan:
			precedence = OperatorTypePrecedence::Comparison;
			break;
		case OperatorType::BooleanAnd:
			precedence = OperatorTypePrecedence::BooleanAnd;
			break;
		case OperatorType::BooleanOr:
			precedence = OperatorTypePrecedence::BooleanOr;
			break;
		case OperatorType::Colon:
		case OperatorType::QuestionMark:
			precedence = OperatorTypePrecedence::Ternary;
			break;
		case OperatorType::Assign:
		case OperatorType::AssignAdd:
		case OperatorType::AssignSubtract:
		case OperatorType::AssignMultiply:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
		case OperatorType::AssignShiftLeft:
		case OperatorType::AssignShiftRight:
		case OperatorType::AssignShiftArithmeticRight:
//		case OperatorType::AssignRotateLeft:
//		case OperatorType::AssignRotateRight:
		case OperatorType::AssignBitwiseAnd:
		case OperatorType::AssignBitwiseXor:
		case OperatorType::AssignBitwiseOr:
//		case OperatorType::AssignBooleanAnd:
//		case OperatorType::AssignBooleanOr:
			precedence = OperatorTypePrecedence::Assignment;
			break;
	}
}
Operator::~Operator() {
	delete left;
	delete right;
}
//determine if this operator should steal the right-hand side of the other operator
//may throw
bool Operator::takesRightHandPrecedence(Operator* other) {
	if (precedence != other->precedence)
		return precedence > other->precedence;
	//some operators group on the right, whereas most operators group on the left
	switch (precedence) {
		case OperatorTypePrecedence::Ternary:
			//beginning of a ternary, always group on the right
			if (type == OperatorType::QuestionMark)
				return true;
			//this is a colon and the other one is a question mark- steal the right hand side if there isn't a colon already
			else if (other->type == OperatorType::QuestionMark) {
				Operator* o;
				return (o = dynamic_cast<Operator*>(other->right)) == nullptr || o->type != OperatorType::Colon;
			//this is a colon and the other one is a colon too
			//since we never steal the right hand side of a question mark, we can only get here on an error
			} else
				Error::makeError(ErrorType::General, "ternary expression missing conditional", this);
		case OperatorTypePrecedence::BooleanAnd:
		case OperatorTypePrecedence::BooleanOr:
		case OperatorTypePrecedence::Assignment:
			return true;
	}
	return false;
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
	tokens->deleteContents();
	delete tokens;
	if (directives != nullptr) {
		directives->deleteContents();
		delete directives;
	}
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
//replace the substituted token stack's resulting token with the new one
//always assumes the new one is deletable
void SubstitutedToken::replaceResultingToken(Token* newResultingToken) {
	SubstitutedToken* s;
	if ((s = dynamic_cast<SubstitutedToken*>(resultingToken)) != nullptr)
		s->replaceResultingToken(newResultingToken);
	else {
		if (shouldDelete)
			delete resultingToken;
		else
			shouldDelete = true;
		resultingToken = newResultingToken;
	}
}
ParenthesizedExpression::ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source)
: Token(onlyWhenTrackingIDs("PNTHEXP" COMMA) source->contentPos, source->endContentPos, source->owningFile)
, expression(pExpression) {
}
ParenthesizedExpression::~ParenthesizedExpression() {
	delete expression;
}
Cast::Cast(CType* pType, bool pIsRaw, AbstractCodeBlock* source)
: Operator(onlyWhenTrackingIDs("CAST" COMMA) OperatorType::Cast, source->contentPos, source->endContentPos, source->owningFile)
, type(pType)
, isRaw(pIsRaw) {
}
Cast::~Cast() {
	//don't delete the type since it's owned by something else
}
FunctionCall::FunctionCall(Token* pFunction, Array<Token*>* pArguments)
: Token(onlyWhenTrackingIDs("FNCALL" COMMA) pFunction->contentPos, pFunction->endContentPos, pFunction->owningFile)
, function(pFunction)
, arguments(pArguments) {
}
FunctionCall::~FunctionCall() {
	delete function;
	arguments->deleteContents();
	delete arguments;
}
FunctionDefinition::FunctionDefinition(CType* pReturnType, Array<CVariableDefinition*>* pParameters, StatementList* pBody,
	int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: Token(onlyWhenTrackingIDs("FNDEF" COMMA) pContentPos, pEndContentPos, pOwningFile)
, returnType(pReturnType)
, parameters(pParameters)
, body(pBody) {
}
FunctionDefinition::~FunctionDefinition() {
	//don't delete the type since it's owned by something else
	parameters->deleteContents();
	delete parameters;
	delete body;
}
