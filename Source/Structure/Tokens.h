#include "../General/globals.h"
#include "string"
using namespace std;

class CDirective;
class BigInt2;
template <class Type> class Array;

enum SeparatorType: unsigned char {
	LeftParenthesis,
	RightParenthesis,
	Comma,
	Semicolon
};
enum OperatorType: unsigned char {
//	None,
	Dot,
	ObjectMemberAccess,
	Increment,
	Decrement,
	VariableLogicalNot,
	VariableBitwiseNot,
	VariableNegate,
	LogicalNot,
	BitwiseNot,
	Negate,
	Multiply,
	Divide,
	Modulus,
	Add,
	Subtract,
	ShiftLeft,
	ShiftRight,
	ShiftArithmeticRight,
//	RotateLeft,
//	RotateRight,
	BitwiseAnd,
	BitwiseXor,
	BitwiseOr,
	Equal,
	NotEqual,
	LessOrEqual,
	GreaterOrEqual,
	LessThan,
	GreaterThan,
	BooleanAnd,
//	BooleanXor,
	BooleanOr,
	QuestionMark,
	Colon,
	Assign,
	AssignAdd,
	AssignSubtract,
	AssignMultiply,
	AssignDivide,
	AssignModulus,
	AssignShiftLeft,
	AssignShiftRight,
	AssignShiftArithmeticRight,
//	AssignRotateLeft,
//	AssignRotateRight,
	AssignBitwiseAnd,
	AssignBitwiseXor,
	AssignBitwiseOr,
//	AssignBooleanAnd,
//	AssignBooleanXor,
//	AssignBooleanOr
};

//Booleans, etc. group on the right; additions, etc. groups on the left

class Token onlyInDebug(: public ObjCounter) {
protected:
	Token(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos);
public:
	virtual ~Token();

	int contentPos; //copper: readonly
	int row; //copper: readonly
	int rowStartContentPos; //copper: readonly
};
//For empty contents (ex. errors, between commas or semicolons, etc.)
class EmptyToken: public Token {
public:
	EmptyToken(int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~EmptyToken();
};

//Tokens used in lexing
class LexToken: public Token {
protected:
	LexToken(onlyWhenTrackingIDsWithComma(char* pObjType) int pContentPos, int pRow, int pRowStartContentPos);
public:
	virtual ~LexToken();
};
class Identifier: public LexToken {
public:
	Identifier(string pName, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~Identifier();

	string name; //copper: readonly
};
class IntConstant2: public LexToken {
public:
	IntConstant2(int pVal, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~IntConstant2();

	int val; //copper: readonly
};
class FloatConstant2: public LexToken {
public:
	FloatConstant2(BigInt2* pMantissa, int pExponent, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~FloatConstant2();

	static const int FLOAT_TOO_BIG_EXPONENT = 0x100000;
	BigInt2* mantissa; //copper: readonly
	int exponent; //copper: readonly
};
class StringLiteral: public LexToken {
public:
	StringLiteral(string pVal, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~StringLiteral();

	string val; //copper: readonly
};
class Separator2: public LexToken {
public:
	Separator2(SeparatorType pType, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~Separator2();

	SeparatorType type; //copper: readonly
};
class Operator: public LexToken {
public:
	Operator(OperatorType pType, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~Operator();

	OperatorType type; //copper: readonly
	Token* left; //copper: readonly
	Token* right; //copper: readonly
};
class DirectiveTitle: public LexToken {
public:
	DirectiveTitle(string pTitle, int pContentPos, int pRow, int pRowStartContentPos);
	virtual ~DirectiveTitle();

	string title; //copper: readonly
	CDirective* directive; //copper: readonly
};

//Tokens used in parsing
class AbstractCodeBlock: public Token {
public:
	AbstractCodeBlock(Array<Token*>* pTokens, Array<CDirective*>* pDirectives);
	virtual ~AbstractCodeBlock();

	Array<Token*>* tokens; //copper: readonly
	Array<CDirective*>* directives; //copper: readonly
};
//class IdentifierList: public Token {
//public:
//	IdentifierList(Identifier* pI1, Identifier* pI2);
//	virtual ~IdentifierList();
//
//	Array<Identifier*> identifiers;
//};