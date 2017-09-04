#include "Project.h"

//converts abstract sequences of tokens into structured trees
//provides each source file with a list of global variable definitions and type definitions

//parse all expressions in all files
void ParseExpressions::parseExpressions(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		try {
			printf("Parsing expressions for %s...\n", s->filename.c_str());
			parseGlobalDefinitions(s->abstractContents);
			//TODO: parse expressions
		} catch (...) {
		}
	}
}
void ParseExpressions::parseGlobalDefinitions(AbstractCodeBlock* a) {
	forEach(Token*, t, a->tokens, ti) {
		Token* fullToken = t;
		//if it's a substituted token, find the resulting token and its parent in addition to the full token
		SubstitutedToken* parentS;
		if ((parentS = dynamic_cast<SubstitutedToken*>(t)) != nullptr) {
			SubstitutedToken* s2 = parentS;
			while ((s2 = dynamic_cast<SubstitutedToken*>(parentS->resultingToken)) != nullptr)
				parentS = s2;
			t = parentS->resultingToken;
			//if we can't delete it then we can't mutate it either- clone or replace it if we need to
			if (!parentS->shouldDelete) {
				Token* oldResultingToken = parentS->resultingToken;
				Operator* o;
				if ((o = dynamic_cast<Operator*>(oldResultingToken)) != nullptr)
					parentS->resultingToken = new Operator(o->type, o->contentPos, o->endContentPos, o->owningFile);
				if (oldResultingToken != parentS->resultingToken)
					parentS->shouldDelete = true;
			}
		}
		//????????????????????
	}
}








//to test retcon parsing: 3 * 4 + 5 * 6 == 3 * 4 + 5 * 6 && 3 * 4 + 5 * 6 == 3 * 4 + 5 * 6

//Token* addToIdentifier(Identifier* base, Token* next);

//this is the meat of parsing
//add a token to the right side of this operator
//cascade up as necessary
//TODO: improve this comment to say what it really does
bool addToOperator(Operator* base) {
	//find a good token
	//if we get any bad tokens, dump them until we find a good one, erroring on the first one
	while (true) {
		//something

	}

	return false;
}

	////every global definition starts with an identifier or a directive
	//if (dynamic_cast<Identifier*>(active) == nullptr /*&& dynamic_cast<CDirective*>(active) == nullptr*/)
	//	makeError(0, "expected an identifier or directive declaration", active->contentPos);

	//while (true) {
	//	LexToken* next = lex();
	//	if (next == nullptr) {
	//		tokens->add(active);
	//		return tokens;
	//	}

	//	//find out what we have
	//	Token* result;
	//	Identifier* ai;
	//	if (((ai = dynamic_cast<Identifier*>(active)) != nullptr && (result = addToIdentifier(ai, next)) != nullptr) ||
	//			true) {

	//	}
	//}

//try to add a token to an identifier
//possible outcomes:
//	identifier + identifier -> identifier list
//	identifier + left parenthesis ->
//		identifier + fdsafsadfsfdsafdsfdsafasd
//Token* addToIdentifier(Identifier* base, LexToken* next) {
//	Identifier* ni;
//	Operator* oi;
//	if ((ni = dynamic_cast<Identifier*>(next)) != nullptr)
//		return new IdentifierList(base, ni);
//	else if ((oi = dynamic_cast<Operator*>(next)) != nullptr) {
//		oi->left = base;
//		return addToOperator(oi) ? oi : nullptr;
//	}
//	return next;
//}
