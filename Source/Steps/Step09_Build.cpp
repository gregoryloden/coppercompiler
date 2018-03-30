#include "Project.h"

//build the final executable file
//TODO: finalize byte sizes for primitive types
//TODO: circular definitions
//TODO: unitialized variables (all uninitialized variable errors will be handled here)

//variable data: global variable data stored in tree
//each scope has a PrefixTrieUnion with the data
//when adding variable data:
//use the PrefixTrie check to see if it's got one already
//if not: make a new one, and if the parent has one (getFromParent), port the data from that one to the new one

//build the final executable file
void Build::build(Pliers* pliers) {
	if (pliers->printProgress)
		puts("Building executable...");
	//TODO: build
};

/*
	if (!CVariableData::variableDataContains(variableData, i->name, CVariableData::isInitialized)) {
		string errorMessage = "\"" + i->name + "\" may not have been initialized";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
*/
