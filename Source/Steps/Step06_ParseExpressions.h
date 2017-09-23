class SourceFile;
class AbstractCodeBlock;
class CType;
class Token;
class SubstitutedToken;
template <class Type> class Array;
template <class Type> class ArrayIterator;

class ParseExpressions {
public:
	static void parseExpressionsInFiles(Array<SourceFile*>* files);
private:
	static void parseGlobalDefinitions(SourceFile* sf);
	static void completeVariableDefinition(CType* type, Token* typeToken, ArrayIterator<Token*>* ti);
	static Token* parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
		char* emptyExpressionErrorMessage, Token* emptyExpressionErrorToken);
	static Token* getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti);
	static Token* addToOperator(Operator* o, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* evaluateAbstractCodeBlock(
		AbstractCodeBlock* a, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti);
	static Token* completeCast(CType* type, Token* fullToken, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti);
	static Token* completeFunctionDefinition(CType* type, AbstractCodeBlock* parameters, ArrayIterator<Token*>* ti);
	static Token* completeFunctionCall(Token* function, AbstractCodeBlock* arguments);
};
