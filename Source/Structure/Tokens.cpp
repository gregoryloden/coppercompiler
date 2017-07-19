#include "Project.h"

Token::Token(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
contentPos(pContentPos) {
}
Token::~Token() {}
EmptyToken::EmptyToken(int pContentPos)
: Token(onlyWhenTrackingIDs("EMPTTKN" COMMA) pContentPos) {
}
EmptyToken::~EmptyToken() {}
LexToken::LexToken(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos)
: Token(onlyWhenTrackingIDs(pObjType COMMA) pContentPos) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, int pContentPos)
: LexToken(onlyWhenTrackingIDs("IDNTFR" COMMA) pContentPos)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant2::IntConstant2(int pVal, int pContentPos)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pMantissa, int pExponent, int pContentPos)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos)
, mantissa(new BigInt2(pMantissa))
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {
	delete mantissa;
}
StringLiteral::StringLiteral(string pVal, int pContentPos)
: LexToken(onlyWhenTrackingIDs("STRING" COMMA) pContentPos)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, int pContentPos)
: LexToken(onlyWhenTrackingIDs("SEPRATR" COMMA) pContentPos)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, int pContentPos)
: LexToken(onlyWhenTrackingIDs("OPERATR" COMMA) pContentPos)
, type(pType)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	//do not delete left or right because they are maintained by the abstract code block
}
DirectiveTitle::DirectiveTitle(string pTitle, int pContentPos)
: LexToken(onlyWhenTrackingIDs("DCTVTTL" COMMA) pContentPos)
, title(pTitle) {
}
DirectiveTitle::~DirectiveTitle() {}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives)
: Token(onlyWhenTrackingIDs("ABCDBLK" COMMA) 0)
, tokens(pTokens)
, directives(pDirectives) {
	if (pTokens->length >= 1) {
		Token* token = pTokens->first();
		contentPos = token->contentPos;
	}
}
AbstractCodeBlock::~AbstractCodeBlock() {
	tokens->deleteSelfAndContents();
	directives->deleteSelfAndContents();
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDFRLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
