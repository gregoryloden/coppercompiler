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
	static void semantToken(
		Token* t,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantIdentifier(
		Identifier* i,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantIntConstant(
		IntConstant* i,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantFloatConstant(
		FloatConstant* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantOperator(
		Operator* o,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantDirectiveTitle(
		DirectiveTitle* d,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantCast(
		Cast* c,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantStaticOperator(
		StaticOperator* s,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantFunctionCall(
		FunctionCall* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantFunctionDefinition(
		FunctionDefinition* f,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
	static void semantGroup(
		Group* g,
		PrefixTrie<char, CVariableDefinition*>* variables,
		PrefixTrie<char, CVariableData*>* variableData,
		CDataType* typeExpected,
		bool baseToken);
};
