class SourceFile;
class CDirectiveReplace;
class AbstractCodeBlock;
template <class Type> class Array;

class Replace {
private:
	static thread_local PrefixTrie<char, CDirectiveReplace*>* replaces;
	static thread_local Array<Token*>* resultContents;

public:
	static void replaceCodeInFiles(Array<SourceFile*>* files);
private:
	static void replaceCode(AbstractCodeBlock* abstractContents, SourceFile* owningFile);
};
