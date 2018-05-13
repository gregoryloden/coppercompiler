#include "Project.h"

//perform any pre-build optimizations

//perform optimizations in all files
void Optimize::optimize(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Optimizing %s...\n", s->path->fileName.c_str());
			//TODO: optimize
		} catch (...) {
		}
	}
};
