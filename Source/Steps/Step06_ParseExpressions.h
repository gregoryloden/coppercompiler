#include "string"
using namespace std;

class AbstractCodeBlock;
class CDataType;
class VariableInitialization;
class CVariableDefinition;
class Token;
class Statement;
class Identifier;
class ExpressionStatement;
class DirectiveTitle;
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
	static void parseGlobalDefinitions(AbstractCodeBlock* a);
	static VariableInitialization* completeVariableInitialization(
		CDataType* type, Identifier* name, ArrayIterator<Token*>* ti, SeparatorType endingSeparatorType);
	static Token* parseExpression(
		ArrayIterator<Token*>* ti,
		unsigned char endingSeparatorTypes,
		ErrorType expectedExpressionErrorType,
		const char* expectedExpressionErrorMessage,
		Token* expectedExpressionErrorToken);
	static Token* parseValueExpression(Token* t, ArrayIterator<Token*>* ti);
	static Token* addToOperator(Operator* o, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* evaluateAbstractCodeBlock(AbstractCodeBlock* a, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* completeCast(CDataType* type, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti);
	static Token* completeParenthesizedExpression(AbstractCodeBlock* a, bool wrapExpression);
	static Token* completeFunctionDefinition(CDataType* type, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti);
	static Token* completeFunctionCall(Token* function, AbstractCodeBlock* argumentsBlock);
	static Array<Statement*>* parseStatementOrStatementList(
		ArrayIterator<Token*>* ti, Token* noValueErrorToken, const char* statementDescription);
	static Statement* parseStatement(ArrayIterator<Token*>* ti);
	static Statement* parseDirectiveTitleStatementList(ArrayIterator<Token*>* ti);
	static Statement* parseKeywordStatement(ArrayIterator<Token*>* ti);
	static ExpressionStatement* parseExpressionStatement(ArrayIterator<Token*>* ti);
	//helpers
	static Token* getLastToken(Token* t);
	static bool hasSemicolon(AbstractCodeBlock* a);
//	static bool isKeyword(string s);
	static string buildExpectedSeparatorErrorMessage(unsigned char expectedSeparatorTypesMask, bool concludeList);
};
