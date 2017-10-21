#include "Project.h"

#ifdef DEBUG
	void Test::testFiles() {
		testFile("Test/Step01_Lex_lex.cu", 0);
		testFile("Test/Step01_Lex_badLex.cu", 10);
		testFile("Test/Step01_Lex_blockCommentEOF.cu", 1);
		testFile("Test/Step01_Lex_intBaseEOF.cu", 1);
		testFile("Test/Step01_Lex_floatEOF.cu", 1);
		testFile("Test/Step01_Lex_stringEOF.cu", 1);
		testFile("Test/Step01_Lex_stringEscapeSequenceEOF.cu", 1);
		testFile("Test/Step01_Lex_stringHexEscapeSequenceEOF.cu", 1);
		testFile("Test/Step01_Lex_characterEOF.cu", 1);
		testFile("Test/Step01_Lex_characterUnterminatedEOF.cu", 1);
		testFile("Test/Step01_Lex_directiveEOF.cu", 1);
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
