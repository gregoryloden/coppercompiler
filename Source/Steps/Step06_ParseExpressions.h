class SourceFile;
class AbstractCodeBlock;
template <class Type> class Array;

class ParseExpressions {
public:
	static void parseExpressions(Array<SourceFile*>* files);
	static void parseGlobalDefinitions(AbstractCodeBlock* a);
};
