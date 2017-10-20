class SourceFile;
class Pliers;
template <class Type> class Array;
template <class KeyElement, class Value> class PrefixTrie;

class Include {
private:
	static thread_local Pliers* currentPliers;
	static thread_local PrefixTrie<char, SourceFile*>* filesByName;

public:
	static void loadFiles(Pliers* pliers);
private:
	static SourceFile* newSourceFile(const char* fileName);
};
