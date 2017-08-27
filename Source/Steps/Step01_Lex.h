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

public:
	static void initializeLexer(SourceFile* newSourceFile);
	static LexToken* lex();
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
	static void makeLexError(ErrorType type, char* message);
};
//void buildTokens();
//void castConstant(IntConstant* c, int context);
