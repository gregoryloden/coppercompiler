#include "Project.h"

//Token* addToIdentifier(Identifier* base, Token* next);

//get the list of global definitions
Array<Token*>* parseDirectives(bool endsWithParenthesis) {
	Array<Token*>* tokens = new Array<Token*>();
	Token* active = lex();
	//an empty file is totally valid
	if (active == nullptr)
		return tokens;

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
	return nullptr;
}
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
