class Pliers;
class SourceFile;
class CVariableDefinition;
class VariableDefinitionList;
template <class KeyElement, class Value> class PrefixTrie;

class Semant {
public:
	static void semant(Pliers* pliers);
private:
	static void addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFile(SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantToken(Token* t, PrefixTrie<char, CVariableDefinition*>* variables);
};
