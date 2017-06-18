#include "Project.h"

//only loads and links files, does nothing else
Array<SourceFile*>* Include::loadFiles(char* baseFile) {
	Array<SourceFile*>* files = new Array<SourceFile*>();
	SourceFile* sourceFile = new SourceFile(baseFile);
	ParseDirectives::parseDirectives(sourceFile);
	files->add(sourceFile);
	return files;
}
