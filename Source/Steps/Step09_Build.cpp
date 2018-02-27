#include "Project.h"

//build the final executable file
//TODO: finalize byte sizes for primitive types
//TODO: uninitialized variables

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
