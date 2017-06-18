#include "string"
using namespace std;

class SourceFile;
class AbstractCodeBlock;
class CDirective;
class DirectiveTitle;
class Identifier;
class Separator2;
enum SeparatorType: unsigned char;
template <class type> class Array;

class ParseDirectives {
public:
	static void parseDirectives(SourceFile* newSourceFile);
private:
	static SourceFile* sourceFile;

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
