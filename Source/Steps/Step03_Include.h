class SourceFile;
template <class type> class Array;

class Include {
public:
	static Array<SourceFile*>* loadFiles(char* baseFile);
};
