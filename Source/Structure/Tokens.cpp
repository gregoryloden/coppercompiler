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
//subclasses with subtokens (function statements don't count) will use the visitor to iterate over them
void Token::visitSubtokens(TokenVisitor* t) {}
TokenVisitor::TokenVisitor(onlyWhenTrackingIDs(char* pObjType))
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs(pObjType))) {
}
TokenVisitor::~TokenVisitor() {}
//some iterations should only iterate on tokens that will definitely evaluate
bool TokenVisitor::shouldHandleBooleanRightSide() {
	return true;
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
, name(pName)
, variable(nullptr) {
}
Identifier::Identifier(Identifier* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, name(cloneSource->name)
, variable(nullptr) {
}
Identifier::~Identifier() {
	//don't delete variable because it's owned by something else
}
cloneWithReplacementSourceForType(Identifier)
IntConstant::IntConstant(BigInt* pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
	dataType = CDataType::infiniteByteSizeIntType;
}
IntConstant::IntConstant(IntConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, val(new BigInt(cloneSource->val, false)) {
}
IntConstant::~IntConstant() {
	delete val;
}
cloneWithReplacementSourceForType(IntConstant)
FloatConstant::FloatConstant(
	BigInt* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, significand(pSignificand)
, exponent(pExponent) {
	dataType = CDataType::infinitePrecisionFloatType;
//TODO:
//	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
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
BoolConstant::BoolConstant(bool pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("BOLCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
	dataType = CDataType::boolType;
}
BoolConstant::BoolConstant(BoolConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, val(cloneSource->val) {
}
BoolConstant::~BoolConstant() {}
cloneWithReplacementSourceForType(BoolConstant)
StringLiteral::StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("STRING" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
	dataType = CDataType::stringType;
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
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
			precedence = OperatorTypePrecedence::StaticMember;
			break;
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
		default:
			Error::makeError(ErrorType::CompilerIssue, "determining the operator precedence of this operator", this);
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
//iterate the left token, and the right token if the visitor accepts it
void Operator::visitSubtokens(TokenVisitor* visitor) {
	if (left != nullptr)
		visitor->handleExpression(left);
	if (right != nullptr &&
			(visitor->shouldHandleBooleanRightSide() ||
				(operatorType != OperatorType::BooleanAnd && operatorType != OperatorType::BooleanOr)))
		visitor->handleExpression(right);
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
VariableDeclarationList::VariableDeclarationList(Array<CVariableDefinition*>* pVariables, Identifier* firstType)
: Token(onlyWhenTrackingIDs("VRDFLST" COMMA) firstType->contentPos, firstType->endContentPos, firstType->owningFile)
, variables(pVariables) {
	replacementSource = firstType->replacementSource;
	redetermineType();
}
VariableDeclarationList::~VariableDeclarationList() {
	variables->deleteContents();
	delete variables;
}
//check all the types of the inner variables to get the net type of this variable declaration list
void VariableDeclarationList::redetermineType() {
	if (variables->length == 1)
		dataType = variables->get(0)->type;
	else {
		Array<CVariableDefinition*>* types = new Array<CVariableDefinition*>();
		forEach(CVariableDefinition*, c, variables, ci) {
			types->add(new CVariableDefinition(c->type, nullptr));
		}
		dataType = CGenericGroup::typeFor(types);
	}
}
ParenthesizedExpression::ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source)
: Token(onlyWhenTrackingIDs("PNTHEXP" COMMA) source->contentPos, source->endContentPos, source->owningFile)
, expression(pExpression) {
	replacementSource = source->replacementSource;
}
ParenthesizedExpression::~ParenthesizedExpression() {
	delete expression;
}
//iterate the subtoken
void ParenthesizedExpression::visitSubtokens(TokenVisitor* visitor) {
	visitor->handleExpression(expression);
}
Cast::Cast(CDataType* pType, bool pIsRaw, AbstractCodeBlock* source)
: Operator(onlyWhenTrackingIDs("CAST" COMMA) OperatorType::Cast, source->contentPos, source->endContentPos, source->owningFile)
, isRaw(pIsRaw)
, castType(pType) {
	replacementSource = source->replacementSource;
}
Cast::~Cast() {}
StaticOperator::StaticOperator(CDataType* pOwnerType, OperatorType pType, Operator* source)
: Operator(onlyWhenTrackingIDs("STCOPER" COMMA) pType, source->contentPos, source->endContentPos, source->owningFile)
, ownerType(pOwnerType) {
	replacementSource = source->replacementSource;
}
StaticOperator::~StaticOperator() {
	//don't delete ownerType since it's owned by something else
}
FunctionCall::FunctionCall(Token* pFunction, Array<Token*>* pArguments, AbstractCodeBlock* argumentsBlock)
: Token(
	onlyWhenTrackingIDs("FNCALL" COMMA) argumentsBlock->contentPos, argumentsBlock->endContentPos, argumentsBlock->owningFile)
, function(pFunction)
, arguments(pArguments) {
	replacementSource = argumentsBlock->replacementSource;
}
FunctionCall::~FunctionCall() {
	delete function;
	arguments->deleteContents();
	delete arguments;
}
//iterate the function itself and all its arguments
void FunctionCall::visitSubtokens(TokenVisitor* visitor) {
	visitor->handleExpression(function);
	forEach(Token*, a, arguments, ai) {
		visitor->handleExpression(a);
	}
}
FunctionDefinition::FunctionDefinition(
	CDataType* pReturnType, Array<CVariableDefinition*>* pParameters, Array<Statement*>* pBody, Identifier* typeToken)
: Token(onlyWhenTrackingIDs("FNDEF" COMMA) typeToken->contentPos, typeToken->endContentPos, typeToken->owningFile)
, returnType(pReturnType)
, parameters(pParameters)
, body(pBody) {
	replacementSource = typeToken->replacementSource;
	Array<CDataType*>* parameterTypes = new Array<CDataType*>();
	forEach(CVariableDefinition*, c, pParameters, ci) {
		parameterTypes->add(c->type);
	}
	dataType = CGenericFunction::typeFor(pReturnType, parameterTypes);
}
FunctionDefinition::~FunctionDefinition() {
	//don't delete the return type since it's owned by something else
	parameters->deleteContents();
	delete parameters;
	body->deleteContents();
	delete body;
}
Group::Group(Array<Token*>* pValues, Identifier* source)
: Token(onlyWhenTrackingIDs("GROUP" COMMA) source->contentPos, source->endContentPos, source->owningFile)
, values(pValues) {
	replacementSource = source->replacementSource;
}
Group::~Group() {
	values->deleteContents();
	delete values;
}
//iterate the grouped values
void Group::visitSubtokens(TokenVisitor* visitor) {
	forEach(Token*, v, values, vi) {
		visitor->handleExpression(v);
	}
}
