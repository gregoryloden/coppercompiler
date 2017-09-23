#include "string"
using namespace std;

class SourceFile;
class CDirectiveReplace;
class AbstractCodeBlock;
class Identifier;
class StringLiteral;
class SubstitutedToken;
class Token;
template <class Key, class Value> class PrefixTrie;
template <class Type> class Array;

class Replace {
public:
	static void replaceCodeInFiles(Array<SourceFile*>* files);
private:
	static void addReplacesToTrie(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces);
	static void replaceTokens(Array<Token*>* tokens, PrefixTrie<char, CDirectiveReplace*>* replaces);
	static Array<AbstractCodeBlock*>* collectArguments(
		AbstractCodeBlock* argumentsCodeBlock, int expectedArgumentCount, Token* errorToken);
	static SubstitutedToken* substituteTokens(Token* tokenBeingReplaced, Token* resultingToken, bool deleteResultingToken);
	static Array<Token*>* buildReplacement(Array<Token*>* tokensOutput, AbstractCodeBlock* replacementBody,
		Array<AbstractCodeBlock*>* arguments, Array<string>* input, Token* tokenBeingReplaced);
	static StringLiteral* replaceStringLiteral(StringLiteral* s, Array<AbstractCodeBlock*>* arguments, Array<string>* input);
	static void replaceIdentifier(Array<Token*>* tokensOutput, Identifier* i, Array<AbstractCodeBlock*>* arguments,
		Array<string>* input, Token* tokenBeingReplaced);
};
