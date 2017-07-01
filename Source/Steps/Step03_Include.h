class SourceFile;
template <class Type> class Array;
template <class KeyElement, class Value> class PrefixTrie;

class Include {
public:
	static Array<SourceFile*>* loadFiles(char* baseFileName);
private:
	static SourceFile* newSourceFile(
		const char* baseFileName, Array<SourceFile*>* allFiles, PrefixTrie<char, SourceFile*>* filesByName);
};
