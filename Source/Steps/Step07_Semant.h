class Pliers;
class SourceFile;
class CVariableDefinition;
class VariableDeclarationList;
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
class Statement;
template <class KeyElement, class Value> class PrefixTrie;
template <class Type> class Array;

enum class SemantExpressionLevel: unsigned char {
	TopLevel,
	TopLevelInParentheses,
	Subexpression
};
enum class OperatorSemanticsType: unsigned char {
	SingleBoolean,
	SingleInteger,
	SingleNumber,
	BooleanBoolean,
	IntegerInteger,
	IntegerIntegerBitShift,
	NumberNumber,
	NumberNumberOrStringString,
	AnyAny,
	Ternary
};
class Semant {
private:
	static thread_local bool shouldSemantStatements;

public:
	static void semant(Pliers* pliers);
private:
	static void addVariablesToTrie(VariableDeclarationList* v, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFileDefinitions(
		SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, Array<Operator*>* redoVariables);
	static void semantFileStatements(SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantToken(
		Token* t, PrefixTrie<char, CVariableDefinition*>* variables, SemantExpressionLevel semantExpressionLevel);
	static void semantIdentifier(Identifier* i, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantDirectiveTitle(DirectiveTitle* d, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantCast(Cast* c, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantStaticOperator(StaticOperator* s, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantOperator(
		Operator* o, PrefixTrie<char, CVariableDefinition*>* variables, SemantExpressionLevel semantExpressionLevel);
	static void semantFunctionCall(FunctionCall* f, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantFunctionDefinition(FunctionDefinition* f, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantGroup(Group* g, PrefixTrie<char, CVariableDefinition*>* variables);
	static void semantStatementList(Array<Statement*>* statements);
	static bool tokenHasKnownType(Token* t);
	static bool checkType(Token* t, CDataType* expectedType);
};
