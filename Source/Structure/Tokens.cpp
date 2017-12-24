#include "Project.h"

#define cloneWithReplacementSourceForType(Type) \
	Type* Type::cloneWithReplacementSource(Identifier* pReplacementSource) { return new Type(this, pReplacementSource); }

Token::Token(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
contentPos(pContentPos)
, endContentPos(pEndContentPos)
, owningFile(pOwningFile)
, replacementSource(nullptr)
, dataType(nullptr) {
}
Token::Token(Token* cloneSource, Identifier* pReplacementSource)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(cloneSource)) COMMA)
contentPos(cloneSource->contentPos)
, endContentPos(cloneSource->endContentPos)
, owningFile(cloneSource->owningFile)
, replacementSource(pReplacementSource)
, dataType(cloneSource->dataType) {
}
Token::~Token() {
	//don't delete replacementSource, owningFile or dataType, they're owned by something else
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
Identifier::Identifier(Identifier* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, name(cloneSource->name) {
}
Identifier::~Identifier() {}
cloneWithReplacementSourceForType(Identifier)
IntConstant::IntConstant(int pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal)
, isBool(false) {
}
IntConstant::IntConstant(bool pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal ? 1 : 0)
, isBool(true) {
}
IntConstant::IntConstant(IntConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, val(cloneSource->val)
, isBool(cloneSource->isBool) {
}
IntConstant::~IntConstant() {}
cloneWithReplacementSourceForType(IntConstant)
FloatConstant::FloatConstant(
	BigInt* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, significand(pSignificand)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant::FloatConstant(FloatConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, significand(new BigInt(cloneSource->significand, false))
, exponent(cloneSource->exponent) {
}
FloatConstant::~FloatConstant() {
	delete significand;
}
cloneWithReplacementSourceForType(FloatConstant)
StringLiteral::StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("STRING" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
}
StringLiteral::StringLiteral(StringLiteral* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, val(cloneSource->val) {
}
StringLiteral::~StringLiteral() {}
cloneWithReplacementSourceForType(StringLiteral)
Separator::Separator(SeparatorType pSeparatorType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("SEPRATR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, separatorType(pSeparatorType) {
}
Separator::Separator(Separator* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, separatorType(cloneSource->separatorType) {
}
Separator::~Separator() {}
cloneWithReplacementSourceForType(Separator)
string Separator::separatorName(SeparatorType s, bool withIndefiniteArticle) {
	switch (s) {
		case SeparatorType::Semicolon: return withIndefiniteArticle ? "a semicolon" : "semicolon";
		case SeparatorType::LeftParenthesis: return withIndefiniteArticle ? "a left parenthesis" : "left parenthesis";
		case SeparatorType::RightParenthesis: return withIndefiniteArticle ? "a right parenthesis" : "right parenthesis";
		default: return withIndefiniteArticle ? "a comma" : "comma";
	}
}
#ifdef TRACK_OBJ_IDS
	Operator::Operator(OperatorType pOperatorType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
	: Operator("OPERATR", pOperatorType, pContentPos, pEndContentPos, pOwningFile) {
	}
#endif
Operator::Operator(
	onlyWhenTrackingIDs(char* pObjType COMMA)
	OperatorType pOperatorType,
	int pContentPos,
	int pEndContentPos,
	SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs(pObjType COMMA) pContentPos, pEndContentPos, pOwningFile)
, operatorType(pOperatorType)
, left(nullptr)
, right(nullptr) {
	switch (pOperatorType) {
//		case OperatorType::None:
		case OperatorType::Dot:
			precedence = OperatorTypePrecedence::ObjectMember;
			break;
		case OperatorType::ObjectMemberAccess:
			precedence = OperatorTypePrecedence::ObjectMemberAccess;
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
Operator::Operator(Operator* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, operatorType(cloneSource->operatorType)
, precedence(cloneSource->precedence)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	delete left;
	delete right;
}
cloneWithReplacementSourceForType(Operator)
//determine if this operator should steal the right side of the other operator
//may throw
bool Operator::takesRightSidePrecedence(Operator* other) {
	if (precedence != other->precedence)
		return precedence > other->precedence;
	//some operators group on the right, whereas most operators group on the left
	switch (precedence) {
		case OperatorTypePrecedence::Ternary:
			//beginning of a ternary, always group on the right
			if (operatorType == OperatorType::QuestionMark)
				return true;
			//this is a colon and the other one is a question mark- steal the right side if there isn't a colon already
			else if (other->operatorType == OperatorType::QuestionMark) {
				Operator* o;
				return (o = dynamic_cast<Operator*>(other->right)) == nullptr || o->operatorType != OperatorType::Colon;
			//this is a colon and the other one is a colon too
			//since we never steal the right side of a question mark, we can only get here on an error
			} else
				Error::makeError(ErrorType::General, "ternary expression missing conditional", this);
		case OperatorTypePrecedence::ObjectMemberAccess:
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
DirectiveTitle::DirectiveTitle(DirectiveTitle* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, title(cloneSource->title)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {
	//don't delete the directive, the SourceFile owns it
}
cloneWithReplacementSourceForType(DirectiveTitle)
AbstractCodeBlock::AbstractCodeBlock(
	Array<Token*>* pTokens, Array<CDirective*>* pDirectives, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: Token(onlyWhenTrackingIDs("ABCDBLK" COMMA) pContentPos, pEndContentPos, pOwningFile)
, tokens(pTokens)
, directives(pDirectives) {
}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, AbstractCodeBlock* cloneSource, Identifier* pReplacementSource)
: Token(cloneSource, pReplacementSource)
, tokens(pTokens)
, directives(nullptr) {
}
AbstractCodeBlock::~AbstractCodeBlock() {
	tokens->deleteContents();
	delete tokens;
	if (directives != nullptr) {
		directives->deleteContents();
		delete directives;
	}
}
VariableInitialization::VariableInitialization(
	Array<CVariableDefinition*>* pVariables, Token* pInitialization, Token* lastToken)
: Token(onlyWhenTrackingIDs("VARINIT" COMMA) lastToken->endContentPos, lastToken->endContentPos, lastToken->owningFile)
, variables(pVariables)
, initialization(pInitialization) {
	replacementSource = lastToken->replacementSource;
}
VariableInitialization::~VariableInitialization() {
	variables->deleteContents();
	delete variables;
	delete initialization;
}
ParenthesizedExpression::ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source)
: Token(onlyWhenTrackingIDs("PNTHEXP" COMMA) source->contentPos, source->endContentPos, source->owningFile)
, expression(pExpression) {
	replacementSource = source->replacementSource;
}
ParenthesizedExpression::~ParenthesizedExpression() {
	delete expression;
}
Cast::Cast(CDataType* pType, bool pIsRaw, AbstractCodeBlock* source)
: Operator(onlyWhenTrackingIDs("CAST" COMMA) OperatorType::Cast, source->contentPos, source->endContentPos, source->owningFile)
, isRaw(pIsRaw) {
	dataType = pType;
	replacementSource = source->replacementSource;
}
Cast::~Cast() {}
FunctionCall::FunctionCall(
	Token* pFunction, Array<Token*>* pArguments, AbstractCodeBlock* lastToken)
: Token(onlyWhenTrackingIDs("FNCALL" COMMA) lastToken->contentPos, lastToken->endContentPos, lastToken->owningFile)
, function(pFunction)
, arguments(pArguments) {
	replacementSource = lastToken->replacementSource;
}
FunctionCall::~FunctionCall() {
	delete function;
	arguments->deleteContents();
	delete arguments;
}
FunctionDefinition::FunctionDefinition(
	CDataType* pReturnType, Array<CVariableDefinition*>* pParameters, Array<Statement*>* pBody, Token* lastToken)
: Token(onlyWhenTrackingIDs("FNDEF" COMMA) lastToken->endContentPos, lastToken->endContentPos, lastToken->owningFile)
, returnType(pReturnType)
, parameters(pParameters)
, body(pBody) {
	replacementSource = lastToken->replacementSource;
}
FunctionDefinition::~FunctionDefinition() {
	//don't delete the return type since it's owned by something else
	parameters->deleteContents();
	delete parameters;
	body->deleteContents();
	delete body;
}
