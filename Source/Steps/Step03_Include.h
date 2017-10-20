class SourceFile;
class Pliers;
template <class Type> class Array;
template <class KeyElement, class Value> class PrefixTrie;

class Include {
private:
	static thread_local Array<SourceFile*>* allFiles;
	static thread_local PrefixTrie<char, SourceFile*>* filesByName;

public:
	static Array<SourceFile*>* loadFiles(char* baseFileName, Pliers* owningPliers);
private:
	static SourceFile* newSourceFile(const char* fileName, Pliers* owningPliers);
};
