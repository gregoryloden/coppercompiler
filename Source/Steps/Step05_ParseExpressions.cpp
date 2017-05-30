#include "Project.h"

//this is the meat of parsing
//add a token to the right side of this operator
//cascade up as necessary
//TODO: improve this comment to say what it really does
bool addToOperator(Operator* base) {
	//find a good token
	//if we get any bad tokens, dump them until we find a good one, erroring on the first one
	while (true) {
		LexToken* next = lex();
		if (next == nullptr)
			makeError(0, "expected an expression to follow", base->contentPos);

	}

	return false;
}