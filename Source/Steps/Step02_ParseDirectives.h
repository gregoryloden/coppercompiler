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
class Token;
enum class SeparatorType: unsigned char;
template <class Type> class Array;

class ParseDirectives {
private:
	static thread_local SourceFile* sourceFile;
	static thread_local Token* searchOrigin;

public:
	static void parseDirectives(SourceFile* newSourceFile);
private:
	static AbstractCodeBlock* parseAbstractCodeBlock(bool endsWithParenthesis, int contentPos);
	static CDirective* completeDirective(DirectiveTitle* dt);
	static CDirectiveReplace* completeDirectiveReplace(bool replaceInput, DirectiveTitle* endOfFileErrorToken);
	static CDirectiveInclude* completeDirectiveInclude(bool includeAll, DirectiveTitle* endOfFileErrorToken);
	template <class TokenType> static TokenType* parseToken(const char* expectedTokenTypeName, Token* endOfFileErrorToken);
	static int parseSeparator(SeparatorType type, const char* expectedTokenTypeName, Token* endOfFileErrorToken);
	static Array<string>* parseReplaceParameters(Identifier* endOfFileErrorToken);
};
