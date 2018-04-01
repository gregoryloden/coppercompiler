#include "Project.h"

File::File()
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs("FILE"))) {
}
File::~File() {}
IncludedPath::IncludedPath(string fullPath)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("INCLPTH")) COMMA)
splitPath(new Array<string>()) {
	splitPath->add(fullPath);
}
IncludedPath::~IncludedPath() {
	delete splitPath;
}
