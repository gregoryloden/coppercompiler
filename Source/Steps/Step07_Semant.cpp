#include "Project.h"

//verify that all types are correct, variable names match, etc.

//validate the semantics for all files
void Semant::semant(Pliers* pliers) {
	//start by gathering the trie of all global variables, as well as which ones are initialized
	if (pliers->printProgress)
		printf("Gathering all global variables...\n");
	PrefixTrie<char, CVariableDefinition*> allGlobalVariables;
	PrefixTrie<char, CVariableData*> allGlobalVariableData;
	forEach(SourceFile*, s, pliers->allFiles, si1) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			VariableDefinitionList* v;
			if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
			else if ((o = dynamic_cast<Operator*>(t)) != nullptr &&
				(v = dynamic_cast<VariableDefinitionList*>(o->left)) != nullptr)
			{
				addVariablesToTrie(v, &allGlobalVariables);
				forEach(CVariableDefinition*, c, v->variables, ci) {
					CVariableData* d = new CVariableData();
					d->dataBitmask |= CVariableData::isInitialized;
					delete allGlobalVariableData.set(c->name->name.c_str(), c->name->name.length(), d);
				}
			}
		}
	}

	//then go through all the files
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		if (pliers->printProgress)
			printf("Analyzing semantics for %s...\n", s->filename.c_str());
		semantFile(s, &allGlobalVariables, &allGlobalVariableData);
	}
}
//add variables from the definition list into the trie
void Semant::addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables) {
	forEach(CVariableDefinition*, c, v->variables, ci) {
		CVariableDefinition* old = variables->set(c->name->name.c_str(), c->name->name.length(), c);
		if (old != nullptr) {
			variables->set(old->name->name.c_str(), old->name->name.length(), old);
			string errorMessage = "\"" + old->name->name + "\" has already been defined";
			Error::logError(ErrorType::General, errorMessage.c_str(), c->name);
		}
	}
}
//validate the semantics in the given file
void Semant::semantFile(
	SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o;
		VariableDefinitionList* v;
		if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
			Error::logError(ErrorType::Expected, "a variable initalization", t);
			continue;
		} else if ((v = dynamic_cast<VariableDefinitionList*>(o->left)) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable declaration list", t);
			continue;
		}
		semantToken(o, variables, variableData, nullptr);
	}
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything under it is also valid
void Semant::semantToken(
	Token* t,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
	Identifier* i;
	IntConstant* ic;
	FloatConstant* f;
	BoolConstant* b;
	StringLiteral* s;
	Operator* o;
	DirectiveTitle* d;
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr)
		semantIdentifier(i, variables, variableData, typeExpected);
	else if ((ic = dynamic_cast<IntConstant*>(t)) != nullptr)
		semantIntConstant(ic, variables, variableData, typeExpected);
	else if ((f = dynamic_cast<FloatConstant*>(t)) != nullptr)
		semantFloatConstant(f, variables, variableData, typeExpected);
	else if ((b = dynamic_cast<BoolConstant*>(t)) != nullptr)
		semantBoolConstant(b, variables, variableData, typeExpected);
	else if ((s = dynamic_cast<StringLiteral*>(t)) != nullptr)
		semantStringLiteral(s, variables, variableData, typeExpected);
	else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		semantOperator(o, variables, variableData, typeExpected);
	else if ((d = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
		semantDirectiveTitle(d, variables, variableData, typeExpected);
	else {
		assert(false);
	}
}
//verify that this identifier ????????????????
void Semant::semantIdentifier(
	Identifier* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this int constant ????????????????
void Semant::semantIntConstant(
	IntConstant* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this float constant ????????????????
void Semant::semantFloatConstant(
	FloatConstant* f,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this bool constant ????????????????
void Semant::semantBoolConstant(
	BoolConstant* b,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this string literal ????????????????
void Semant::semantStringLiteral(
	StringLiteral* s,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this operator ????????????????
void Semant::semantOperator(
	Operator* o,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
}
//verify that this directive title ????????????????
void Semant::semantDirectiveTitle(
	DirectiveTitle* d,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	CDataType* typeExpected)
{
	assert(false); //TODO: do something with directive titles
}

//TODO:
//-make sure variable initializations are either parenthesized or top-level in statements
//-make sure variable initializations in parentheses have values
//-make sure assignment operators are either parenthesized or top-level in statements
//-make sure all global variables are variable definition lists or assignments to variable definition lists
//-make sure variable initializations match the initial values (groups with all the types, or one value matching all the types)
//-make sure ternary operators are well-formed
/*
				//in an expression, declared variables must be initialized
				if (variableInitialization->initialization == nullptr) {
					Deleter<VariableInitialization> variableInitializationDeleter (variableInitialization);
					Error::makeError(ErrorType::ExpectedToFollow, "a variable initialization", variableInitialization);
				}
*/
// Error::makeError(ErrorType::General, "ternary expression missing condition", oNew);
