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
		AbstractCodeBlock* argumentsCodeBlock, int expectedArgumentCount, Identifier* errorToken);
	static void buildReplacement(Array<Token*>* tokensOutput, AbstractCodeBlock* replacementBody,
		Array<AbstractCodeBlock*>* arguments, Array<string>* input, Identifier* replacementSource);
	static StringLiteral* replaceStringLiteral(
		StringLiteral* s, Array<AbstractCodeBlock*>* arguments, Array<string>* input, Identifier* replacementSource);
	static void replaceIdentifier(Array<Token*>* tokensOutput, Identifier* i, Array<AbstractCodeBlock*>* arguments,
		Array<string>* input, Identifier* replacementSource);
};
