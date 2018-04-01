#include "../General/globals.h"
#include "string"
using namespace std;

template <class Type> class Array;

class File onlyInDebug(: public ObjCounter) {
public:
	File();
	virtual ~File();
};
class IncludedPath onlyInDebug(: public ObjCounter) {
public:
	Array<string>* splitPath; //copper: private<readonly Include>

	IncludedPath(string fullPath);
	virtual ~IncludedPath();
};
