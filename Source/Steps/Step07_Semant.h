class Pliers;
class SourceFile;
class CVariableDefinition;
class VariableDefinitionList;
class Token;
class Identifier;
class IntConstant;
class FloatConstant;
class BoolConstant;
class StringLiteral;
class Operator;
class DirectiveTitle;
template <class KeyElement, class Value> class PrefixTrie;

class Semant {
public:
	static void semant(Pliers* pliers);
private:
	static void addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFile(SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantToken(Token* t, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantIdentifier(Identifier* i, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantIntConstant(IntConstant* i, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFloatConstant(FloatConstant* f, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantBoolConstant(BoolConstant* b, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantStringLiteral(StringLiteral* s, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantOperator(Operator* o, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantDirectiveTitle(DirectiveTitle* d, PrefixTrie<char, CVariableDefinition*>* variables);
};
