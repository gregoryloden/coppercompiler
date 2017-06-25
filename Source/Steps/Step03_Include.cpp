#include "Project.h"

//only loads and links files, does nothing else
Array<SourceFile*>* Include::loadFiles(char* baseFile) {
Trie<char, SourceFile*>* filesByName = new Trie<char, SourceFile*>();
if (filesByName == nullptr)
return nullptr;

Array<SourceFile*>* files = new Array<SourceFile*>();
SourceFile* sourceFile = new SourceFile(baseFile);
ParseDirectives::parseDirectives(sourceFile);
files->add(sourceFile);
return files;
}
