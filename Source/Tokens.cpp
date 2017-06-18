#include "Project.h"

Token::Token(onlyInDebugWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos)
: onlyInDebugWithComma(ObjCounter(pObjType))
contentPos(pContentPos)
, row(pRow)
, rowStartContentPos(pRowStartContentPos) {
}
Token::~Token() {}
EmptyToken::EmptyToken(int pContentPos, int pRow, int pRowStartContentPos)
: Token(onlyInDebugWithComma("EMTKN") pContentPos, pRow, pRowStartContentPos) {
}
EmptyToken::~EmptyToken() {}
LexToken::LexToken(onlyInDebugWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos)
: Token(onlyInDebugWithComma(pObjType) pContentPos, pRow, pRowStartContentPos) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("IDNTFR") pContentPos, pRow, pRowStartContentPos)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant2::IntConstant2(int pVal, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("ICNST") pContentPos, pRow, pRowStartContentPos)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pMantissa, int pExponent, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("FCNST") pContentPos, pRow, pRowStartContentPos)
, mantissa(pMantissa)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {}
StringLiteral::StringLiteral(string pVal, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("STRNG") pContentPos, pRow, pRowStartContentPos)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("SEPR") pContentPos, pRow, pRowStartContentPos)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("OPER") pContentPos, pRow, pRowStartContentPos)
, type(pType)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	//do not delete left or right because they are maintained by the abstract code block
}
DirectiveTitle::DirectiveTitle(string pTitle, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyInDebugWithComma("DTTL") pContentPos, pRow, pRowStartContentPos)
, title(pTitle)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {
	//do not delete directive because it will be deleted in the abstract code block directives array
}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives)
: Token(onlyInDebugWithComma("ACBLK") 0, 0, 0)
, tokens(pTokens)
, directives(pDirectives) {
	if (pTokens->getLength() >= 1) {
		Token* token = pTokens->first();
		contentPos = token->contentPos;
		row = token->row;
		rowStartContentPos = token->rowStartContentPos;
	}
}
AbstractCodeBlock::~AbstractCodeBlock() {
	Memory::deleteArrayAndContents(tokens);
	Memory::deleteArrayAndContents(directives);
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
