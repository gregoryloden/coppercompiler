#include "../General/globals.h"

template <class Type> class Array;

class Path onlyInDebug(: public ObjCounter) {
public:
	static string currentWorkingDirectory;

	string fileName; //copper: readonly
	bool isDirectory; //copper: readonly
	Path* parentDirectory; //copper: readonly

	Path(string pFileName, bool pIsDirectory, Path* pParentDirectory);
	virtual ~Path();

	void deleteFullPath();
	static Path* createPath(string fullPath);
	static Path* createPathWithParentDirectory(string relativePath, Path* pParentDirectory);
	Path* clone();
	string getFullPathName();
private:
	void appendToPathName(string* path);
};
