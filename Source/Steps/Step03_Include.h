class SourceFile;
template <class Type> class Array;

class Include {
public:
	static Array<SourceFile*>* loadFiles(char* baseFile);
};
