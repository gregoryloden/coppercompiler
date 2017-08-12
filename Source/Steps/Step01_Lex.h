class SourceFile;
class LexToken;
class StringLiteral;
class IntConstant2;
class Separator2;
class Operator;
class DirectiveTitle;
enum ErrorType: unsigned char;
template <class Type> class Array;
//class IntConstant;

class Lex {
private:
	static thread_local SourceFile* sourceFile;
	static thread_local char* contents;
	static thread_local int contentsLength;
	static thread_local int pos;
	static thread_local Array<int>* rowStarts;
	static thread_local char c;
	#ifdef DEBUG
		static thread_local Array<int> debugRowStarts;
	#endif

public:
	static void initializeLexer(SourceFile* newSourceFile);
	static LexToken* lex();
	#ifdef DEBUG
		static void printToken(LexToken* t);
	#endif
private:
	static bool skipWhitespace();
	static bool skipComment();
	static LexToken* lexIdentifier();
	static LexToken* lexNumber();
	static char cToDigit();
	static StringLiteral* lexString();
	static char nextStringCharacter();
	static IntConstant2* lexCharacter();
	static Separator2* lexSeparator();
	static Operator* lexOperator();
	static DirectiveTitle* lexDirectiveTitle();
public:
	static void makeLexError(ErrorType type, char* message);
private:
	static void makeLexError(ErrorType type, char* message, Token* errorToken);
};
//void buildTokens();
//void castConstant(IntConstant* c, int context);
