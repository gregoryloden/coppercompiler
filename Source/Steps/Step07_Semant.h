#include "../Structure/Tokens.h"

class Pliers;
class SourceFile;
class CVariableDefinition;
class CDataType;
class Statement;
template <class KeyElement, class Value> class PrefixTrie;
template <class Type> class Array;
enum class ErrorType: unsigned char;
enum class OperatorType: unsigned char;

enum class SemantExpressionLevel: unsigned char {
	Subexpression = 0,
	TopLevel = 1,
	TopLevelInParentheses = 3
};
enum class ScopeExitType: unsigned char {
	None = 0,
	LoopJump = 1,
	Return = 2
};

class Semant {
private:
	class FindVisibleVariableDefinitionsVisitor: public TokenVisitor {
	private:
		PrefixTrie<char, CVariableDefinition*>* variables;
		SourceFile* originFile;

	public:
		FindVisibleVariableDefinitionsVisitor(PrefixTrie<char, CVariableDefinition*>* pVariables, SourceFile* pOriginFile);
		virtual ~FindVisibleVariableDefinitionsVisitor();

		void handleExpression(Token* t);
		bool shouldHandleBooleanRightSide();
	};
	class SemantFileStatementsVisitor: public TokenVisitor {
	private:
		PrefixTrie<char, CVariableDefinition*>* variables;

	public:
		SemantFileStatementsVisitor(PrefixTrie<char, CVariableDefinition*>* pVariables);
		virtual ~SemantFileStatementsVisitor();

		void handleExpression(Token* t);
	};

	static thread_local bool skipStatementsWhileFindingGlobalVariableDefinitions;
	static thread_local bool resemantingGenericTypeVariables;

public:
	static void semant(Pliers* pliers);
private:
	static void addVariablesToTrie(
		Array<CVariableDefinition*>* v, PrefixTrie<char, CVariableDefinition*>* variables, SourceFile* originFile);
	static bool finalizeTypes(VariableDeclarationList* v, CDataType* valueType);
	static void semantFileDefinitions(
		SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, Array<Operator*>* redoVariables);
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
	static void semantGroup(GroupToken* g, PrefixTrie<char, CVariableDefinition*>* variables);
	static ScopeExitType semantStatementList(
		Array<Statement*>* statements,
		PrefixTrie<char, CVariableDefinition*>* previousVariables,
		Array<CVariableDefinition*>* functionParameters,
		CDataType* returnType);
	static bool tokenHasKnownType(Token* t);
	static Token* implicitCast(Token* t, CDataType* castType);
	static void logSemantError(ErrorType type, const char* message, Token* token);
	static void logSemantErrorWithErrorSourceAndOriginFile(
		ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile);
	static void logSemantErrorWithErrorCheck(ErrorType type, const char* message, Token* token, Token* errorCheck);
	static void logSemantErrorWithErrorSourceAndOriginFileWithErrorCheck(
		ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile, Token* errorCheck);
};
