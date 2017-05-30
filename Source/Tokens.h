#include "string"
using namespace std;

class ObjCounter;
template <class type> class Array;

enum SeparatorType: unsigned char {
	LeftParenthesis,
	RightParenthesis,
	Comma,
	Semicolon
};
enum OperatorType: unsigned char {
//	None,
	Dot,
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

class Token
: public ObjCounter
 {
protected:
	Token(char* pObjType, size_t pContentPos);
public:
	virtual ~Token();

	size_t contentPos;
};

//Tokens used in lexing
class LexToken: public Token {
protected:
	LexToken(char* pObjType, size_t pContentPos);
public:
	virtual ~LexToken();
};
class Identifier: public LexToken {
public:
	Identifier(string pName, size_t pContentPos);
	virtual ~Identifier();

	string name;
};
class IntConstant2: public LexToken {
public:
	IntConstant2(int pVal, size_t pContentPos);
	virtual ~IntConstant2();

	int val;
};
class FloatConstant2: public LexToken {
public:
	FloatConstant2(BigInt2* pMantissa, int pExponent, size_t pContentPos);
	virtual ~FloatConstant2();

	static const int FLOAT_TOO_BIG_EXPONENT = 0x100000;

	BigInt2 mantissa;
	int exponent;
};
class StringLiteral: public LexToken {
public:
	StringLiteral(string pVal, size_t pContentPos);
	virtual ~StringLiteral();

	string val;
};
class Separator2: public LexToken {
public:
	Separator2(SeparatorType pType, size_t pContentPos);
	virtual ~Separator2();

	SeparatorType type;
};
class Operator: public LexToken {
public:
	Operator(OperatorType pType, size_t pContentPos);
	virtual ~Operator();

	OperatorType type;
	Token* left;
	Token* right;
};
class DirectiveTitle: public LexToken {
public:
	DirectiveTitle(string pTitle, size_t pContentPos);
	virtual ~DirectiveTitle();

	string title;
};

//Tokens used in parsing
class AbstractCodeBlock: public Token {
public:
	AbstractCodeBlock(size_t pContentPos);
	virtual ~AbstractCodeBlock();

	Array<Token*> tokens;
};
//class IdentifierList: public Token {
//public:
//	IdentifierList(Identifier* pI1, Identifier* pI2);
//	virtual ~IdentifierList();
//
//	Array<Identifier*> identifiers;
//};
