#include "Project.h"

//converts abstract sequences of tokens into structured trees
//provides each source file with a list of global variable definitions and type definitions

//parse all expressions in all files
void ParseExpressions::parseExpressionsInFiles(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		try {
			printf("Parsing expressions for %s...\n", s->filename.c_str());
			parseGlobalDefinitions(s);
		} catch (...) {
		}
	}
}
//parse all the definitions in the source file and store them in it
//parse location: EOF
void ParseExpressions::parseGlobalDefinitions(SourceFile* sf) {
	forEach(Token*, fullToken, sf->abstractContents->tokens, ti) {
		try {
			ti.replaceThis(nullptr);
			Deleter<Token> fullTokenDeleter (fullToken);
			Token* t = Token::getResultingToken(fullToken);
			Identifier* i;
			CType* ct;
			DirectiveTitle* dt;
			if ((i = dynamic_cast<Identifier*>(t)) != nullptr &&
					(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr)
				sf->globalDefinitions->add(completeVariableDefinition(ct, fullToken, &ti));
			else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
				continue;
			else
				//TODO: handle type definitions
				Error::makeError(ErrorType::General, "expected a type or variable definition", fullToken);
		} catch (...) {
		}
	}
	//TODO: ???????????? what do we do with variable definitions?
}
//we parsed a type name, now get a variable initialization
//parse location: the semicolon after the variable definition
//may throw
CVariableDefinition* ParseExpressions::completeVariableDefinition(CType* type, Token* fullToken, ArrayIterator<Token*>* ti) {
	ti->getNext();
	Token* definition =
		parseExpression(ti, (unsigned char)SeparatorType::Semicolon, "expected a variable assignment to follow", fullToken);
	Token* t = Token::getResultingToken(definition);
	Identifier* i;
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
		CVariableDefinition* result = new CVariableDefinition(type, i->name, nullptr);
		delete i;
		return result;
	}
	Operator* o;
	if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign)
		Error::makeError(ErrorType::General, "invalid variable initialization", definition);
	if ((i = dynamic_cast<Identifier*>(Token::getResultingToken(o->left))) == nullptr)
		Error::makeError(ErrorType::General, "expected a variable name", o->left);
	return new CVariableDefinition(type, i->name, definition);
}
//this is the main loop of parsing
//return the expression that starts at the current iterator position
//fully expects a non-empty expression and nothing else- other functions will find things like parameters or keywords
//parse location: one of the ending separator types | the end of the abstract code block (EOF for the global one)
//may throw
Token* ParseExpressions::parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
	char* emptyExpressionErrorMessage, Token* emptyExpressionErrorToken)
{
	Token* activeExpression = nullptr;
	try {
		forEachContinued(Token*, fullToken, ti) {
			ti->replaceThis(nullptr);
			Deleter<Token> fullTokenDeleter (fullToken);
			Token* t = Token::getResultingToken(fullToken);
			Separator* s;
			//before we do anything, see if we're done parsing
			if ((s = dynamic_cast<Separator*>(t)) != nullptr) {
				if (activeExpression != nullptr && (endingSeparatorTypes & (unsigned char)(s->separatorType)) != 0)
					return activeExpression;
				string errorMessage = "unexpected " + Separator::separatorName(s->separatorType);
				Error::makeError(ErrorType::General, errorMessage.c_str(), fullToken);
			}
			AbstractCodeBlock* a;
			Operator* o;
			DirectiveTitle* dt;
			if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
				activeExpression = evaluateAbstractCodeBlock(a, fullToken, activeExpression, ti);
				continue; //the tokens have been taken so let it get deleted
			} else if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
				SubstitutedToken* st;
				//replace the operator if it's substituted
				if ((st = dynamic_cast<SubstitutedToken*>(fullToken)) != nullptr)
					st->replaceResultingToken(
						o = new Operator(o->operatorType, o->contentPos, o->endContentPos, o->owningFile));
				activeExpression = addToOperator(o, fullToken, activeExpression, ti);
			} else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
				continue; //let the directive title get deleted
			//we don't have anything yet so we expect a value expression
			else if (activeExpression == nullptr)
				activeExpression = getValueExpression(t, fullToken, ti);
			else
				Error::makeError(ErrorType::General, "expected an operator or end of expression", fullToken);
			fullTokenDeleter.release();
		}

		if (activeExpression == nullptr) {
			assert(emptyExpressionErrorMessage != nullptr && emptyExpressionErrorToken != nullptr);
			Error::makeError(ErrorType::General, emptyExpressionErrorMessage, emptyExpressionErrorToken);
		} else if ((endingSeparatorTypes & (unsigned char)SeparatorType::RightParenthesis) == 0) {
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
//if it's an operator, recursively get a value expression if it's a prefix operator
//if it's an abstract code block, evaluate the value it represents
//TODO: if it's a directive title, see if it represents a value and get it if so
//if it's a value on it's own, we're done
//parse location: the location of the given token | the next value token after the location of the given operator
//may throw
Token* ParseExpressions::getValueExpression(Token* t, Token* fullToken, ArrayIterator<Token*>* ti) {
	Operator* o;
	AbstractCodeBlock* a;
	DirectiveTitle* dt;
	//try to parse a prefix operator
	if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		addToOperator(o, fullToken, nullptr, ti);
	//parse a parenthesized expression or a cast
	else if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
		return evaluateAbstractCodeBlock(a, fullToken, nullptr, ti);
	//a select few directives represent values or contain values like #file or #enable
	else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr) {
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
//parse location: the next value token after this operator
//may throw
Token* ParseExpressions::addToOperator(Operator* o, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti) {
	//if we parsed a subtraction sign but there's no active expression, it's a negative sign
	if (activeExpression == nullptr && o->operatorType == OperatorType::Subtract) {
		o->operatorType = OperatorType::Negate;
		o->precedence = OperatorTypePrecedence::Prefix;
	}

	//whatever the operator is, if it's not postfix then we expect a value to follow
	if (o->precedence != OperatorTypePrecedence::Postfix) {
		Token* nextFullToken = ti->getNext();
		if (!ti->hasThis())
			Error::makeError(ErrorType::General, "expected a value to follow", fullToken);
		ti->replaceThis(nullptr);
		Deleter<Token> nextFullTokenDeleter (nextFullToken);
		Token* t = Token::getResultingToken(nextFullToken);
		o->right = getValueExpression(t, nextFullToken, ti);
		//if it's an abstract code block then we're done with it and actually do want to delete it, but otherwise release it
		AbstractCodeBlock* a;
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) == nullptr)
			nextFullTokenDeleter.release();
	}

	//we have no active expression- this is ok for prefix operators but nothing else
	if (activeExpression == nullptr) {
		if (o->precedence != OperatorTypePrecedence::Prefix)
			Error::makeError(ErrorType::General, "expected a value", fullToken);
		//since we already got the following value we can just return the token
		return fullToken;
	//of course, if we do have an active expression, then we can't have a prefix operator
	} else if (o->precedence == OperatorTypePrecedence::Prefix)
		Error::makeError(ErrorType::General, "unexpected prefix operator", fullToken);

	//since we have an active expression, check to see if it's an operator with a lower precedence
	//if so, we need to go through and reassign
	//nothing can take the right hand side of a postfix operator since it's nullptr
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(Token::getResultingToken(activeExpression))) != nullptr &&
		o->takesRightHandPrecedence(oNext))
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
//if there is no active expression, this could be a cast or a value expression
//if there is, this could be a function call or function parameters
//whatever it is, it's not a statement list
//parse location: the last token of whatever expression uses this abstract code block
//may throw
Token* ParseExpressions::evaluateAbstractCodeBlock(
	AbstractCodeBlock* a, Token* fullToken, Token* activeExpression, ArrayIterator<Token*>* ti)
{
	Identifier* i;
	CType* ct;
	//since we have no active expression, this could be either a value or a cast
	if (activeExpression == nullptr) {
		if (a->tokens->length == 0)
			Error::makeError(ErrorType::General, "expected a value", fullToken);
		Token* lastToken = a->tokens->last();
		if ((i = dynamic_cast<Identifier*>(Token::getResultingToken(lastToken))) != nullptr &&
				(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr)
			return completeCast(ct, a, ti);
		else {
			ArrayIterator<Token*> ai (a->tokens);
			return new ParenthesizedExpression(
				parseExpression(&ai, (unsigned char)SeparatorType::RightParenthesis, "expected an expression", fullToken), a);
		}
	}

	//since we have an active expression, find the rightmost value
	//this could be either a function definition or a function call
	Operator* oParent = nullptr;
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(Token::getResultingToken(activeExpression))) != nullptr) {
		do {
			oParent = oNext;
		} while ((oNext = dynamic_cast<Operator*>(Token::getResultingToken(oParent->right))));
	}
	Token* lastToken = oParent == nullptr ? activeExpression : oParent->right;
	Token* newToken;
	//in order for it to be a function definition, the type must be the last token and not in any parentheses
	if ((i = dynamic_cast<Identifier*>(Token::getResultingToken(lastToken))) != nullptr &&
		(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr)
	{
		newToken = completeFunctionDefinition(ct, lastToken, a, ti);
		delete lastToken;
	} else
		newToken = completeFunctionCall(lastToken, a);
	if (oParent != nullptr) {
		oParent->right = newToken;
		return activeExpression;
	} else
		return newToken;
}
//get a cast expression with the type
//parse location: the next value token after this cast
//may throw
Token* ParseExpressions::completeCast(CType* type, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti) {
	Identifier* i;
	bool rawCast = (castBody->tokens->length >= 2 &&
		(i = dynamic_cast<Identifier*>(Token::getResultingToken(castBody->tokens->first()))) != nullptr &&
		i->name == Keyword::rawKeyword);
	if (castBody->tokens->length > (rawCast ? 2 : 1))
		Error::makeError(ErrorType::General, "unexpected token in type cast", castBody->tokens->get(rawCast ? 1 : 0));
	Cast* cast = new Cast(type, rawCast, castBody);
	Token* castFullToken = castBody->tokens->last();
	//if the type name was substituted, use it for the operator
	SubstitutedToken* st;
	if ((st = dynamic_cast<SubstitutedToken*>(castFullToken)) != nullptr) {
		st->replaceResultingToken(cast);
		castBody->tokens->set(castBody->tokens->length - 1, nullptr);
	} else
		castFullToken = cast;
	Deleter<Token> castDeleter (castFullToken);
	Token* value = addToOperator(cast, castFullToken, nullptr, ti);
	assert(value == cast);
	castDeleter.release();
	return value;
}
//get a function definition from the given abstract code block as parameters plus a following function body
//parse location: the function body
//may throw
Token* ParseExpressions::completeFunctionDefinition(
	CType* type, Token* fullToken, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti)
{
	Array<CVariableDefinition*>* parameters = new Array<CVariableDefinition*>();
	StatementList* statements = nullptr;
	try {
		//parse parameters
		Token* commaFullToken = nullptr;
		forEach(Token*, typeFullToken, parametersBlock->tokens, pi) {
			Token* t = Token::getResultingToken(typeFullToken);
			Identifier* i;
			CType* ct;
			if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
					(ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) == nullptr)
				Error::makeError(ErrorType::General, "expected a type name", typeFullToken);
			i = getExpectedToken<Identifier>(&pi, typeFullToken, "variable name");
			commaFullToken = pi.getNext();
			if (pi.hasThis()) {
				Separator* s;
				if ((s = dynamic_cast<Separator*>(commaFullToken)) == nullptr || s->separatorType != SeparatorType::Comma)
					Error::makeError(ErrorType::General, "expected a comma or right parenthesis", commaFullToken);
			} else
				commaFullToken = nullptr;
			parameters->add(new CVariableDefinition(ct, i->name, nullptr));
		}
		if (commaFullToken != nullptr)
			Error::makeError(ErrorType::General, "trailing comma in parentheses list", commaFullToken);
		AbstractCodeBlock* bodyBlock = getExpectedToken<AbstractCodeBlock>(ti, fullToken, "function body");
		ti->replaceThis(nullptr);
		Deleter<AbstractCodeBlock> bodyBlockDeleter (bodyBlock);
		statements = parseStatements(bodyBlock);
	} catch (...) {
		parameters->deleteContents();
		delete parameters;
		throw;
	}
	return new FunctionDefinition(
		type, parameters, statements, parametersBlock->contentPos, parametersBlock->endContentPos, parametersBlock->owningFile);
}
//convert the given arguments into a list of arguments
//parse location: no change (the location of the given abstract code block)
//may throw
Token* ParseExpressions::completeFunctionCall(Token* function, AbstractCodeBlock* argumentsBlock) {
	Array<Token*>* arguments = new Array<Token*>();
	try {
		forEach(Token*, t, argumentsBlock->tokens, ti) {
			t = parseExpression(
				&ti,
				(unsigned char)(SeparatorType::Comma) | (unsigned char)(SeparatorType::RightParenthesis),
				//we already know that the array isn't empty so we don't have to worry about an empty expression error
				nullptr,
				nullptr);
			arguments->add(t);
		}
	} catch (...) {
		arguments->deleteContents();
		delete arguments;
		throw;
	}
	return new FunctionCall(function, arguments);
}
//get a token of the specified type from the array
//parse location: the location of the resulting token
//may throw
template <class TokenType> TokenType* ParseExpressions::getExpectedToken(
	ArrayIterator<Token*>* ti, Token* precedingFullToken, const char* tokenDescription)
{
	Token* nextFullToken = ti->getNext();
	if (!ti->hasThis()) {
		string errorMessage = string("expected a ") + tokenDescription + " to follow";
		Error::makeError(ErrorType::General, errorMessage.c_str(), precedingFullToken);
	}
	TokenType* t;
	if ((t = dynamic_cast<TokenType*>(Token::getResultingToken(nextFullToken))) == nullptr) {
		string errorMessage = string("expected a ") + tokenDescription;
		Error::makeError(ErrorType::General, errorMessage.c_str(), nextFullToken);
	}
	return t;
}
//parse semicolon-separated statments out of the abstract code block
//parse location: the end of the abstract code block
//may throw
StatementList* ParseExpressions::parseStatements(AbstractCodeBlock* a) {
	Array<Statement*>* statements = new Array<Statement*>();
	Array<CVariableDefinition*>* localVariables = new Array<CVariableDefinition*>();
	try {
		forEach(Token*, fullToken, a->tokens, ti) {
			Token* t = Token::getResultingToken(fullToken);
			Separator* s;
			if ((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon)
				continue;
			Identifier* i;
			CType* ct;
			DirectiveTitle* dt;
			//if it's not an expression statement, it starts with a keyword or a type name for a variable definition
			if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
				if ((ct = CType::globalTypes->get(i->name.c_str(), i->name.length())) != nullptr) {
					CVariableDefinition* definition = completeVariableDefinition(ct, fullToken, &ti);
					localVariables->add(definition);
					if (definition->initialization != nullptr) {
						statements->add(new ExpressionStatement(definition->initialization));
						definition->initialization = nullptr;
					}
					continue;
				} else if (i->name == Keyword::ifKeyword) {
					//TODO: if statments
					continue;
				} else if (i->name == Keyword::forKeyword) {
					//TODO: if statments
					continue;
				} else if (i->name == Keyword::whileKeyword) {
					//TODO: if statments
					continue;
				} else if (i->name == Keyword::doKeyword) {
					//TODO: if statments
					continue;
				}
			} else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr) {
				//TODO: handle #enable stuff
				continue;
			}
			//if we get here then it's a plain, non-empty expression
			//we don't have to worry about an empty expression error message
			statements->add(
				new ExpressionStatement(parseExpression(&ti, (unsigned char)SeparatorType::Semicolon, nullptr, nullptr)));
		}
	} catch (...) {
		statements->deleteContents();
		delete statements;
		localVariables->deleteContents();
		delete localVariables;
		throw;
	}
	return new StatementList(statements, localVariables);
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
