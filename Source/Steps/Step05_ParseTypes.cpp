#include "Project.h"

//registers all class and enum types per source file
//done before other parsing so that copper can discern things like
//	function declaration vs function call or generic type vs lesser/greater comparison

//parse all types in all files
void ParseTypes::parseTypes(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Parsing types for %s...\n", s->path->fileName.c_str());
			//TODO: parse types
		} catch (...) {
		}
	}
}
