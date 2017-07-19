class SourceFile;
class CDirectiveReplace;
class AbstractCodeBlock;
template <class Type> class Array;

class Replace {
public:
	static void replaceCode(Array<SourceFile*>* files);
	static void searchContents(PrefixTrie<char, CDirectiveReplace*>* replaces, AbstractCodeBlock* abstractContents);
};
