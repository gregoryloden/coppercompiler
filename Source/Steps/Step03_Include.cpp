#include "Project.h"

//this will be a closure in copper
string nextFilename = "";
SourceFile* generateSourceFile() {
	return new SourceFile(nextFilename);
}
//only loads and links files, does nothing else
Array<SourceFile*>* Include::loadFiles(char* baseFile) {
	Trie<char, SourceFile*>* filesByName = new Trie<char, SourceFile*>();
	Array<SourceFile*>* allFiles = new Array<SourceFile*>();
	allFiles->add(new SourceFile(baseFile));
	nextFilename = allFiles->inner[0]->filename;
	filesByName->getOrSet(nextFilename.c_str(), nextFilename.length(), generateSourceFile);
	for (int i = 0; i < allFiles->length; i++) {
		SourceFile* nextFile = allFiles->inner[i];
		//TODO: store by the full file path instead of just the filename string
		ParseDirectives::parseDirectives(nextFile);

		//TODO: add included files
		//TODO: file paths should be relative to the file, instead of just relative to the working directory
		//TODO: wildcard includes
	}
	delete filesByName;
	return allFiles;
}
