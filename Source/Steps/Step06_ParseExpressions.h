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
	static void replaceResultingToken(SubstitutedToken* parentS, Token* resultingToken);
	static void parseGlobalDefinitions(SourceFile* sf);
	static void completeVariableDefinition(CType* type, Token* typeToken, ArrayIterator<Token*>* ti);
	static Token* parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
		char* emptyExpressionErrorMessage, Token* emptyExpressionErrorToken);
	static Token* getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti);
	static Token* addToOperator(Token* fullToken, Operator* o, Token* activeExpression, ArrayIterator<Token*>* ti);
};
