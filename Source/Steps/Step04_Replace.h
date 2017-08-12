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
	static void replaceAbstractContents(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces);
	static void addReplacesToTrie(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces);
	static void replaceTokens(Array<Token*>* source, Array<Token*>* result, PrefixTrie<char, CDirectiveReplace*>* replaces,
		SubstitutedToken* substitutions);
	static Token* cloneSubstitutions(SubstitutedToken* substitutions, Token* token);
};
