#include "Project.h"

//evaluates #if, #replace and #replace-input, #line

void Replace::replaceCode(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		printf("Replacing code in %s...\n", s->filename.c_str());


	}
}
