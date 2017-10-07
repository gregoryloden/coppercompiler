class SourceFile;
class AbstractCodeBlock;
class CType;
class CVariableDefinition;
class Token;
class SubstitutedToken;
class StatementList;
class ExpressionStatement;
template <class Type> class Array;
template <class Type> class ArrayIterator;

class ParseExpressions {
public:
	static void parseExpressionsInFiles(Array<SourceFile*>* files);
private:
	static void parseGlobalDefinitions(SourceFile* sf);
	static CVariableDefinition* completeVariableDefinition(CType* type, Token* fullToken, ArrayIterator<Token*>* ti);
	static Token* parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
		char* emptyExpressionErrorMessage, Token* emptyExpressionErrorToken);
	static Token* getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti);
	static Token* addToOperator(Operator* o, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* evaluateAbstractCodeBlock(
		AbstractCodeBlock* a, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* completeCast(CType* type, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti);
	static Token* completeFunctionDefinition(
		CType* type, Token* fullToken, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti);
	static Token* completeFunctionCall(Token* function, AbstractCodeBlock* argumentsBlock);
	template <class TokenType> static TokenType* getExpectedToken(
		ArrayIterator<Token*>* ti, Token* precedingFullToken, const char* tokenDescription);
	static StatementList* parseStatements(AbstractCodeBlock* a);
};
