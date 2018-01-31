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

	allGlobalVariableData.deleteValues();
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
		if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
			continue;
		else if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
			Error::logError(ErrorType::Expected, "a variable initalization", t);
			continue;
		} else if ((v = dynamic_cast<VariableDefinitionList*>(o->left)) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable declaration list", t);
			continue;
		}
		semantToken(o, variables, variableData, true);
	}
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything under it is also valid
Token* Semant::semantToken(
	Token* t,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	Identifier* i;
	IntConstant* ic;
	FloatConstant* f;
	Operator* o;
	DirectiveTitle* d;
	ParenthesizedExpression* p;
	Cast* c;
	StaticOperator* s;
	FunctionCall* fc;
	FunctionDefinition* fd;
	Group* g;
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr)
		return semantIdentifier(i, variables, variableData, baseToken);
	else if ((ic = dynamic_cast<IntConstant*>(t)) != nullptr)
		return semantIntConstant(ic, variables, variableData, baseToken);
	else if ((f = dynamic_cast<FloatConstant*>(t)) != nullptr)
		return semantFloatConstant(f, variables, variableData, baseToken);
	else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		return semantOperator(o, variables, variableData, baseToken);
	else if ((d = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
		return semantDirectiveTitle(d, variables, variableData, baseToken);
	else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr)
		return semantToken(p->expression, variables, variableData, true);
	else if ((c = dynamic_cast<Cast*>(t)) != nullptr)
		return semantCast(c, variables, variableData, baseToken);
	else if ((s = dynamic_cast<StaticOperator*>(t)) != nullptr)
		return semantStaticOperator(s, variables, variableData, baseToken);
	else if ((fc = dynamic_cast<FunctionCall*>(t)) != nullptr)
		return semantFunctionCall(fc, variables, variableData, baseToken);
	else if ((fd = dynamic_cast<FunctionDefinition*>(t)) != nullptr)
		return semantFunctionDefinition(fd, variables, variableData, baseToken);
	else if ((g = dynamic_cast<Group*>(t)) != nullptr)
		return semantGroup(g, variables, variableData, baseToken);
	else {
		assert(false);
		return t;
	}
}
//verify that this identifier ????????????????
Token* Semant::semantIdentifier(
	Identifier* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return i;
}
//assign the right size to this int constant
Token* Semant::semantIntConstant(
	IntConstant* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	if (i->val == (int)((char)i->val))
		i->dataType = CDataType::byteType;
	else if (i->val == (int)((short)i->val))
		i->dataType = CDataType::shortType;
	else
		i->dataType = CDataType::intType;
	return i;
}
//verify that this float constant ????????????????
Token* Semant::semantFloatConstant(
	FloatConstant* f,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return f;
}
//verify that this operator ????????????????
Token* Semant::semantOperator(
	Operator* o,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return o;
}
//verify that this directive title ????????????????
Token* Semant::semantDirectiveTitle(
	DirectiveTitle* d,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	assert(false); //TODO: do something with directive titles
	//TODO: ???????????
	return d;
}
//verify that this cast ????????????????
Token* Semant::semantCast(
	Cast* c,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return c;
}
//verify that this static operator ????????????????
Token* Semant::semantStaticOperator(
	StaticOperator* s,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return s;
}
//verify that this function call ????????????????
Token* Semant::semantFunctionCall(
	FunctionCall* f,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return f;
}
//verify that this function definition ????????????????
Token* Semant::semantFunctionDefinition(
	FunctionDefinition* f,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	//TODO: ???????????
	return f;
}
//verify that this group ????????????????
Token* Semant::semantGroup(
	Group* g,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	assert(false); //TODO: Groups
	//TODO: ???????????
	return g;
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
