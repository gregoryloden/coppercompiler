#include "Project.h"

//loads and links files, does nothing else

thread_local Array<SourceFile*>* Include::allFiles = nullptr;
thread_local PrefixTrie<char, SourceFile*>* Include::filesByName = nullptr;

//load the file indicated by the given filename, and the files of the file graph it includes
Array<SourceFile*>* Include::loadFiles(char* baseFileName) {
	allFiles = new Array<SourceFile*>();
	filesByName = new PrefixTrie<char, SourceFile*>();
	SourceFile* baseFile = newSourceFile(baseFileName);
	for (int filei = 0; filei < allFiles->length; filei++) {
		SourceFile* nextFile = allFiles->inner[filei];
		ParseDirectives::parseDirectives(nextFile);
		forEach(CDirective*, d, nextFile->abstractContents->directives, di) {
			CDirectiveInclude* i;
			if ((i = dynamic_cast<CDirectiveInclude*>(d)) == nullptr)
				continue;

			//get the included file
			string includedName = i->filename;
			SourceFile* includedFile = filesByName->get(includedName.c_str(), includedName.length());
			if (includedFile == PrefixTrie<char, SourceFile*>::emptyValue)
				includedFile = newSourceFile(includedName.c_str());
			nextFile->includedFiles->set(includedFile, true);
			//if we're including all, add all its included files and add this to its listeners list
			if (i->includeAll) {
				nextFile->includedFiles->setAllFrom(includedFile->includedFiles);
				includedFile->inclusionListeners->add(nextFile);
			}
		}
		//and now notify any of our own inclusion listeners that we included files
		forEach(SourceFile*, listener, nextFile->inclusionListeners, sfi)
			listener->includedFiles->setAllFrom(nextFile->includedFiles);
		//TODO: file paths should be relative to the file, instead of just relative to the working directory
		//TODO: wildcard includes
	}
	delete filesByName;
	return allFiles;
}
//create a new SourceFile, add it to the two collections, and return it
SourceFile* Include::newSourceFile(const char* fileName) {
	//TODO: store by the full file path instead of just the filename string
	SourceFile* file = new SourceFile(fileName);
	allFiles->add(file);
	filesByName->set(fileName, file->filename.length(), file);
	//include itself so that we have one place to look for all directives
	file->includedFiles->set(file, true);
	return file;
}
