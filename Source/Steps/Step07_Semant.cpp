#include "Project.h"

//verify that all types are correct, variable names match, etc.

void Semant::semant(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Analyzing semantics for %s...\n", s->filename.c_str());
			semantFile(s);
		} catch (...) {
		}
	}
}
void Semant::semantFile(SourceFile* sourceFile) {
	forEach(Token*, t, sourceFile->globalVariables, ti) {

	}
}

//TODO:
//-make sure variable initializations are either parenthesized or top-level in statements
//-make sure variable initializations in parentheses have values
//-make sure assignment operators are either parenthesized or top-level in statements
//-make sure all global variables are variable definition lists or assignments to variable definition lists
//-make sure variable initializations match the initial values (groups with all the types, or one value matching all the types)
/*
				//in an expression, declared variables must be initialized
				if (variableInitialization->initialization == nullptr) {
					Deleter<VariableInitialization> variableInitializationDeleter (variableInitialization);
					Error::makeError(ErrorType::ExpectedToFollow, "a variable initialization", variableInitialization);
				}
*/
