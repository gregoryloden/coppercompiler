#include "../General/globals.h"
#include "string"
using namespace std;

class SourceFile;
class CDirective;
class BigInt;
template <class Type> class Array;

enum SeparatorType: unsigned char {
	LeftParenthesis = 0x1,
	RightParenthesis = 0x2,
	Comma = 0x4,
	Semicolon = 0x8
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
	Cast,
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
	BooleanOr,
	Colon,
	QuestionMark,
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
//	AssignBooleanOr
};
enum OperatorTypePrecedence: unsigned char {
	PrecedenceObjectMember = 14,
	PrecedencePostfix = 13,
	PrecedencePrefix = 12,
	PrecedenceMultiplication = 11,
	PrecedenceAddition = 10,
	PrecedenceBitShift = 9,
	PrecedenceBitwiseAnd = 8,
	PrecedenceBitwiseXor = 7,
	PrecedenceBitwiseOr = 6,
	PrecedenceComparison = 5,
	PrecedenceBooleanAnd = 4,
	PrecedenceBooleanOr = 3,
	PrecedenceTernary = 2,
	PrecedenceAssignment = 1
};

//Booleans, etc. group on the right; additions, etc. groups on the left

class Token onlyInDebug(: public ObjCounter) {
protected:
	Token(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
public:
	virtual ~Token();

	int contentPos; //copper: readonly
	int endContentPos; //copper: readonly
	SourceFile* owningFile; //copper: readonly

	int getRow();
	static Token* getResultingToken(Token* t);
};
//For empty contents (ex. errors, between commas or semicolons, etc.)
class EmptyToken: public Token {
public:
	EmptyToken(int pContentPos, SourceFile* pOwningFile);
	virtual ~EmptyToken();
};

//Tokens used in lexing
class LexToken: public Token {
protected:
	LexToken(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
public:
	virtual ~LexToken();
};
class Identifier: public LexToken {
public:
	Identifier(string pName, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~Identifier();

	string name; //copper: readonly
};
class IntConstant: public LexToken {
public:
	IntConstant(int pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~IntConstant();

	int val; //copper: readonly
};
class FloatConstant: public LexToken {
public:
	FloatConstant(BigInt* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~FloatConstant();

//	static const int FLOAT_TOO_BIG_EXPONENT = 0x100000;
	BigInt* significand; //copper: readonly
	int exponent; //copper: readonly
};
class StringLiteral: public LexToken {
public:
	StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~StringLiteral();

	string val; //copper: readonly
};
class Separator: public LexToken {
public:
	Separator(SeparatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~Separator();

	SeparatorType type; //copper: readonly

	static string separatorName(SeparatorType s);
};
class Operator: public LexToken {
public:
	Operator(OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~Operator();

	OperatorType type; //copper: readonly
	OperatorTypePrecedence precedence; //copper: readonly
	Token* left; //copper: readonly
	Token* right; //copper: readonly

	bool takesRightHandPrecedence(Operator* other);
};
class DirectiveTitle: public LexToken {
public:
	DirectiveTitle(string pTitle, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~DirectiveTitle();

	string title; //copper: readonly
	CDirective* directive; //copper: readonly<ParseDirectives>
};

//Tokens used in parsing
class AbstractCodeBlock: public Token {
public:
	AbstractCodeBlock(
		Array<Token*>* pTokens, Array<CDirective*>* pDirectives, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	virtual ~AbstractCodeBlock();

	Array<Token*>* tokens; //copper: readonly
	Array<CDirective*>* directives; //copper: readonly
};

//Tokens used in replacing
class SubstitutedToken: public Token {
public:
	SubstitutedToken(Token* pResultingToken, bool pShouldDelete, Token* tokenBeingReplaced);
	virtual ~SubstitutedToken();

	Token* resultingToken; //copper: readonly
	bool shouldDelete;
};

//Tokens used in expressions
class ParenthesizedExpression: public Token {
public:
	ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* pSource);
	~ParenthesizedExpression();

	Token* expression;
};
//class IdentifierList: public Token {
//public:
//	IdentifierList(Identifier* pI1, Identifier* pI2);
//	virtual ~IdentifierList();
//
//	Array<Identifier*> identifiers;
//};
