#include "Project.h"

void Semant::semant(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Analyzing semantics for %s...\n", s->filename.c_str());
			semantFile(s);
		} catch (...) {
		}
	}
}
void Semant::semantFile(SourceFile* sourceFile) {

}
