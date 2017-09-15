#include "Project.h"

//converts abstract sequences of tokens into structured trees
//provides each source file with a list of global variable definitions and type definitions

//parse all expressions in all files
void ParseExpressions::parseExpressionsInFiles(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		try {
			printf("Parsing expressions for %s...\n", s->filename.c_str());
			parseGlobalDefinitions(s);
			//TODO: parse expressions
		} catch (...) {
		}
	}
}
//return the token if it's not a subsituted token, otherwise return its innermost resulting token
Token* ParseExpressions::getResultingToken(Token* t) {
	SubstitutedToken* s;
	return ((s = dynamic_cast<SubstitutedToken*>(t)) != nullptr) ? s->resultingToken : t;
}
//if the token is a substituted token, replace its resulting token with the new one
void ParseExpressions::replaceResultingToken(SubstitutedToken* parentS, Token* resultingToken) {
	SubstitutedToken* s;
	while ((s = dynamic_cast<SubstitutedToken*>(parentS->resultingToken)) != nullptr)
		parentS = s;
	if (parentS->shouldDelete)
		delete parentS->resultingToken;
	else
		parentS->shouldDelete = true;
	parentS->resultingToken = resultingToken;
}
//parse all the definitions in the source file and store them in it
void ParseExpressions::parseGlobalDefinitions(SourceFile* sf) {
	forEach(Token*, t, sf->abstractContents->tokens, ti) {
		Token* fullToken = t;
		t = getResultingToken(t);
		Identifier* i;
		CType* ct;
		if ((i = dynamic_cast<Identifier*>(t)) != nullptr &&
				(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr)
			completeVariableDefinition(ct, i, &ti);
		//TODO: handle type definitions
		Error::makeError(General, "expected a type or variable definition", fullToken);
	}
	//TODO: ???????????? what do we do with variable definitions?
}
//we parsed a type name, now get a variable initialization
//TODO: ????????????????? save it where?
void ParseExpressions::completeVariableDefinition(CType* type, Identifier* typeToken, ArrayIterator<Token*>* ti) {
	Token* assignment = parseExpression(ti, SeparatorType::Semicolon);
	if (assignment == nullptr)
		Error::makeError(General, "expected a variable assignment to follow", typeToken);
	//TODO: ????????????? what do we do with our completed variable definition?
}
//this is the main loop of parsing
//return the expression that starts at the next iterator position
//fully expects an expression and nothing else- other functions will find things like parameters or keywords
//returns null if there were no tokens remaining in the iterator
Token* ParseExpressions::parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes) {
	Token* activeExpression = nullptr;
	forEachContinued(Token*, t, ti) {
		Token* fullToken = t;
		t = getResultingToken(t);
		Separator* s;
		//before we do anything, see if we're done parsing
		if ((s = dynamic_cast<Separator*>(t)) != nullptr) {
			if ((s->type & endingSeparatorTypes) != 0)
				return activeExpression;
			string errorMessage = "unexpected " + Separator::separatorName(s->type);
			Error::makeError(General, errorMessage.c_str(), fullToken);
		}
		AbstractCodeBlock* a;
		Operator* o;
		//this could be a function call, function parameters, a cast, or a value expression
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			//TODO: ???????????????????? what if it's a cast?
			//TODO: ?????????????????? build function call/definition
		//if we have an operator, it could be prefix, postfix, binary, or ternary, and it might have been lexed wrong
		//whatever it is, figure it out with what we've got
		} else if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
			SubstitutedToken* st;
			//we have to replace the operator
			if ((st = dynamic_cast<SubstitutedToken*>(fullToken)) != nullptr)
				replaceResultingToken(st, o = new Operator(o->type, o->contentPos, o->endContentPos, o->owningFile));
			activeExpression = addToOperator(fullToken, o, activeExpression, ti);
		//we don't have anything yet and nothing came before, we expect a value expression
		} else if (activeExpression == nullptr)
			activeExpression = getValueExpression(t, fullToken, ti);
		else
			Error::makeError(General, "expected an operator or end of expression", fullToken);
	}
	if (activeExpression != nullptr && (endingSeparatorTypes & SeparatorType::RightParenthesis) == 0) {
		if (endingSeparatorTypes == SeparatorType::Semicolon)
			Error::makeError(General, "expected a semicolon to follow", ti->getPrevious());
		else
			Error::makeError(General, "unexpected end of expression", ti->getPrevious());
	}
	return activeExpression;
}
//get a value expression out of the token
//if it's a value on it's own, we're done
//if it's an abstract code block, get a value expression from it
//if it's an operator, make sure it's good to be a prefix operator, then recursively get a value expression
Token* ParseExpressions::getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti) {
	AbstractCodeBlock* a;
	DirectiveTitle* dt;
	Operator* o;
	//try to parse a prefix operator
	if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		addToOperator(fullToken, o, nullptr, ti);
	//parse a parenthesized expression
	else if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
		ArrayIterator<Token*> ai (a->tokens);
		t = parseExpression(&ai, SeparatorType::RightParenthesis);
		if (t == nullptr)
			Error::makeError(General, "expected an expression", a);
		return new ParenthesizedExpression(t, a);
	//a select few directives represent values or contain values like #file or #enable
	} else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr) {
		//TODO: but not yet
	//parse a regular single value
	} else if (dynamic_cast<Identifier*>(t) != nullptr ||
			dynamic_cast<IntConstant*>(t) != nullptr ||
			dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr)
		return fullToken;
	assert(dynamic_cast<ParenthesizedExpression*>(t) == nullptr);
	Error::makeError(General, "expected a value", fullToken);
	return nullptr;
}
//this is the meat of parsing, where retcon parsing happens
//add a token to the right side of this operator, cascade as necessary
Token* ParseExpressions::addToOperator(Token* fullToken, Operator* o, Token* activeExpression, ArrayIterator<Token*>* ti) {
	//if we parsed a subtraction sign but there's no active expression, it's a negative sign
	if (activeExpression == nullptr && o->type == Subtract) {
		o->type = Negate;
		o->precedence = PrecedencePrefix;
	}

	//whatever the operator is, if it's not postfix then we expect a value to follow
	if (o->precedence != PrecedencePostfix) {
		Token* t = ti->getNext();
		if (!ti->hasThis())
			Error::makeError(General, "expected a value to follow", fullToken);
		o->right = getValueExpression(getResultingToken(t), t, ti);
	}

	//we have no active expression- this is ok for prefix operators but nothing else
	if (activeExpression == nullptr) {
		if (o->precedence != PrecedencePrefix)
			Error::makeError(General, "expected a value", fullToken);
		return fullToken;
	}

	//since we have an active expression, check to see if it's an operator with a lower precedence
	//if so, we need to go through and reassign
	//this works with postfix operators, no special processing required
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(getResultingToken(activeExpression))) != nullptr &&
		oNext->precedence < o->precedence)
	{
		Operator* oParent = oNext;
		while ((oNext = dynamic_cast<Operator*>(getResultingToken(oParent->right))) != nullptr &&
				oNext->precedence < o->precedence) //TODO: compare equal precedences
			oParent = oNext;
		o->left = oParent->right;
		oParent->right = fullToken;
		return activeExpression;
	//it's not an operator or it's a higher precedence, we're done
	} else {
		o->left = activeExpression;
		return fullToken;
	}
}

//parse an expression or control flow
//void ParseExpressions::parseStatement





//to test retcon parsing: 3 * 4 + 5 * 6 == 3 * 4 + 5 * 6 && 3 * 4 + 5 * 6 == 3 * 4 + 5 * 6

//Token* addToIdentifier(Identifier* base, Token* next);

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
