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
class Cast;
class StaticOperator;
class FunctionCall;
class FunctionDefinition;
class Group;
class CDataType;
class CVariableData;
template <class KeyElement, class Value> class PrefixTrie;

class Semant {
public:
	static void semant(Pliers* pliers);
private:
	static void addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFile(
		SourceFile* sourceFile,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData);
	static Token* semantToken(
		Token* t,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantIdentifier(
		Identifier* i,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantIntConstant(
		IntConstant* i,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantFloatConstant(
		FloatConstant* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantOperator(
		Operator* o,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantDirectiveTitle(
		DirectiveTitle* d,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantCast(
		Cast* c,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantStaticOperator(
		StaticOperator* s,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantFunctionCall(
		FunctionCall* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantFunctionDefinition(
		FunctionDefinition* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
	static Token* semantGroup(
		Group* g,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		bool baseToken);
};
