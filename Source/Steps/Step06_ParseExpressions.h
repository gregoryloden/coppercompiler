#include "string"
using namespace std;

class AbstractCodeBlock;
class CDataType;
class VariableDeclarationList;
class CVariableDefinition;
class Token;
class Statement;
class Identifier;
class ExpressionStatement;
class DirectiveTitle;
class FunctionDefinition;
class Operator;
template <class Type> class Array;
template <class Type> class ArrayIterator;
enum class SeparatorType: unsigned char;
enum class ErrorType: unsigned char;

class ParseExpressions {
private:
	static const char* returnKeyword;
	static const char* ifKeyword;
	static const char* elseKeyword;
	static const char* forKeyword;
	static const char* whileKeyword;
	static const char* doKeyword;
	static const char* continueKeyword;
	static const char* breakKeyword;
	static const char* classKeyword;
	static const char* rawKeyword;

public:
	static void parseExpressionsInFiles(Pliers* pliers);
private:
	template <class TokenType> static TokenType* parseExpectedToken(
		ArrayIterator<Token*>* ti, Token* precedingToken, const char* tokenDescription);
	static void parseNamespaceDefinitions(AbstractCodeBlock* a, Array<Token*>* definitionList);
	static CDataType* parseType(Identifier* i, ArrayIterator<Token*>* ti);
	static Token* parseCommaInParenthesizedList(ArrayIterator<Token*>* ti);
	static VariableDeclarationList* completeVariableDeclarationList(
		CDataType* type, Identifier* typeToken, Identifier* name, ArrayIterator<Token*>* ti);
	static Token* parseExpression(
		ArrayIterator<Token*>* ti,
		unsigned char endingSeparatorTypes,
		ErrorType expectedExpressionErrorType,
		const char* expectedExpressionErrorMessage,
		Token* expectedExpressionErrorToken);
	static Token* parseValueExpression(Token* t, ArrayIterator<Token*>* ti);
	static Token* addToOperator(Operator* o, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* completeExpressionStartingWithType(CDataType* type, Identifier* typeToken, ArrayIterator<Token*>* ti);
	static Token* completeParenthesizedExpression(AbstractCodeBlock* a, ArrayIterator<Token*>* castI);
	static FunctionDefinition* completeFunctionDefinition(
		CDataType* type, Identifier* typeToken, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti);
	static Token* completeFunctionCall(Token* activeExpression, AbstractCodeBlock* argumentsBlock);
	static Token* completeCast(CDataType* type, bool rawCast, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti);
	static Array<Statement*>* parseStatementOrStatementList(
		ArrayIterator<Token*>* ti, Token* noValueErrorToken, const char* statementDescription);
	static Statement* parseStatement(Token* t, ArrayIterator<Token*>* ti, bool permitDirectiveStatementList);
	static Statement* parseDirectiveStatementList(Token* t, ArrayIterator<Token*>* ti);
	static Statement* parseKeywordStatement(Token* t, ArrayIterator<Token*>* ti);
	static ExpressionStatement* parseExpressionStatement(Token* t, ArrayIterator<Token*>* ti);
	//helpers
	static bool newOperatorTakesRightSidePrecedence(Operator* oNew, Operator* oOld);
	static Token* getLastToken(Token* t);
	static bool hasSemicolon(AbstractCodeBlock* a);
//	static bool isKeyword(string s);
	static string buildExpectedSeparatorErrorMessage(unsigned char expectedSeparatorTypesMask, bool concludeList);
};
