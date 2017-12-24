#include "../General/globals.h"
#include "string"
using namespace std;

class SourceFile;
class CDirective;
class BigInt;
class CDataType;
class CVariableDefinition;
class Statement;
class LexToken;
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
	ObjectMember = 15,
	ObjectMemberAccess = 14,
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
	Token(Token* cloneSource, Identifier* pReplacementSource);
public:
	virtual ~Token();

	int contentPos; //copper: readonly
	int endContentPos; //copper: readonly
	SourceFile* owningFile; //copper: readonly
	Identifier* replacementSource; //copper: readonly
	CDataType* dataType; //copper: readonly<Semant>
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
	LexToken(LexToken* cloneSource, Identifier* pReplacementSource): Token(cloneSource, pReplacementSource) {}
public:
	virtual ~LexToken();

	virtual LexToken* cloneWithReplacementSource(Identifier* pReplacementSource) = 0;
};
class Identifier: public LexToken {
public:
	Identifier(string pName, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	Identifier(Identifier* cloneSource, Identifier* pReplacementSource);
	virtual ~Identifier();

	string name; //copper: readonly

	Identifier* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class IntConstant: public LexToken {
public:
	IntConstant(int pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	IntConstant(bool pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	IntConstant(IntConstant* cloneSource, Identifier* pReplacementSource);
	virtual ~IntConstant();

	int val; //copper: readonly
	bool isBool; //copper: readonly

	IntConstant* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class FloatConstant: public LexToken {
public:
	FloatConstant(BigInt* pSignificand, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	FloatConstant(FloatConstant* cloneSource, Identifier* pReplacementSource);
	virtual ~FloatConstant();

//	static const int FLOAT_TOO_BIG_EXPONENT = 0x100000;
	BigInt* significand; //copper: readonly
	int exponent; //copper: readonly

	FloatConstant* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class StringLiteral: public LexToken {
public:
	StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	StringLiteral(StringLiteral* cloneSource, Identifier* pReplacementSource);
	virtual ~StringLiteral();

	string val; //copper: readonly

	StringLiteral* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class Separator: public LexToken {
public:
	Separator(SeparatorType pSeparatorType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	Separator(Separator* cloneSource, Identifier* pReplacementSource);
	virtual ~Separator();

	SeparatorType separatorType; //copper: readonly

	Separator* cloneWithReplacementSource(Identifier* pReplacementSource);
	static string separatorName(SeparatorType s, bool withIndefiniteArticle);
};
class Operator: public LexToken {
public:
	#ifdef TRACK_OBJ_IDS
		Operator(OperatorType pOperatorType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	#endif
	Operator(
		onlyWhenTrackingIDs(char* pObjType COMMA)
		OperatorType pOperatorType,
		int pContentPos,
		int pEndContentPos,
		SourceFile* pOwningFile);
	Operator(Operator* cloneSource, Identifier* pReplacementSource);
	virtual ~Operator();

	OperatorType operatorType; //copper: readonly
	OperatorTypePrecedence precedence; //copper: readonly
	Token* left; //copper: readonly
	Token* right; //copper: readonly

	Operator* cloneWithReplacementSource(Identifier* pReplacementSource);
	OperatorTypePrecedence getPrecedence(OperatorType pOperatorType);
	bool takesRightSidePrecedence(Operator* other);
};
class DirectiveTitle: public LexToken {
public:
	DirectiveTitle(string pTitle, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	DirectiveTitle(DirectiveTitle* cloneSource, Identifier* pReplacementSource);
	virtual ~DirectiveTitle();

	string title; //copper: readonly
	CDirective* directive; //copper: readonly<ParseDirectives>

	DirectiveTitle* cloneWithReplacementSource(Identifier* pReplacementSource);
};

//Tokens used in parsing
class AbstractCodeBlock: public Token {
public:
	AbstractCodeBlock(
		Array<Token*>* pTokens, Array<CDirective*>* pDirectives, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	AbstractCodeBlock(Array<Token*>* pTokens, AbstractCodeBlock* cloneSource, Identifier* pReplacementSource);
	virtual ~AbstractCodeBlock();

	Array<Token*>* tokens; //copper: readonly
	Array<CDirective*>* directives; //copper: readonly
};

//Tokens used in expressions
class VariableInitialization: public Token {
public:
	VariableInitialization(Array<CVariableDefinition*>* pVariables, Token* pInitialization, Token* lastToken);
	virtual ~VariableInitialization();

	Array<CVariableDefinition*>* variables;
	Token* initialization;
};
class ParenthesizedExpression: public Token {
public:
	ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source);
	virtual ~ParenthesizedExpression();

	Token* expression;
};
class Cast: public Operator {
public:
	Cast(CDataType* pType, bool pIsRaw, AbstractCodeBlock* source);
	virtual ~Cast();

	bool isRaw;
};
class FunctionCall: public Token {
public:
	FunctionCall(Token* pFunction, Array<Token*>* pArguments, AbstractCodeBlock* lastToken);
	virtual ~FunctionCall();

	Token* function;
	Array<Token*>* arguments;
};
class FunctionDefinition: public Token {
public:
	FunctionDefinition(
		CDataType* pReturnType, Array<CVariableDefinition*>* pParameters, Array<Statement*>* pBody, Token* lastToken);
	virtual ~FunctionDefinition();

	CDataType* returnType;
	Array<CVariableDefinition*>* parameters;
	Array<Statement*>* body;
};
