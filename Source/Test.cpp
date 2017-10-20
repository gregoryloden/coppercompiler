#include "Project.h"

#ifdef DEBUG
	void Test::testFiles() {
		testFile("Test/test0001_lex.cu", 0);
		testFile("Test/test0002_badLex.cu", 10);
	}
	void Test::testFile(const char* fileName, int errorsExpected) {
		Pliers* p = new Pliers(fileName, false onlyInDebug(COMMA false));
		assert(p->errorMessages->length == errorsExpected);
		delete p;
	}
#endif
