#include "Project.h"
#ifdef WIN32
	#include <Windows.h>
	#undef min
	#undef max
#endif

//loads and links files, does nothing else

thread_local Pliers* Include::currentPliers = nullptr;
thread_local PrefixTrie<char, SourceFile*>* Include::filesByName = nullptr;

//load the file indicated by the given filename, and the files of the file graph it includes
void Include::loadFiles(Pliers* pliers) {
	currentPliers = pliers;
	filesByName = new PrefixTrie<char, SourceFile*>();
	Path* basePath = Path::createPath(Path::currentWorkingDirectory + "/" + pliers->baseFileName);
	getSourceFile(basePath, nullptr, false);
	for (int filei = 0; filei < pliers->allFiles->length; filei++) {
		SourceFile* nextFile = pliers->allFiles->get(filei);
		ParseDirectives::parseDirectives(nextFile);
		if (nextFile->abstractContents->directives == nullptr)
			continue;

		forEach(CDirective*, d, nextFile->abstractContents->directives, di) {
			CDirectiveInclude* i;
			if (!let(CDirectiveInclude*, i, d))
				continue;

			resolveIncludedFiles(nextFile, nextFile->path->parentDirectory, i->path, i->pathName, false);
		}
		//and now notify any of our own inclusion listeners that we included files
		forEach(SourceFile*, listener, nextFile->inclusionListeners, sfi)
			listener->includedFiles->setAllFrom(nextFile->includedFiles);
	}
	basePath->deleteFullPath();
	delete filesByName;
}
//using the included path, find all files included, include them, and add this file to their inslusion listeners
void Include::resolveIncludedFiles(
	SourceFile* file, Path* currentPath, Path* path, StringLiteral* inclusionSource, bool wasWildcard)
{
	//a null path means we finished our search- make or get a file from it and return
	if (path == nullptr) {
		SourceFile* includedFile = getSourceFile(currentPath, inclusionSource, wasWildcard);
		if (includedFile == nullptr)
			return;
		//include the file, include all its files, and add this file as an inclusion listener
		file->includedFiles->set(includedFile, true);
		file->includedFiles->setAllFrom(includedFile->includedFiles);
		includedFile->inclusionListeners->add(file);
		return;
	}
	//continue our search
	//regular filename: build a path and continue
	if (path->fileName.find('*') == string::npos) {
		if (path->fileName == ".")
			resolveIncludedFiles(file, currentPath, path->parentDirectory, inclusionSource, wasWildcard);
		else if (path->fileName == "..") {
			if (currentPath->parentDirectory != nullptr)
				resolveIncludedFiles(file, currentPath->parentDirectory, path->parentDirectory, inclusionSource, wasWildcard);
			else {
				Error::logError(ErrorType::General, "cannot get the parent of the root directory", inclusionSource);
				return;
			}
		} else {
			Path nextPath(path->fileName, false, currentPath);
			resolveIncludedFiles(file, &nextPath, path->parentDirectory, inclusionSource, wasWildcard);
		}
	//we have some kind of wildcard, we have to check the files in the current path to see what matches
	} else {
		Path* targetNextPath = path;
		//recursive wildcard directory
		//use the child directory in a wildcard search, whether it's got a wildcard character or not
		if (path->fileName == "**") {
			if (path->parentDirectory == nullptr) {
				Error::logError(
					ErrorType::General, "an included path cannot end in a recursive wildcard directory", inclusionSource);
				return;
			} else if (path->parentDirectory->fileName == "**") {
				Error::logError(
					ErrorType::General,
					"a recursive wildcard directory cannot be followed by another recursive wildcard directory",
					inclusionSource);
				return;
			}
			targetNextPath = path->parentDirectory;
		}
		Array<string>* wildcardMatchSubstrings = StringUtils::split(targetNextPath->fileName, '*');
		//get the contents of the folder
		Array<Path*> folderContents;
		string currentPathString = currentPath->getFullPathName();
		#ifdef WIN32
			currentPathString += "/*";
			WIN32_FIND_DATA fileData;
			HANDLE findFileHandle = FindFirstFile(currentPathString.c_str(), &fileData);
			if (findFileHandle != INVALID_HANDLE_VALUE) {
				do {
					if (strcmp(fileData.cFileName, ".") != 0 && strcmp(fileData.cFileName, "..") != 0)
						folderContents.add(new Path(
							fileData.cFileName, (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, currentPath));
				} while (FindNextFile(findFileHandle, &fileData) != 0);
			}
		#else
			Error::logError(
				ErrorType::General, "wildcard paths are only supported on the Windows compiler", inclusionSource->pathName);
			return;
		#endif
		//check for files that match the wildcard substrings
		//if it's the child of a recursive directory wildcard and it isn't a wildcard, we'll pretend it is
		//if any files match, continue the include with them
		bool shouldWildcardsMatchDirectories = (targetNextPath->parentDirectory != nullptr);
		forEach(Path*, nextPath, &folderContents, fi) {
			//if the name matches (and its directory-ness matches our target next path),
			//	we want to continue with the rest of the path
			if (nextPath->isDirectory == shouldWildcardsMatchDirectories &&
					StringUtils::stringMatchesWildcard(nextPath->fileName, wildcardMatchSubstrings))
				resolveIncludedFiles(file, nextPath, targetNextPath->parentDirectory, inclusionSource, true);
			//for recursive directory searches, we also want to continue the search whether it matches or not
			if (targetNextPath != path && nextPath->isDirectory)
				resolveIncludedFiles(file, nextPath, path, inclusionSource, true);
		}
		folderContents.deleteContents();
		delete wildcardMatchSubstrings;
	}
}
//if the path points to a directory or a missing file, log an error, delete the path, skip the file, and return nullptr
//if it's valid, create a new SourceFile, add it to the two collections, and return it
SourceFile* Include::getSourceFile(Path* path, StringLiteral* inclusionSource, bool wasWildcard) {
	string fileName = path->getFullPathName();
	//if we already have the file, return it
	SourceFile* oldFile = filesByName->get(fileName.c_str(), fileName.length());
	if (oldFile != nullptr)
		return oldFile;
	//read the file
	FILE* file = nullptr;
	fopen_s(&file, fileName.c_str(), "rb");
	//if we have an invalid wildcard file, don't error and don't make a placeholder file, just return nullptr
	//wildcard includes can point to 0+ files, even if they match directories
	if (file == nullptr && wasWildcard)
		return nullptr;
	//now make the file itself, whether it exists or not- if not we will use it to produce an error message
	SourceFile* newFile = new SourceFile(path->clone(), currentPliers);
	currentPliers->allFiles->add(newFile);
	//if it doesn't exist, log an error and return
	//the base file doesn't have an inclusion source so just pretend like it's an empty file
	if (file == nullptr) {
		string errorMessage = "Unable to open file \"" + path->fileName + "\"";
		if (inclusionSource != nullptr)
			Error::logError(ErrorType::General, errorMessage.c_str(), inclusionSource);
		else {
			EmptyToken errorToken (0, newFile);
			Error::logError(ErrorType::General, errorMessage.c_str(), &errorToken);
		}
		return nullptr;
	}
	//if we get here, we have a valid, readable file
	newFile->loadFile(file);
	filesByName->set(fileName.c_str(), fileName.length(), newFile);
	//include itself so that we have one place to look for all directives
	newFile->includedFiles->set(newFile, true);
	return newFile;
}
