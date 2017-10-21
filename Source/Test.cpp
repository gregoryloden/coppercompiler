#include "Project.h"

#ifdef DEBUG
	void Test::testFiles() {
		testFile("Test/Step01_Lex_lex.cu", 0);
		testFile("Test/Step01_Lex_badLex.cu", 10);
	}
	void Test::testFile(const char* fileName, int errorsExpected) {
		Pliers* p = new Pliers(fileName, false, false);
		if (p->errorMessages->length != errorsExpected) {
			forEach(ErrorMessage*, errorMessage, p->errorMessages, ei) {
				errorMessage->printError();
			}
			assert(false);
		}
		delete p;
	}
#endif
