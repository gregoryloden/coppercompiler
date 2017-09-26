#include "../General/globals.h"
#include "string"
using namespace std;

class SourceFile;
class CDirective;
class BigInt;
class CType;
template <class Type> class Array;

enum class SeparatorType: unsigned char {
	LeftParenthesis = 0x1,
	RightParenthesis = 0x2,
	Comma = 0x4,
	Semicolon = 0x8
};
enum class OperatorType: unsigned char {
//	None,
	Dot,
	ObjectMemberAccess,
	FunctionCall,
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
	AssignBitwiseOr
//	AssignBooleanAnd,
//	AssignBooleanOr
};
enum class OperatorTypePrecedence: unsigned char {
	ObjectMember = 14,
	Postfix = 13,
	Prefix = 12,
	Multiplication = 11,
	Addition = 10,
	BitShift = 9,
	BitwiseAnd = 8,
	BitwiseXor = 7,
	BitwiseOr = 6,
	Comparison = 5,
	BooleanAnd = 4,
	BooleanOr = 3,
	Ternary = 2,
	Assignment = 1
};

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
	#ifdef TRACK_OBJ_IDS
		Operator(OperatorType pType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	#endif
	Operator(onlyWhenTrackingIDs(char* pObjType COMMA) OperatorType pType, int pContentPos, int pEndContentPos,
		SourceFile* pOwningFile);
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

	void replaceResultingToken(Token* newResultingToken);
};

//Tokens used in expressions
class ParenthesizedExpression: public Token {
public:
	ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source);
	virtual ~ParenthesizedExpression();

	Token* expression;
};
class Cast: public Operator {
public:
	Cast(CType* pType, bool pRaw, AbstractCodeBlock* source);
	virtual ~Cast();

	CType* type;
	bool raw;
};
class FunctionCall: public Operator {
public:
	FunctionCall(Token* function, Array<Token*>* pArguments);
	virtual ~FunctionCall();

	Array<Token*>* arguments;
};
