#include "Project.h"

Token::Token(char* pObjType, size_t pContentPos)
: ObjCounter(pObjType)
, contentPos(pContentPos) {
}
Token::~Token() {}
LexToken::LexToken(char* pObjType, size_t pContentPos)
: Token(pObjType, pContentPos) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, size_t pContentPos)
: LexToken("IDNTFR", pContentPos)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant2::IntConstant2(int pVal, size_t pContentPos)
: LexToken("ICNST", pContentPos)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pMantissa, int pExponent, size_t pContentPos)
: LexToken("FCNST", pContentPos)
, mantissa(pMantissa)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {}
StringLiteral::StringLiteral(string pVal, size_t pContentPos)
: LexToken("STRNG", pContentPos)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, size_t pContentPos)
: LexToken("SEPR", pContentPos)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, size_t pContentPos)
: LexToken("OPER", pContentPos)
, type(pType)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	delete left;
	delete right;
}
DirectiveTitle::DirectiveTitle(string pTitle, size_t pContentPos)
: LexToken("DTTL", pContentPos)
, title(pTitle)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives, size_t pContentPos)
: Token("ACBLK", pContentPos)
, tokens(pTokens)
, directives(pDirectives) {
}
AbstractCodeBlock::~AbstractCodeBlock() {
	delete tokens;
	delete directives;
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
