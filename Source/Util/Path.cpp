#include "Project.h"

string Path::currentWorkingDirectory;
Path::Path(string pFileName, bool pIsDirectory, Path* pParentDirectory)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("PATH")) COMMA)
fileName(pFileName)
, isDirectory(pIsDirectory)
, parentDirectory(pParentDirectory) {
}
Path::~Path() {
	//don't delete the parent directory- the usage in stacks means we don't know if any other file has this pointer
}
//delete this path but stop if we hit the given parent
void Path::deleteFullPath() {
	if (parentDirectory != nullptr)
		parentDirectory->deleteFullPath();
	delete this;
}
//split the path into its directory components
Path* Path::createPath(string fullPath) {
	return createPathWithParentDirectory(fullPath, nullptr);
}
//split the path into its directory components
Path* Path::createPathWithParentDirectory(string relativePath, Path* pParentDirectory) {
	const char* pathCStr = relativePath.c_str();
	int pathLength = relativePath.length();
	int nextPathNameStartIndex = 0;
	for (int i = 0; i < pathLength; i++) {
		char c;
		if ((c = pathCStr[i]) == '/' || c == '\\') {
			pParentDirectory =
				new Path(string(pathCStr + nextPathNameStartIndex, i - nextPathNameStartIndex), true, pParentDirectory);
			nextPathNameStartIndex = i + 1;
		}
	}
	return new Path(string(pathCStr + nextPathNameStartIndex, pathLength - nextPathNameStartIndex), false, pParentDirectory);
}
//clone this path
Path* Path::clone() {
	return new Path(fileName, isDirectory, parentDirectory == nullptr ? nullptr : parentDirectory->clone());
}
//get the full relative path that this file represents
string Path::getFullPathName() {
	string path;
	appendToPathName(&path);
	return path;
}
//add to the given path the full relative path that this file represents
void Path::appendToPathName(string* path) {
	if (parentDirectory != nullptr) {
		parentDirectory->appendToPathName(path);
		(*path) += '/';
	}
	(*path) += fileName;
}
