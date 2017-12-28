class Pliers;
class SourceFile;

class Semant {
public:
	static void semant(Pliers* pliers);
private:
	static void semantFile(SourceFile* sourceFile);
};
