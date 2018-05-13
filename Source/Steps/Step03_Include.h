class SourceFile;
class Pliers;
class Path;
class StringLiteral;
template <class Type> class Array;
template <class KeyElement, class Value> class PrefixTrie;

class Include {
private:
	static thread_local Pliers* currentPliers;
	static thread_local PrefixTrie<char, SourceFile*>* filesByName;

public:
	static void loadFiles(Pliers* pliers);
private:
	static void resolveIncludedFiles(
		SourceFile* file, Path* currentPath, Path* includedPath, StringLiteral* inclusionSource, bool wasWildcard);
	static SourceFile* getSourceFile(Path* path, StringLiteral* inclusionSource, bool wasWildcard);
};
