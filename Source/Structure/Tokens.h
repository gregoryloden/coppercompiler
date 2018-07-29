#ifndef TOKENS_H
#define TOKENS_H
#include "../General/globals.h"

class SourceFile;
class CDirective;
class BigInt;
class CDataType;
class CVariableDefinition;
class Statement;
class LexToken;
class TokenVisitor;
class TempStorage;
class AssemblyInstruction;
template <class Type> class Array;

enum class SeparatorType: unsigned char {
	LeftParenthesis = 0x1,
	RightParenthesis = 0x2,
	Comma = 0x4,
	Semicolon = 0x8
};
enum class OperatorType: unsigned char {
	None,
	StaticDot,
	StaticMemberAccess,
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
	StaticMember = 16,
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
public:
	int contentPos; //copper: readonly
	int endContentPos; //copper: readonly
	SourceFile* owningFile; //copper: readonly
	Identifier* replacementSource; //copper: readonly
	CDataType* dataType; //copper: readonly<Semant>

protected:
	Token(onlyWhenTrackingIDs(char* pObjType COMMA) int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	Token(Token* cloneSource, Identifier* pReplacementSource);
public:
	virtual ~Token();

	virtual void visitSubtokens(TokenVisitor* visitor);
};
class TokenVisitor onlyInDebug(: public ObjCounter) {
protected:
	TokenVisitor(onlyWhenTrackingIDs(char* pObjType));
public:
	virtual ~TokenVisitor();

	virtual void handleExpression(Token* t) = 0;
	virtual bool shouldHandleBooleanRightSide();
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
	string name; //copper: readonly
	CVariableDefinition* variable; //copper: private<Semant>

	Identifier(string pName, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	Identifier(Identifier* cloneSource, Identifier* pReplacementSource);
	virtual ~Identifier();

	Identifier* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class IntConstant: public LexToken {
public:
	BigInt* val; //copper: readonly

	IntConstant(BigInt* pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	IntConstant(IntConstant* cloneSource, Identifier* pReplacementSource);
	virtual ~IntConstant();

	IntConstant* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class FloatConstant: public LexToken {
public:
//	static const int FLOAT_TOO_BIG_EXPONENT = 0x100000;

	BigInt* significand; //copper: readonly
	int fractionDigits; //copper: readonly
	int exponent; //copper: readonly

	FloatConstant(
		BigInt* pSignificand, int pFractionDigits, int pExponent, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	FloatConstant(FloatConstant* cloneSource, Identifier* pReplacementSource);
	virtual ~FloatConstant();

	FloatConstant* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class BoolConstant: public LexToken {
public:
	bool val; //copper: readonly

	BoolConstant(bool pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	BoolConstant(BoolConstant* cloneSource, Identifier* pReplacementSource);
	virtual ~BoolConstant();

	BoolConstant* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class StringLiteral: public LexToken {
public:
	string val; //copper: readonly

	StringLiteral(string pVal, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	StringLiteral(StringLiteral* cloneSource, Identifier* pReplacementSource);
	virtual ~StringLiteral();

	StringLiteral* cloneWithReplacementSource(Identifier* pReplacementSource);
};
class Separator: public LexToken {
public:
	SeparatorType separatorType; //copper: readonly

	Separator(SeparatorType pSeparatorType, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	Separator(Separator* cloneSource, Identifier* pReplacementSource);
	virtual ~Separator();

	Separator* cloneWithReplacementSource(Identifier* pReplacementSource);
	static string separatorName(SeparatorType s, bool withIndefiniteArticle);
};
class Operator: public LexToken {
public:
	OperatorType operatorType; //copper: readonly
	OperatorTypePrecedence precedence; //copper: readonly
	bool modifiesVariable; //copper: readonly
	Token* left; //copper: readonly
	Token* right; //copper: readonly

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

	Operator* cloneWithReplacementSource(Identifier* pReplacementSource);
	virtual void visitSubtokens(TokenVisitor* visitor);
};
class DirectiveTitle: public LexToken {
public:
	string title; //copper: readonly
	CDirective* directive; //copper: readonly<ParseDirectives>

	DirectiveTitle(string pTitle, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	DirectiveTitle(DirectiveTitle* cloneSource, Identifier* pReplacementSource);
	virtual ~DirectiveTitle();

	DirectiveTitle* cloneWithReplacementSource(Identifier* pReplacementSource);
};

//Tokens used in parsing
class AbstractCodeBlock: public Token {
public:
	Array<Token*>* tokens; //copper: readonly
	Array<CDirective*>* directives; //copper: readonly

	AbstractCodeBlock(
		Array<Token*>* pTokens, Array<CDirective*>* pDirectives, int pContentPos, int pEndContentPos, SourceFile* pOwningFile);
	AbstractCodeBlock(Array<Token*>* pTokens, AbstractCodeBlock* cloneSource, Identifier* pReplacementSource);
	virtual ~AbstractCodeBlock();
};

//Tokens used in expressions
class VariableDeclarationList: public Token {
public:
	Array<CVariableDefinition*>* variables;

	VariableDeclarationList(Array<CVariableDefinition*>* pVariables, Identifier* firstType);
	virtual ~VariableDeclarationList();

	void redetermineType();
};
class ParenthesizedExpression: public Token {
public:
	Token* expression;

	ParenthesizedExpression(Token* pExpression, AbstractCodeBlock* source);
	virtual ~ParenthesizedExpression();

	virtual void visitSubtokens(TokenVisitor* visitor);
};
class Cast: public Operator {
public:
	bool isRaw;
	CDataType* castType;

	Cast(CDataType* pType, bool pIsRaw, AbstractCodeBlock* source);
	virtual ~Cast();
};
class StaticOperator: public Operator {
public:
	CDataType* ownerType;

	StaticOperator(CDataType* pOwnerType, OperatorType pType, Operator* source);
	virtual ~StaticOperator();
};
class FunctionCall: public Token {
public:
	Token* function;
	Array<Token*>* arguments;

	FunctionCall(Token* pFunction, Array<Token*>* pArguments, AbstractCodeBlock* argumentsBlock);
	virtual ~FunctionCall();

	virtual void visitSubtokens(TokenVisitor* visitor);
};
class FunctionDefinition: public Token {
public:
	CDataType* returnType;
	Array<CVariableDefinition*>* parameters;
	Array<Statement*>* body;
	bool eligibleForRegisterParameters;
	TempStorage* resultStorage;
	Array<AssemblyInstruction*>* instructions;

	FunctionDefinition(
		CDataType* pReturnType, Array<CVariableDefinition*>* pParameters, Array<Statement*>* pBody, Identifier* typeToken);
	virtual ~FunctionDefinition();
};
class Group: public Token {
public:
	Array<Token*>* values;

	Group(Array<Token*>* pValues, Identifier* source);
	virtual ~Group();

	virtual void visitSubtokens(TokenVisitor* visitor);
};
#endif
