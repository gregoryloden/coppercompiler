#include "Project.h"

//registers all class and enum types per source file
//done before other parsing so that copper can discern things like
//	function declaration vs function call or generic type vs lesser/greater comparison

//parse all types in all files
void ParseTypes::parseTypes(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		try {
			//TODO: parse types
		} catch (...) {
		}
	}
}
