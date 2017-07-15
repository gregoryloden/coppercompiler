#include "Project.h"

//evaluates #if, #replace and #replace-input, #line

void Replace::replaceCode(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		printf("Replacing code in %s...\n", s->filename.c_str());

		Array<CDirectiveReplace*> replaces;
		Array<SourceFile*>* allIncluded = s->includedFiles->keys();
		forEach(SourceFile*, included, allIncluded, includedi) {
			forEach(CDirective*, d, included->abstractContents->directives, di) {
				CDirectiveReplace* r;
				if ((r = dynamic_cast<CDirectiveReplace*>(d)) != nullptr)
					replaces.add(r);
			}
		}
		delete allIncluded;
	}
}
