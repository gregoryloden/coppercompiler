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
, val(pVal)
, negative(false) {
	dataType = CDataType::infiniteByteSizeIntType;
}
IntConstant::IntConstant(IntConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, val(new BigInt(cloneSource->val, false))
, negative(cloneSource->negative) {
}
IntConstant::~IntConstant() {
	delete val;
}
cloneWithReplacementSourceForType(IntConstant)
FloatConstant::FloatConstant(
	BigInt* pSignificand, int pFractionDigits, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, significand(pSignificand)
, fractionDigits(pFractionDigits)
, exponent(pExponent)
, negative(false) {
	dataType = CDataType::infinitePrecisionFloatType;
//TODO:
//	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant::FloatConstant(FloatConstant* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, significand(new BigInt(cloneSource->significand, false))
, fractionDigits(cloneSource->fractionDigits)
, exponent(cloneSource->exponent)
, negative(cloneSource->negative) {
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
, precedence(OperatorTypePrecedence::Assignment)
, semanticsType(OperatorSemanticsType::AnyAny)
, modifiesVariable(false)
, left(nullptr)
, right(nullptr) {
	switch (pOperatorType) {
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
			precedence = OperatorTypePrecedence::StaticMember;
			//StaticOperators don't use semantics type
			break;
		case OperatorType::Dot:
			precedence = OperatorTypePrecedence::ObjectMember;
			//TODO: classes, semantics type for Dot operators
			break;
		case OperatorType::ObjectMemberAccess:
			precedence = OperatorTypePrecedence::ObjectMemberAccess;
			//TODO: classes, semantics type for Dot operators
			break;
		case OperatorType::VariableLogicalNot:
			precedence = OperatorTypePrecedence::Postfix;
			semanticsType = OperatorSemanticsType::SingleBoolean;
			modifiesVariable = true;
			break;
		case OperatorType::Increment:
		case OperatorType::Decrement:
		case OperatorType::VariableBitwiseNot:
			precedence = OperatorTypePrecedence::Postfix;
			semanticsType = OperatorSemanticsType::SingleInteger;
			modifiesVariable = true;
			break;
		case OperatorType::VariableNegate:
			precedence = OperatorTypePrecedence::Postfix;
			semanticsType = OperatorSemanticsType::SingleNumber;
			modifiesVariable = true;
			break;
		case OperatorType::Cast:
			precedence = OperatorTypePrecedence::Prefix;
			//Casts don't use semantics type
			break;
		case OperatorType::LogicalNot:
			precedence = OperatorTypePrecedence::Prefix;
			semanticsType = OperatorSemanticsType::SingleBoolean;
			break;
		case OperatorType::BitwiseNot:
			precedence = OperatorTypePrecedence::Prefix;
			semanticsType = OperatorSemanticsType::SingleInteger;
			break;
		case OperatorType::Negate:
			precedence = OperatorTypePrecedence::Prefix;
			semanticsType = OperatorSemanticsType::SingleNumber;
			break;
		case OperatorType::Multiply:
		case OperatorType::Divide:
		case OperatorType::Modulus:
			precedence = OperatorTypePrecedence::Multiplication;
			semanticsType = OperatorSemanticsType::NumberNumber;
			break;
		case OperatorType::Add:
			precedence = OperatorTypePrecedence::Addition;
			semanticsType = OperatorSemanticsType::NumberNumberOrStringString;
			break;
		case OperatorType::Subtract:
			precedence = OperatorTypePrecedence::Addition;
			semanticsType = OperatorSemanticsType::NumberNumber;
			break;
		case OperatorType::ShiftLeft:
		case OperatorType::ShiftRight:
		case OperatorType::ShiftArithmeticRight:
//		case OperatorType::RotateLeft:
//		case OperatorType::RotateRight:
			precedence = OperatorTypePrecedence::BitShift;
			semanticsType = OperatorSemanticsType::IntegerIntegerBitShift;
			break;
		case OperatorType::BitwiseAnd:
			precedence = OperatorTypePrecedence::BitwiseAnd;
			semanticsType = OperatorSemanticsType::IntegerInteger;
			break;
		case OperatorType::BitwiseXor:
			precedence = OperatorTypePrecedence::BitwiseXor;
			semanticsType = OperatorSemanticsType::IntegerInteger;
			break;
		case OperatorType::BitwiseOr:
			precedence = OperatorTypePrecedence::BitwiseOr;
			semanticsType = OperatorSemanticsType::IntegerInteger;
			break;
		case OperatorType::Equal:
		case OperatorType::NotEqual:
			precedence = OperatorTypePrecedence::Comparison;
			//semanticsType is AnyAny by default
			break;
		case OperatorType::LessOrEqual:
		case OperatorType::GreaterOrEqual:
		case OperatorType::LessThan:
		case OperatorType::GreaterThan:
			precedence = OperatorTypePrecedence::Comparison;
			semanticsType = OperatorSemanticsType::NumberNumber;
			break;
		case OperatorType::BooleanAnd:
			precedence = OperatorTypePrecedence::BooleanAnd;
			semanticsType = OperatorSemanticsType::BooleanBoolean;
			break;
		case OperatorType::BooleanOr:
			precedence = OperatorTypePrecedence::BooleanOr;
			semanticsType = OperatorSemanticsType::BooleanBoolean;
			break;
		case OperatorType::Colon:
		case OperatorType::QuestionMark:
			precedence = OperatorTypePrecedence::Ternary;
			semanticsType = OperatorSemanticsType::Ternary;
			break;
		case OperatorType::Assign:
			//precedence is Assignment by default
			//semanticsType is AnyAny by default
			modifiesVariable = true;
			break;
		case OperatorType::AssignAdd:
			//precedence is Assignment by default
			semanticsType = OperatorSemanticsType::NumberNumberOrStringString;
			modifiesVariable = true;
			break;
		case OperatorType::AssignSubtract:
		case OperatorType::AssignMultiply:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
			//precedence is Assignment by default
			semanticsType = OperatorSemanticsType::NumberNumber;
			modifiesVariable = true;
			break;
		case OperatorType::AssignShiftLeft:
		case OperatorType::AssignShiftRight:
		case OperatorType::AssignShiftArithmeticRight:
//		case OperatorType::AssignRotateLeft:
//		case OperatorType::AssignRotateRight:
			//precedence is Assignment by default
			semanticsType = OperatorSemanticsType::IntegerIntegerBitShift;
			modifiesVariable = true;
			break;
		case OperatorType::AssignBitwiseAnd:
		case OperatorType::AssignBitwiseXor:
		case OperatorType::AssignBitwiseOr:
			//precedence is Assignment by default
			semanticsType = OperatorSemanticsType::IntegerInteger;
			modifiesVariable = true;
			break;
//		case OperatorType::AssignBooleanAnd:
//		case OperatorType::AssignBooleanOr:
//			//precedence is Assignment by default
//			semanticsType = OperatorSemanticsType::BooleanBoolean;
//			modifiesVariable = true;
//			break;
		case OperatorType::None:
		default:
			Error::makeError(ErrorType::CompilerIssue, "determining the operator precedence of this operator", this);
			break;
	}
}
Operator::Operator(Operator* cloneSource, Identifier* pReplacementSource)
: LexToken(cloneSource, pReplacementSource)
, operatorType(cloneSource->operatorType)
, precedence(cloneSource->precedence)
, semanticsType(cloneSource->semanticsType)
, modifiesVariable(cloneSource->modifiesVariable)
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
	if (right != nullptr
			&& (visitor->shouldHandleBooleanRightSide()
				|| (operatorType != OperatorType::BooleanAnd && operatorType != OperatorType::BooleanOr)))
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
, directives(cloneSource->directives) {
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
Cast::Cast(CDataType* pType, bool pIsRaw, Token* source)
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
GroupToken::GroupToken(Array<Token*>* pValues, Identifier* source)
: Token(onlyWhenTrackingIDs("GROUP" COMMA) source->contentPos, source->endContentPos, source->owningFile)
, values(pValues) {
	replacementSource = source->replacementSource;
}
GroupToken::~GroupToken() {
	values->deleteContents();
	delete values;
}
//iterate the grouped values
void GroupToken::visitSubtokens(TokenVisitor* visitor) {
	forEach(Token*, v, values, vi) {
		visitor->handleExpression(v);
	}
}
