class SourceFile;
class CDirectiveReplace;
class AbstractCodeBlock;
class Identifier;
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
	static Array<Token*>* simpleReplace(AbstractCodeBlock* abstractContents, Token* tokenBeingReplaced);
	static SubstitutedToken* substituteTokens(Token* tokenBeingReplaced, Token* resultingToken);
};
