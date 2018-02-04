#include "Project.h"

//verify that all types are correct, variable names match, etc.

thread_local bool Semant::shouldSemantStatements = false;
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

	//then go through all the files to find global variables
	Array<Operator*> redoVariables;
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileDefinitions(s, &allGlobalVariables, &allGlobalVariableData, &redoVariables);
	}

	//next go through the redo variables and make sure that all variables end up with a specific type
	//at the end, any remaining generic types must be either circular definitions or assigned the wrong type
	//wrong type errors were already handled and circular definition errors will be handled later
	bool fixedVariable = true;
	while (fixedVariable) {
		fixedVariable = false;
		for (int i = redoVariables.length - 1; i >= 0; i--) {
			bool needsRedo = false;
			Operator* o = redoVariables.get(i);
			VariableDefinitionList* v = dynamic_cast<VariableDefinitionList*>(o->left);
			forEach(CVariableDefinition*, c, v->variables, ci) {
				if (c->type == CDataType::functionType) {
					if (dynamic_cast<CSpecificFunction*>(o->right->dataType) != nullptr)
						c->type = o->right->dataType;
					else
						needsRedo = true;
				}
			}
			if (!needsRedo) {
				redoVariables.remove(i);
				fixedVariable = true;
			}
		}
	}

	//and now go through and semant all the rest of the code
	forEach(SourceFile*, s, pliers->allFiles, si3) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileStatements(s, &allGlobalVariables, &allGlobalVariableData);
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
//validate the semantics of the global variables in the given file
//does not look at function body statements, since we need to finalize global variable types first
void Semant::semantFileDefinitions(
	SourceFile* sourceFile,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	Array<Operator*>* redoVariables)
{
	shouldSemantStatements = false;
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o = nullptr;
		VariableDefinitionList* v;
		if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
			;
		else if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
			Error::logError(ErrorType::Expected, "a variable initalization", t);
			continue;
		} else if ((v = dynamic_cast<VariableDefinitionList*>(o->left)) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable declaration list", t);
			continue;
		} else
			semantToken(o, variables, variableData, true);

		//check for any generic-type variables in this variable definition
		forEach(CVariableDefinition*, c, v->variables, ci) {
			//TODO: check for groups
			CDataType* genericType = c->type;
			if (genericType == CDataType::functionType) {
				if (o == nullptr) {
					string errorMessage = "could not determine specific " + genericType->name + " type for " + c->name->name;
					Error::logError(ErrorType::General, errorMessage.c_str(), c->name);
				} else {
					redoVariables->add(o);
					break;
				}
			}
		}
	}
}
//????????????????????????????
void Semant::semantFileStatements(
	SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	shouldSemantStatements = true;
	//??????????????????????????????????
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
	DirectiveTitle* d;
	ParenthesizedExpression* p;
	Cast* c;
	StaticOperator* s;
	Operator* o;
	FunctionCall* fc;
	FunctionDefinition* fd;
	Group* g;
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr)
		return semantIdentifier(i, variables, variableData, baseToken, true);
	else if ((ic = dynamic_cast<IntConstant*>(t)) != nullptr)
		return semantIntConstant(ic, variables, variableData, baseToken);
	else if ((d = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
		return semantDirectiveTitle(d, variables, variableData, baseToken);
	else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr) {
		t = semantToken(p->expression, variables, variableData, true);
		p->expression = nullptr;
		delete p;
		return t;
	} else if ((c = dynamic_cast<Cast*>(t)) != nullptr)
		return semantCast(c, variables, variableData, baseToken);
	else if ((s = dynamic_cast<StaticOperator*>(t)) != nullptr)
		return semantStaticOperator(s, variables, variableData, baseToken);
	else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		return semantOperator(o, variables, variableData, baseToken);
	else if ((fc = dynamic_cast<FunctionCall*>(t)) != nullptr)
		return semantFunctionCall(fc, variables, variableData, baseToken);
	else if ((fd = dynamic_cast<FunctionDefinition*>(t)) != nullptr)
		return semantFunctionDefinition(fd, variables, variableData, baseToken);
	else if ((g = dynamic_cast<Group*>(t)) != nullptr)
		return semantGroup(g, variables, variableData, baseToken);
	else if (dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<BoolConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr ||
			dynamic_cast<VariableDefinitionList*>(t) != nullptr)
		return t;
	else {
		assert(false);
		return t;
	}
}
//give this identifier the right variable
Token* Semant::semantIdentifier(
	Identifier* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken,
	bool beingRead)
{
	i->variable = variables->get(i->name.c_str(), i->name.length());
	if (i->variable == nullptr) {
		string errorMessage = "\"" + i->name + "\" has not been defined";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
	i->dataType = i->variable->type;
	if (beingRead) {
		CVariableData* iVariableData = variableData->get(i->name.c_str(), i->name.length());
		if (iVariableData == nullptr || (iVariableData->dataBitmask & CVariableData::isInitialized) == 0) {
			string errorMessage = "\"" + i->name + "\" may not have been initialized";
			Error::logError(ErrorType::General, errorMessage.c_str(), i);
		}
	}
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
//verify that this static operator has a valid right side
Token* Semant::semantStaticOperator(
	StaticOperator* s,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	bool baseToken)
{
	Identifier* i;
	if ((i = dynamic_cast<Identifier*>(s->right)) == nullptr) {
		Error::logError(ErrorType::ExpectedToFollow, "a member variable", s);
		return s;
	}
	//TODO: currently only Main has support for member variables
	if (s->ownerType != CDataType::mainType) {
		Error::logError(ErrorType::General, "currently only Main has member variable support", s);
		return s;
	}
	if (i->name == "str")
		s->dataType = CDataType::stringType;
	else if (i->name == "print" || i->name == "exit")
		s->dataType = CDataType::voidType;
	else {
		Error::logError(ErrorType::General, "currently Main only has support for print, str, and exit", i);
		return s;
	}
	//TODO: implement classes
	return s;
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
