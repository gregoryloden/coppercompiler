#include "Project.h"

//verify that all types are correct, variable names match, etc.

//validate the semantics for all files
void Semant::semant(Pliers* pliers) {
	//start by gathering the trie of all global variables
	if (pliers->printProgress)
		printf("Gathering all global variables...\n");
	PrefixTrie<char, CVariableDefinition*> allGlobalVariables;
	forEach(SourceFile*, s, pliers->allFiles, si1) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			VariableDefinitionList* v;
			if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
			else if ((o = dynamic_cast<Operator*>(t)) != nullptr &&
					(v = dynamic_cast<VariableDefinitionList*>(o->left)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
		}
	}

	//then go through all the files
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		if (pliers->printProgress)
			printf("Analyzing semantics for %s...\n", s->filename.c_str());
		semantFile(s, &allGlobalVariables);
	}
}
//add variables from the definition list into the trie
void Semant::addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables) {
	forEach(CVariableDefinition*, c, v->variables, ci) {
		CVariableDefinition* old = variables->set(c->name->name.c_str(), c->name->name.length(), c);
		if (old != nullptr) {
			variables->set(old->name->name.c_str(), old->name->name.length(), old);
			string errorMessage = "\"" + old->name->name + "\" has already been defined";
			Error::makeError(ErrorType::General, errorMessage.c_str(), c->name);
		}
	}
}
//validate the semantics in the given file
void Semant::semantFile(SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables) {
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o = nullptr;
		VariableDefinitionList* v = dynamic_cast<VariableDefinitionList*>(t);
		if (v == nullptr) {
			if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
				Error::logError(ErrorType::Expected, "a variable initalization", t);
				continue;
			} else if ((v = dynamic_cast<VariableDefinitionList*>(o->left)) == nullptr) {
				Error::logError(ErrorType::Expected, "a variable declaration list", t);
				continue;
			}
		}
		if (o != nullptr)
			semantToken(o, variables);
	}
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything it relies on is also valid
//checks types
void Semant::semantToken(Token* t, PrefixTrie<char, CVariableDefinition*>* variables) {

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
