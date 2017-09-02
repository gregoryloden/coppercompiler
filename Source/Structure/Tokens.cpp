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
IntConstant2::IntConstant2(int pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("INTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
}
IntConstant2::~IntConstant2() {}
FloatConstant2::FloatConstant2(BigInt2* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("FLTCNST" COMMA) pContentPos, pEndContentPos, pOwningFile)
, significand(pSignificand)
, exponent(pExponent) {
	int expbias = 1 == 1 ? 1023/* double */ : 127/* float */;
}
FloatConstant2::~FloatConstant2() {
	delete significand;
}
StringLiteral::StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("STRING" COMMA) pContentPos, pEndContentPos, pOwningFile)
, val(pVal) {
}
StringLiteral::~StringLiteral() {}
Separator2::Separator2(SeparatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("SEPRATR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, type(pType) {
}
Separator2::~Separator2() {}
Operator::Operator(OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile)
: LexToken(onlyWhenTrackingIDs("OPERATR" COMMA) pContentPos, pEndContentPos, pOwningFile)
, type(pType)
, left(nullptr)
, right(nullptr) {
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
//IdentifierList::IdentifierList(Identifier* pI1, Identifier* pI2)
//: Token("IDFRLST", pI1->contentPos)
//, identifiers(pI1) {
//	identifiers.add(pI2);
//}
//IdentifierList::~IdentifierList() {}
