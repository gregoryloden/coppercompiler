#include "Project.h"

Token::Token(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs(pObjType)))
contentPos(pContentPos) {
}
Token::~Token() {}
EmptyToken::EmptyToken(int pContentPos)
: Token(onlyWhenTrackingIDsWithComma("EMPTTKN") pContentPos) {
}
EmptyToken::~EmptyToken() {}
LexToken::LexToken(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos)
: Token(onlyWhenTrackingIDsWithComma(pObjType) pContentPos) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("IDNTFR") pContentPos)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant2::IntConstant2(int pVal, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("INTCNST") pContentPos)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pMantissa, int pExponent, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("FLTCNST") pContentPos)
, mantissa(new BigInt2(pMantissa))
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {
	delete mantissa;
}
StringLiteral::StringLiteral(string pVal, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("STRING") pContentPos)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("SEPRATR") pContentPos)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("OPERATR") pContentPos)
, type(pType)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	//do not delete left or right because they are maintained by the abstract code block
}
DirectiveTitle::DirectiveTitle(string pTitle, int pContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("DCTVTTL") pContentPos)
, title(pTitle)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {
	delete directive;
}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives)
: Token(onlyWhenTrackingIDsWithComma("ABCDBLK") 0)
, tokens(pTokens)
, directives(pDirectives) {
	if (pTokens->length >= 1) {
		Token* token = pTokens->first();
		contentPos = token->contentPos;
	}
}
AbstractCodeBlock::~AbstractCodeBlock() {
	tokens->deleteSelfAndContents();
	delete directives; // do not delete the contents as they are owned by the DirectiveTitles
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDFRLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
