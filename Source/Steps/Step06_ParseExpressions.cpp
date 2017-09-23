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
//parse all the definitions in the source file and store them in it
void ParseExpressions::parseGlobalDefinitions(SourceFile* sf) {
	forEach(Token*, t, sf->abstractContents->tokens, ti) {
		try {
			Token* fullToken = t;
			ti.replaceThis(nullptr);
			Deleter<Token> fullTokenDeleter (fullToken);
			t = Token::getResultingToken(t);
			Identifier* i;
			CType* ct;
			if ((i = dynamic_cast<Identifier*>(t)) != nullptr &&
					(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr)
				completeVariableDefinition(ct, fullToken, &ti);
			else
				//TODO: handle type definitions
				Error::makeError(ErrorType::General, "expected a type or variable definition", fullToken);
		} catch (...) {
		}
	}
	//TODO: ???????????? what do we do with variable definitions?
}
//we parsed a type name, now get a variable initialization
//TODO: ????????????????? save it where?
//may throw
void ParseExpressions::completeVariableDefinition(CType* type, Token* typeToken, ArrayIterator<Token*>* ti) {
	Token* assignment =
		parseExpression(ti, (unsigned char)SeparatorType::Semicolon, "expected a variable assignment to follow", typeToken);
	//TODO: ????????????? what do we do with our completed variable definition?
}
//this is the main loop of parsing
//return the expression that starts at the next iterator position
//fully expects an expression and nothing else- other functions will find things like parameters or keywords
//may throw
Token* ParseExpressions::parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
	char* emptyExpressionErrorMessage, Token* emptyExpressionErrorToken)
{
	Token* activeExpression = nullptr;
	try {
		forEachContinued(Token*, t, ti) {
			Token* fullToken = t;
			ti->replaceThis(nullptr);
			Deleter<Token> fullTokenDeleter (fullToken);
			t = Token::getResultingToken(t);
			Separator* s;
			//before we do anything, see if we're done parsing
			if ((s = dynamic_cast<Separator*>(t)) != nullptr) {
				if (((unsigned char)(s->type) & endingSeparatorTypes) != 0)
					return activeExpression;
				string errorMessage = "unexpected " + Separator::separatorName(s->type);
				Error::makeError(ErrorType::General, errorMessage.c_str(), fullToken);
			}
			AbstractCodeBlock* a;
			Operator* o;
			if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
				activeExpression = evaluateAbstractCodeBlock(a, fullToken, activeExpression, ti);
			else if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
				SubstitutedToken* st;
				//replace the operator if it's substituted
				if ((st = dynamic_cast<SubstitutedToken*>(fullToken)) != nullptr)
					st->replaceResultingToken(o = new Operator(o->type, o->contentPos, o->endContentPos, o->owningFile));
				activeExpression = addToOperator(o, fullToken, activeExpression, ti);
			//we don't have anything yet so we expect a value expression
			} else if (activeExpression == nullptr)
				activeExpression = getValueExpression(t, fullToken, ti);
			else
				Error::makeError(ErrorType::General, "expected an operator or end of expression", fullToken);
			fullTokenDeleter.release();
		}

		if (activeExpression == nullptr)
			Error::makeError(ErrorType::General, emptyExpressionErrorMessage, emptyExpressionErrorToken);
		else if ((endingSeparatorTypes & (unsigned char)SeparatorType::RightParenthesis) == 0) {
			//find the last token to use as the error token
			Token* lastToken = activeExpression;
			ParenthesizedExpression* p = nullptr;
			while (true) {
				Operator* o;
				Token* resultingLastToken = Token::getResultingToken(lastToken);
				if ((o = dynamic_cast<Operator*>(resultingLastToken)) != nullptr && o->right != nullptr)
					lastToken = o->right;
				else {
					p = dynamic_cast<ParenthesizedExpression*>(resultingLastToken);
					break;
				}
			}
			//if it was a parenthesized expression, use the end content pos instead of the regular content pos
			EmptyToken errorToken (p == nullptr ? lastToken->contentPos : p->endContentPos, lastToken->owningFile);
			if (endingSeparatorTypes == (unsigned char)SeparatorType::Semicolon)
				Error::makeError(ErrorType::General, "expected a semicolon to follow", &errorToken);
			else
				Error::makeError(ErrorType::General, "unexpected end of expression", &errorToken);
		}
	} catch (...) {
		delete activeExpression;
		throw;
	}
	return activeExpression;
}
//get a value expression that starts at the given token
//if it's a value on it's own, we're done
//if it's an abstract code block, get a value expression from it
//if it's an operator, make sure it's a prefix operator, then recursively get a value expression
//may throw
Token* ParseExpressions::getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti) {
	AbstractCodeBlock* a;
	DirectiveTitle* dt;
	Operator* o;
	//try to parse a prefix operator
	if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		addToOperator(o, fullToken, nullptr, ti);
	//parse a parenthesized expression
	else if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
		ArrayIterator<Token*> ai (a->tokens);
		ParenthesizedExpression* value = new ParenthesizedExpression(
			parseExpression(&ai, (unsigned char)SeparatorType::RightParenthesis, "expected an expression", a), a);
		assert(fullToken == a);
		delete fullToken; //any deleters in calling functions will not delete this so we have to delete it here
		return value;
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
	Error::makeError(ErrorType::General, "expected a value", fullToken);
	return nullptr;
}
//this is the meat of parsing, where retcon parsing happens
//add a token to the right side of this operator, cascade as necessary
//it could be prefix, postfix, binary, or ternary, and it might have been lexed wrong
//whatever it is, figure it out with what we've got
//may throw
Token* ParseExpressions::addToOperator(Operator* o, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti) {
	//if we parsed a subtraction sign but there's no active expression, it's a negative sign
	if (activeExpression == nullptr && o->type == OperatorType::Subtract) {
		o->type = OperatorType::Negate;
		o->precedence = OperatorTypePrecedence::Prefix;
	}

	//whatever the operator is, if it's not postfix then we expect a value to follow
	if (o->precedence != OperatorTypePrecedence::Postfix) {
		Token* t = ti->getNext();
		if (!ti->hasThis())
			Error::makeError(ErrorType::General, "expected a value to follow", fullToken);
		ti->replaceThis(nullptr);
		Deleter<Token> tDeleter (t);
		o->right = getValueExpression(Token::getResultingToken(t), t, ti);
		tDeleter.release();
	}

	//we have no active expression- this is ok for prefix operators but nothing else
	if (activeExpression == nullptr) {
		if (o->precedence != OperatorTypePrecedence::Prefix)
			Error::makeError(ErrorType::General, "expected a value", fullToken);
		//since we already got the following value we can just return the token
		return fullToken;
	}

	//since we have an active expression, check to see if it's an operator with a lower precedence
	//if so, we need to go through and reassign
	//this works with postfix operators, no special processing required
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(Token::getResultingToken(activeExpression))) != nullptr &&
		oNext->precedence < o->precedence)
	{
		Operator* oParent = oNext;
		while ((oNext = dynamic_cast<Operator*>(Token::getResultingToken(oParent->right))) != nullptr &&
				o->takesRightHandPrecedence(oNext))
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
//complete whatever expression we have using the abstract code block
//this could be a function call, function parameters, a cast, or a value expression, but not a statement list
//may throw
Token* ParseExpressions::evaluateAbstractCodeBlock(
	AbstractCodeBlock* a, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti)
{
	//since we have no active expression, this could be either a value or a cast
	if (activeExpression == nullptr) {
		if (a->tokens->length == 0)
			Error::makeError(ErrorType::General, "expected a value", fullToken);
		//TODO: ???????????????????? what if it's a cast? or a value type?
	}
	//TODO: ?????????????????? build function call/definition
	return nullptr;
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
