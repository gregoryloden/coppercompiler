#include "Project.h"

Token::Token(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs(pObjType)))
contentPos(pContentPos)
, row(pRow)
, rowStartContentPos(pRowStartContentPos) {
}
Token::~Token() {}
EmptyToken::EmptyToken(int pContentPos, int pRow, int pRowStartContentPos)
: Token(onlyWhenTrackingIDsWithComma("EMPTTKN") pContentPos, pRow, pRowStartContentPos) {
}
EmptyToken::~EmptyToken() {}
LexToken::LexToken(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos)
: Token(onlyWhenTrackingIDsWithComma(pObjType) pContentPos, pRow, pRowStartContentPos) {
}
LexToken::~LexToken() {}
Identifier::Identifier(string pName, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("IDNTFR") pContentPos, pRow, pRowStartContentPos)
, name(pName) {
}
Identifier::~Identifier() {}
IntConstant2::IntConstant2(int pVal, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("INTCNST") pContentPos, pRow, pRowStartContentPos)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pMantissa, int pExponent, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("FLTCNST") pContentPos, pRow, pRowStartContentPos)
, mantissa(pMantissa)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {}
StringLiteral::StringLiteral(string pVal, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("STRING") pContentPos, pRow, pRowStartContentPos)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("SEPRATR") pContentPos, pRow, pRowStartContentPos)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("OPERATR") pContentPos, pRow, pRowStartContentPos)
, type(pType)
, left(nullptr)
, right(nullptr) {
}
Operator::~Operator() {
	//do not delete left or right because they are maintained by the abstract code block
}
DirectiveTitle::DirectiveTitle(string pTitle, int pContentPos, int pRow, int pRowStartContentPos)
: LexToken(onlyWhenTrackingIDsWithComma("DCTVTTL") pContentPos, pRow, pRowStartContentPos)
, title(pTitle)
, directive(nullptr) {
}
DirectiveTitle::~DirectiveTitle() {
	delete directive;
}
AbstractCodeBlock::AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives)
: Token(onlyWhenTrackingIDsWithComma("ABCDBLK") 0, 0, 0)
, tokens(pTokens)
, directives(pDirectives) {
	if (pTokens->length >= 1) {
		Token* token = pTokens->first();
		contentPos = token->contentPos;
		row = token->row;
		rowStartContentPos = token->rowStartContentPos;
	}
}
AbstractCodeBlock::~AbstractCodeBlock() {
	Memory::deleteArrayAndContents(tokens);
	delete directives; // do not delete the contents as they are owned by the DirectiveTitles
}
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDFRLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
