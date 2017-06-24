#include "string"
using namespace std;

class SourceFile;
class AbstractCodeBlock;
class CDirective;
class CDirectiveReplace;
class CDirectiveInclude;
class DirectiveTitle;
class Identifier;
class Separator2;
enum SeparatorType: unsigned char;
template <class Type> class Array;

class ParseDirectives {
private:
	static thread_local SourceFile* sourceFile;

public:
	static void parseDirectives(SourceFile* newSourceFile);
private:
	static AbstractCodeBlock* parseAbstractCodeBlock(bool endsWithParenthesis);
	static CDirective* completeDirective(DirectiveTitle* dt);
	static CDirectiveReplace* completeDirectiveReplace(bool replaceInput);
	static CDirectiveInclude* completeDirectiveInclude(bool includeAll);
	template <class TokenType> static TokenType* parseToken(char* expectedTokenTypeName);
	static Identifier* parseIdentifier();
	static Separator2* parseSeparator();
	static void parseSeparator(SeparatorType type);
	static Array<string>* parseParenthesizedCommaSeparatedIdentifierList();
	static void makeUnexpectedTokenError(char* expectedTokenTypeName, Token* t);
};
