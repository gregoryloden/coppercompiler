#include "../General/globals.h"

class File onlyInDebug(: public ObjCounter) {
public:
	File();
	virtual ~File();
};
