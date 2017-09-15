#include "string"
using namespace std;

class SourceFile;
class AbstractCodeBlock;
class CDirective;
class CDirectiveReplace;
class CDirectiveInclude;
class DirectiveTitle;
class Identifier;
class Separator;
enum SeparatorType: unsigned char;
template <class Type> class Array;

class ParseDirectives {
private:
	static thread_local SourceFile* sourceFile;

public:
	static void parseDirectives(SourceFile* newSourceFile);
private:
	static AbstractCodeBlock* parseAbstractCodeBlock(bool endsWithParenthesis, int contentPos);
	static CDirective* completeDirective(DirectiveTitle* dt);
	static CDirectiveReplace* completeDirectiveReplace(bool replaceInput);
	static CDirectiveInclude* completeDirectiveInclude(bool includeAll);
	template <class TokenType> static TokenType* parseToken(const char* expectedTokenTypeName);
	static Identifier* parseIdentifier();
	static Separator* parseSeparator();
	static int parseSeparator(SeparatorType type);
	static Array<string>* parseParenthesizedCommaSeparatedIdentifierList();
	static void makeUnexpectedTokenError(const char* expectedTokenTypeName, Token* t);
	static void makeEndOfFileWhileSearchingError(const char* message);
};
