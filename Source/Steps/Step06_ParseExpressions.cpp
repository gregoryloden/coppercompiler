#include "Project.h"

//converts abstract sequences of tokens into structured trees
//provides each source file with a list of global variable definitions and completed type definitions

const char* ParseExpressions::returnKeyword = "return";
const char* ParseExpressions::ifKeyword = "if";
const char* ParseExpressions::elseKeyword = "else";
const char* ParseExpressions::forKeyword = "for";
const char* ParseExpressions::whileKeyword = "while";
const char* ParseExpressions::doKeyword = "do";
const char* ParseExpressions::continueKeyword = "continue";
const char* ParseExpressions::breakKeyword = "break";
const char* ParseExpressions::classKeyword = "class";
const char* ParseExpressions::rawKeyword = "raw";
//parse all expressions in all files
void ParseExpressions::parseExpressionsInFiles(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Parsing expressions for %s...\n", s->filename.c_str());
			parseNamespaceDefinitions(s->abstractContents, s->globalVariables);
		} catch (...) {
		}
	}
}
//get the next token from the array as a token of the specified type
//starting location: the previous token
//ending location: the resulting token
//may throw
template <class TokenType> TokenType* ParseExpressions::parseExpectedToken(
	ArrayIterator<Token*>* ti, Token* precedingToken, const char* tokenDescription)
{
	Token* nextToken = ti->getNext();
	if (!ti->hasThis())
		Error::makeError(ErrorType::ExpectedToFollow, tokenDescription, precedingToken);
	TokenType* t;
	if ((t = dynamic_cast<TokenType*>(nextToken)) == nullptr)
		Error::makeError(ErrorType::Expected, tokenDescription, nextToken);
	return t;
}
//parse all the definitions in the source file or type and store them in the list it provided
void ParseExpressions::parseNamespaceDefinitions(AbstractCodeBlock* a, Array<Token*>* definitionList) {
	forEach(Token*, t, a->tokens, ti) {
		try {
			Identifier* i;
			DirectiveTitle* dt;
			Separator* s;
			if ((i = dynamic_cast<Identifier*>(t)) != nullptr && i->name == classKeyword) {
				//TODO: handle definitions in types
			} else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
				//TODO: handle #enable directives
				continue;
			else if ((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon)
				continue;
			else
				definitionList->add(
					//since we got a token, we know the expression is not empty
					parseExpression(&ti, (unsigned char)SeparatorType::Semicolon, ErrorType::General, nullptr, nullptr));
		} catch (...) {
		}
	}
}
//get a type out of the identifier, possibly with type arguments
//starting location: the identifier
//ending location: no change | the last token of the type arguments list
//may return null (if the identifier is not a type)
//may throw
CDataType* ParseExpressions::parseType(Identifier* i, ArrayIterator<Token*>* ti) {
	CDataType* cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length());
	if (cdt == nullptr)
		return nullptr;

	Token* t = ti->getNext();
	Operator* o;
	if (!ti->hasThis() || (o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::LessThan) {
		ti->getPrevious();
		return cdt;
	}

	if (dynamic_cast<CGenericFunction*>(cdt) != nullptr) {
		//get the return type
		Identifier* returnTypeToken = parseExpectedToken<Identifier>(ti, o, "a return type argument");
		CDataType* returnType = parseType(returnTypeToken, ti);
		if (returnType == nullptr)
			Error::makeError(ErrorType::Expected, "a return type argument", returnTypeToken);

		//get the parameter types list, which will be empty for functions without arguments
		AbstractCodeBlock* typeArgumentsBlock =
			parseExpectedToken<AbstractCodeBlock>(ti, returnTypeToken, "a list of type arguments");
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		Deleter<Array<CDataType*>> parameterTypesDeleter (parameterTypes);
		Token* comma = nullptr;
		forEach(Token*, t2, typeArgumentsBlock->tokens, t2i) {
			Identifier* typeArgumentToken;
			CDataType* typeArgument;
			if ((typeArgumentToken = dynamic_cast<Identifier*>(t2)) == nullptr ||
					(typeArgument = parseType(typeArgumentToken, ti)) == nullptr)
				Error::makeError(ErrorType::Expected, "a type", t2);
			parameterTypes->add(typeArgument);
			comma = parseCommaInParenthesizedList(&t2i);
		}
		if (comma != nullptr)
			Error::makeError(ErrorType::General, "trailing comma in type arguments list", comma);
		o = parseExpectedToken<Operator>(ti, returnTypeToken, "a closing right angle bracket");
		if (o->operatorType != OperatorType::GreaterThan)
			Error::makeError(ErrorType::Expected, "a closing right angle bracket", o);

		//build the type name
		string typeName = "Function<";
		typeName += returnType->name;
		typeName += '(';
		bool addComma = false;
		forEach(CDataType*, c, parameterTypes, ci) {
			if (addComma)
				typeName += ',';
			else
				addComma = true;
			typeName += c->name;
		}
		typeName += ")>";

		//create the type if we don't already have it
		cdt = CDataType::globalDataTypes->get(typeName.c_str(), typeName.length());
		if (cdt == nullptr) {
			cdt = new CSpecificFunction(typeName, returnType, parameterTypesDeleter.release());
			CDataType::globalDataTypes->set(typeName.c_str(), typeName.length(), cdt);
		}
	//TODO: support more than just function types
	} else {
		string message = i->name + " does not take generic type parameters";
		Error::makeError(ErrorType::General, message.c_str(), i);
	}
	return cdt;
}
//check that we've reached the end of the token array or that the next token is a comma
//starting location: the previous token
//ending location: the comma | the end of the token array
//may return null (if we reached the end of the token array)
//may throw
Token* ParseExpressions::parseCommaInParenthesizedList(ArrayIterator<Token*>* ti) {
	Token* comma = ti->getNext();
	if (!ti->hasThis())
		return nullptr;

	Separator* s;
	if ((s = dynamic_cast<Separator*>(comma)) == nullptr || s->separatorType != SeparatorType::Comma) {
		string errorMessage = buildExpectedSeparatorErrorMessage(
			(unsigned char)SeparatorType::Comma | (unsigned char)SeparatorType::RightParenthesis, true);
		Error::makeError(ErrorType::Expected, errorMessage.c_str(), comma);
	}
	return comma;
}
//we parsed a type name and a variable name, now get any other variables defined
//name will be deleted here if needed
//starting location: the variable name
//ending location: the last variable name of the variable definition list
//may throw
VariableDefinitionList* ParseExpressions::completeVariableDefinitionList(
	CDataType* type, Identifier* typeToken, Identifier* name, ArrayIterator<Token*>* ti)
{
	Array<CVariableDefinition*>* variables = new Array<CVariableDefinition*>();
	ArrayContentDeleter<CVariableDefinition> variablesDeleter (variables);
	variables->add(new CVariableDefinition(type, name));
	ti->getNext();
	forEachContinued(Token*, t, ti) {
		Separator* s;
		if ((s = dynamic_cast<Separator*>(t)) == nullptr || s->separatorType != SeparatorType::Comma) {
			ti->getPrevious();
			break;
		}

		Identifier* i = parseExpectedToken<Identifier>(ti, t, "a variable name or type name");
		CDataType* cdt;
		if ((cdt = parseType(i, ti)) != nullptr) {
			type = cdt;
			i = parseExpectedToken<Identifier>(ti, i, "a variable name");
			if (parseType(i, ti) != nullptr)
				Error::makeError(ErrorType::Expected, "a variable name", i);
		}
		ti->replaceThis(nullptr);
		variables->add(new CVariableDefinition(type, i));
	}
	return new VariableDefinitionList(variablesDeleter.release(), typeToken);
}
//this is the main loop of parsing
//return the complete expression that starts at the current token
//fully expects a non-empty expression and nothing else-
//	other functions will find things like parameters, keywords, or empty statements
//does not accept variable initializations
//starting location: the first token of the expression
//ending location: one of the ending separator types | the end of the token array if we can end on a right parenthesis
//may throw
Token* ParseExpressions::parseExpression(
	ArrayIterator<Token*>* ti,
	unsigned char endingSeparatorTypes,
	ErrorType expectedExpressionErrorType,
	const char* expectedExpressionErrorMessage,
	Token* expectedExpressionErrorToken)
{
	Token* activeExpression = nullptr;
	try {
		forEachContinued(Token*, t, ti) {
			ti->replaceThis(nullptr);
			Deleter<Token> tDeleter (t);

			//before we do anything, see if we're done parsing
			Separator* s;
			if ((s = dynamic_cast<Separator*>(t)) != nullptr) {
				//stick the separator back in the token array, something else might need it
				ti->replaceThis(tDeleter.release());
				assert(s->separatorType != SeparatorType::RightParenthesis);
				if (activeExpression != nullptr && (endingSeparatorTypes & (unsigned char)(s->separatorType)) != 0)
					return activeExpression;
				string errorMessage = "unexpected " + Separator::separatorName(s->separatorType, false);
				Error::makeError(ErrorType::General, errorMessage.c_str(), s);
			}

			AbstractCodeBlock* a;
			Operator* o;
			//abstract code blocks and operators have their own handling
			if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
				//check if we have an active expression for a function call
				activeExpression = activeExpression != nullptr
					? completeFunctionCall(activeExpression, a)
					: completeParenthesizedExpression(a, ti);
				continue; //we're done with the abstract code block so let it get deleted
			} else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
				activeExpression = addToOperator(o, activeExpression, ti);
			//we don't have anything yet so we expect a value expression
			else if (activeExpression == nullptr)
				activeExpression = parseValueExpression(t, ti);
			else {
				string errorMessage =
					buildExpectedSeparatorErrorMessage(endingSeparatorTypes, false) + ", operator, or end of expression";
				Error::makeError(ErrorType::Expected, errorMessage.c_str(), t);
			}
			tDeleter.release();
		}

		//an empty expression is never OK
		if (activeExpression == nullptr) {
			assert(expectedExpressionErrorMessage != nullptr && expectedExpressionErrorToken != nullptr);
			Error::makeError(expectedExpressionErrorType, expectedExpressionErrorMessage, expectedExpressionErrorToken);
		//error if we weren't supposed to end on a right parenthesis
		} else if ((endingSeparatorTypes & (unsigned char)SeparatorType::RightParenthesis) == 0) {
			string errorMessage = buildExpectedSeparatorErrorMessage(endingSeparatorTypes, true);
			Error::makeError(ErrorType::ExpectedToFollow, errorMessage.c_str(), getLastToken(activeExpression));
		}
	} catch (...) {
		delete activeExpression;
		throw;
	}
	return activeExpression;
}
//get a value expression that starts at the given token
//if it's an operator, recursively get a value expression (if it's a prefix operator)
//if it's an abstract code block, evaluate the value it represents
//TODO: if it's a directive title, see if it represents a value and get it if so
//if it's a value on it's own, we're done
//starting location: the given token
//ending location: the last token of a value expression that starts at the given token
//may throw
Token* ParseExpressions::parseValueExpression(Token* t, ArrayIterator<Token*>* ti) {
	Operator* o;
	AbstractCodeBlock* a;
	DirectiveTitle* dt;
	Identifier* i;
	//try to parse a prefix operator
	if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		return addToOperator(o, nullptr, ti);
	//parse a parenthesized expression or a cast
	else if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
		return completeParenthesizedExpression(a, ti);
	//a select few directives represent values or contain values like #file
	else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr) {
		//TODO: but not yet
	//check if this is a type or a variable
	} else if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
		CDataType* cdt = parseType(i, ti);
		//we just have a variable
		if (cdt == nullptr)
			return i;
		t = completeExpressionStartingWithType(cdt, i, ti);
		delete i;
		return t;
	//parse a literal value
	} else if (dynamic_cast<IntConstant*>(t) != nullptr ||
			dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<BoolConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr)
		return t;
	assert(dynamic_cast<ParenthesizedExpression*>(t) == nullptr);
	Error::makeError(ErrorType::Expected, "a value", t);
	return nullptr;
}
//this is the meat of parsing, where retcon parsing happens
//if applicable, parse a value expression to the right of this operator
//this operator could be prefix, postfix, binary, or ternary, and it might have been lexed wrong
//whatever it is, figure it out with what we've got
//starting location: the given operator
//ending location: the next value token after this operator
//may throw
Token* ParseExpressions::addToOperator(Operator* o, Token* activeExpression, ArrayIterator<Token*>* ti) {
	//if we parsed a subtraction sign but there's no active expression, it's a negative sign
	if (activeExpression == nullptr && o->operatorType == OperatorType::Subtract) {
		o->operatorType = OperatorType::Negate;
		o->precedence = OperatorTypePrecedence::Prefix;
	}

	//whatever the operator is, if it's not postfix then we expect a value to follow
	if (o->precedence != OperatorTypePrecedence::Postfix) {
		Token* t = parseExpectedToken<Token>(ti, o, "a value");
		ti->replaceThis(nullptr);
		Deleter<Token> tDeleter (t);
		AbstractCodeBlock* a = dynamic_cast<AbstractCodeBlock*>(t);
		o->right = parseValueExpression(t, ti);
		//if it's an abstract code block then we're done with it and actually do want to delete it, but otherwise release it
		if (a == nullptr)
			tDeleter.release();
	}

	//we have no active expression- this is ok for prefix or static member operators but nothing else
	if (activeExpression == nullptr) {
		if (o->precedence != OperatorTypePrecedence::Prefix && o->precedence != OperatorTypePrecedence::StaticMember)
			Error::makeError(ErrorType::Expected, "a value", o);
		//since we already got the value following it we can just return the token
		return o;
	//of course, if we do have an active expression, then we can't have a prefix operator
	} else if (o->precedence == OperatorTypePrecedence::Prefix)
		Error::makeError(ErrorType::General, "unexpected prefix operator", o);

	//since we have an active expression, check to see if it's an operator with a lower precedence
	//if so, we need to go through and reassign
	//this handles postfix operators since their right side is nullptr
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(activeExpression)) != nullptr && o->takesRightSidePrecedence(oNext)) {
		Operator* oParent = oNext;
		while ((oNext = dynamic_cast<Operator*>(oParent->right)) != nullptr && o->takesRightSidePrecedence(oNext))
			oParent = oNext;
		o->left = oParent->right;
		oParent->right = o;
		return activeExpression;
	//it's not an operator or it's a higher precedence, we're done
	} else {
		o->left = activeExpression;
		return o;
	}
}
//get a value expression that starts with the given type,
//	either a function definition, a variable initialization, or a static member operator
//starting location: the last token of the given type
//ending location: the last token of the value that starts with the given type
//may throw
Token* ParseExpressions::completeExpressionStartingWithType(CDataType* type, Identifier* typeToken, ArrayIterator<Token*>* ti) {
	AbstractCodeBlock* a;
	Identifier* i;
	Operator* o;
	Token* t = parseExpectedToken<Token>(ti, typeToken, "function parameters or a member operator");
	if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
		return completeFunctionDefinition(type, typeToken, a, ti);
	else if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
		ti->replaceThis(nullptr);
		return completeVariableDefinitionList(type, typeToken, i, ti);
	} else if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
		OperatorType newType;
		if (o->operatorType == OperatorType::Dot)
			newType = OperatorType::StaticDot;
		else if (o->operatorType == OperatorType::ObjectMemberAccess)
			newType = OperatorType::StaticMemberAccess;
		else
			Error::makeError(ErrorType::General, "unexpected operator following type", t);
		StaticOperator* s = new StaticOperator(type, newType, o);
		ti->replaceThis(nullptr);
		delete o;
		Deleter<StaticOperator> sDeleter (s);
		t = addToOperator(s, nullptr, ti);
		sDeleter.release();
		return t;
	}
	Error::makeError(ErrorType::Expected, "a variable name, function parameters, or a member operator", t);
	return nullptr;
}
//get a value expression that starts with the abstract code block
//this could be a variable initialization (value required), a cast (if castI is not nullptr), or just a regular expression
//starting location: the given abstract code block
//ending location: no change | the next value token after the cast
//may throw
Token* ParseExpressions::completeParenthesizedExpression(AbstractCodeBlock* a, ArrayIterator<Token*>* castI) {
	Identifier* i;
	ArrayIterator<Token*> ai (a->tokens);
	//check for "raw" or a type
	if (ai.hasThis() && (i = dynamic_cast<Identifier*>(ai.getThis())) != nullptr) {
		CDataType* cdt;
		//starts with "raw", this must be a cast
		if (i->name == rawKeyword) {
			//statements do not expect a cast and will not pass an iterator since there are no potentially valid extra tokens
			if (castI == nullptr)
				Error::makeError(ErrorType::Expected, "a value", i);
			i = parseExpectedToken<Identifier>(&ai, i, "a type name");
			if ((cdt = parseType(i, &ai)) == nullptr)
				Error::makeError(ErrorType::Expected, "a type name", i);
			ai.getNext();
			if (ai.hasThis())
				Error::makeError(ErrorType::General, "unexpected token in type cast", ai.getThis());
			return completeCast(cdt, true, a, castI);
		//starts with a type, check if it's a a cast
		} else if ((cdt = parseType(i, &ai)) != nullptr) {
			ai.getNext();
			//a cast does not have any tokens following it
			if (!ai.hasThis()) {
				//statements do not expect a cast and will not pass an iterator since there are no potentially valid extra tokens
				if (castI == nullptr)
					Error::makeError(ErrorType::Expected, "a value", i);
				return completeCast(cdt, false, a, castI);
			}
			//otherwise it's just a regular expression
			ai.getPrevious();
			return completeExpressionStartingWithType(cdt, i, &ai);
		}
	}
	//we do not have a cast or a type, this is a regular expression
	EmptyToken errorToken (a->contentPos + 1, a->owningFile);
	Token* expression = parseExpression(
		&ai, (unsigned char)SeparatorType::RightParenthesis, ErrorType::Expected, "an expression", &errorToken);
	//statements expect a value to use for the condition and do not need it wrapped
	//otherwise we do need to wrap it to make sure retcon parsing doesn't mess it up
	return castI != nullptr ? new ParenthesizedExpression(expression, a) : expression;
}
//get a function definition from the given abstract code block as parameters and then parse a following function body
//starting location: the parameters abstract code block
//ending location: the function body abstract code block
//may throw
FunctionDefinition* ParseExpressions::completeFunctionDefinition(
	CDataType* type, Identifier* typeToken, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti)
{
	Array<CVariableDefinition*>* parameters = new Array<CVariableDefinition*>();
	ArrayContentDeleter<CVariableDefinition> parametersDeleter (parameters);
	//parse parameters
	Token* comma = nullptr;
	forEach(Token*, t, parametersBlock->tokens, pi) {
		Identifier* i;
		CDataType* cdt;
		if ((i = dynamic_cast<Identifier*>(t)) == nullptr || (cdt = parseType(i, &pi)) == nullptr)
			Error::makeError(ErrorType::Expected, "a type name", t);
		Identifier* variableName = parseExpectedToken<Identifier>(&pi, t, "a variable name");
		if (parseType(variableName, &pi) != nullptr)
			Error::makeError(ErrorType::Expected, "a variable name", variableName);
		pi.replaceThis(nullptr);
		Deleter<Identifier> variableNameDeleter (variableName);
		parameters->add(new CVariableDefinition(cdt, variableNameDeleter.release()));
		comma = parseCommaInParenthesizedList(&pi);
	}
	if (comma != nullptr)
		Error::makeError(ErrorType::General, "trailing comma in parameters list", comma);
	AbstractCodeBlock* a = parseExpectedToken<AbstractCodeBlock>(ti, parametersBlock, "a function body");
	Array<Statement*>* statements = new Array<Statement*>();
	ArrayContentDeleter<Statement> statementsDeleter (statements);
	forEach(Token*, st, a->tokens, ai) {
		Statement* s;
		if ((s = parseStatement(st, &ai, true)) != nullptr)
			statements->add(s);
	}
	return new FunctionDefinition(type, parametersDeleter.release(), statementsDeleter.release(), typeToken);
}
//find the function token in the active expression and combine it with the given abstract code block of arguments
//may throw
Token* ParseExpressions::completeFunctionCall(Token* activeExpression, AbstractCodeBlock* argumentsBlock) {
	//find the rightmost value of our active expression
	Operator* oParent = nullptr;
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(activeExpression)) != nullptr) {
		do {
			oParent = oNext;
		} while ((oNext = dynamic_cast<Operator*>(oParent->right)) &&
			//function calls are like postfix operators, don't steal the right side of operators with a higher precedence
			oNext->precedence < OperatorTypePrecedence::Postfix);
	}
	Token* function = oParent == nullptr ? activeExpression : oParent->right;
	//build a list of arguments
	Array<Token*>* arguments = new Array<Token*>();
	ArrayContentDeleter<Token> argumentsDeleter (arguments);
	Token* comma = nullptr;
	forEach(Token*, t, argumentsBlock->tokens, ti) {
		arguments->add(parseExpression(
			&ti,
			(unsigned char)SeparatorType::Comma | (unsigned char)SeparatorType::RightParenthesis,
			//we already know that the array isn't empty so we don't have to worry about an empty expression error
			ErrorType::General,
			nullptr,
			nullptr));
		if (ti.hasThis())
			comma = ti.getThis();
		else
			comma = nullptr;
	}
	if (comma != nullptr)
		Error::makeError(ErrorType::General, "trailing comma in arguments list", comma);
	//stick it back into the active expression if possible, or return the whole function call if not
	FunctionCall* functionCall = new FunctionCall(function, argumentsDeleter.release(), argumentsBlock);
	if (oParent != nullptr) {
		oParent->right = functionCall;
		return activeExpression;
	} else
		return functionCall;
}
//get a cast expression using the given type
//starting location: the cast body
//ending location: the next value token after this cast
//may throw
Token* ParseExpressions::completeCast(CDataType* type, bool rawCast, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti) {
	Cast* cast = new Cast(type, rawCast, castBody);
	Deleter<Token> castDeleter (cast);
	Token* value = addToOperator(cast, nullptr, ti);
	assert(value == cast);
	return castDeleter.release();
}
//get a single statement or a statement list, erroring if there's nothing left
//starting location: the token preceding the statement or statement list
//ending location: the statement list (statement list) | the semicolon of the statement (single statement)
//may throw
Array<Statement*>* ParseExpressions::parseStatementOrStatementList(
	ArrayIterator<Token*>* ti, Token* noValueErrorToken, const char* statementDescription)
{
	Token* t = parseExpectedToken<Token>(ti, noValueErrorToken, statementDescription);
	Array<Statement*>* statements = new Array<Statement*>();
	ArrayContentDeleter<Statement> statementsDeleter (statements);
	//if the next token is an abstract code block, it might be a statement list
	AbstractCodeBlock* a;
	Statement* s;
	if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
		//we have a statement list if
		//	-it's empty OR
		//	-it has a semicolon OR
		//	-it starts with a directive statement OR
		//	-it starts with a keyword statement
		ArrayIterator<Token*> ai (a->tokens);
		Statement* firstStatement = nullptr;
		if (a->tokens->length == 0 ||
			hasSemicolon(a) ||
			(firstStatement = parseDirectiveStatementList(ai.getThis(), &ai)) != nullptr ||
			(firstStatement = parseKeywordStatement(ai.getThis(), &ai)) != nullptr)
		{
			if (firstStatement != nullptr) {
				statements->add(firstStatement);
				ai.getNext();
			}
			forEachContinued(Token*, t2, (&ai)) {
				//a directive statement list is ok here since we're in a statement list
				if ((s = parseStatement(t2, &ai, true)) != nullptr)
					statements->add(s);
			}
			return statementsDeleter.release();
		}
	}
	//since we got here, it's a single statement
	//a directive statement list is not ok here since we're looking for a single statement
	if ((s = parseStatement(t, ti, true)) != nullptr)
		statements->add(s);
	return statementsDeleter.release();
}
//parse a single statement starting at the current token
//includes expression statements, keyword statements and empty statements
//starting location: the given first token of the statement
//ending location: the last token of the statement
//may return null (for empty statements)
//may throw
Statement* ParseExpressions::parseStatement(Token* t, ArrayIterator<Token*>* ti, bool permitDirectiveStatementList) {
	Statement* result;
	if (permitDirectiveStatementList && (result = parseDirectiveStatementList(t, ti)) != nullptr)
		return result;
	if ((result = parseKeywordStatement(t, ti)) != nullptr)
		return result;
	return parseExpressionStatement(t, ti);
}
//parse a statement if the current token is a statement-surrounding directive
//starting location: the given first token of a statement
//parse location: no change | the last token of the directive
//may return null (if we're not at a directive statement)
//may throw
Statement* ParseExpressions::parseDirectiveStatementList(Token* t, ArrayIterator<Token*>* ti) {
	DirectiveTitle* dt;
	if ((dt = dynamic_cast<DirectiveTitle*>(t)) == nullptr)
		return nullptr;

	CDirective* directive = dt->directive;
	//these directives are valid syntactically but they don't do anything as statements
	if (dynamic_cast<CDirectiveReplace*>(directive) != nullptr)
		return new EmptyStatement();
	//TODO: handle #enable stuff
	return nullptr;
}
//parse a statement if the current token is a keyword identifier
//starting location: the given first token of a statement
//ending location: no change | the last token of the keyword statement
//may return null (if we're not at a keyword statement)
//may throw
Statement* ParseExpressions::parseKeywordStatement(Token* t, ArrayIterator<Token*>* ti) {
	Identifier* keywordToken;
	if ((keywordToken = dynamic_cast<Identifier*>(t)) == nullptr)
		return nullptr;

	string keyword = keywordToken->name;
	bool continueLoop;
	if (keyword == returnKeyword) {
		Token* t = parseExpectedToken<Token>(ti, keywordToken, "an expression or semicolon");
		Separator* s;
		return new ReturnStatement((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon
			? nullptr
			: parseExpression(ti, (unsigned char)SeparatorType::Semicolon, ErrorType::General, nullptr, nullptr));
	} else if (keyword == ifKeyword) {
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, keywordToken, "a condition expression");
		Deleter<Token> condition (completeParenthesizedExpression(conditionBlock, nullptr));
		ArrayContentDeleter<Statement> thenBody (parseStatementOrStatementList(ti, conditionBlock, "if-statement body"));
		Array<Statement*>* elseBody = nullptr;
		Token* elseToken = ti->getNext();
		Identifier* i;
		if (ti->hasThis() && (i = dynamic_cast<Identifier*>(elseToken)) != nullptr && i->name == elseKeyword)
			elseBody = parseStatementOrStatementList(ti, i, "else-statement body");
		//go back if it's not an else keyword
		else
			ti->getPrevious();
		return new IfStatement(condition.release(), thenBody.release(), elseBody);
	} else if (keyword == forKeyword) {
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, keywordToken, "a for-loop condition");
		ArrayIterator<Token*> ai (conditionBlock->tokens);
		if (!ai.hasThis())
			Error::makeError(ErrorType::Expected, "a for-loop initialization", conditionBlock);
		Deleter<ExpressionStatement> initialization (parseExpressionStatement(ai.getThis(), &ai));
		Token* initializationSemicolon = ai.getThis();
		ai.getNext();
		Deleter<Token> condition (parseExpression(
			&ai,
			(unsigned char)SeparatorType::Semicolon,
			ErrorType::ExpectedToFollow,
			"a for-loop condition",
			initializationSemicolon));
		ai.getNext();
		Token* increment = ai.hasThis()
			? parseExpression(&ai, (unsigned char)SeparatorType::RightParenthesis, ErrorType::General, nullptr, nullptr)
			: nullptr;
		Deleter<Token> incrementDeleter (increment);
		Array<Statement*>* body = parseStatementOrStatementList(ti, conditionBlock, "a for-loop body");
		return new LoopStatement(initialization.release(), condition.release(), incrementDeleter.release(), body, true);
	} else if (keyword == whileKeyword) {
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, keywordToken, "a condition expression");
		Deleter<Token> condition (completeParenthesizedExpression(conditionBlock, nullptr));
		Array<Statement*>* body = parseStatementOrStatementList(ti, conditionBlock, "a while-loop body");
		return new LoopStatement(nullptr, condition.release(), nullptr, body, true);
	} else if (keyword == doKeyword) {
		ArrayContentDeleter<Statement> body (parseStatementOrStatementList(ti, keywordToken, "a do-while-loop body"));
		Identifier* whileToken = parseExpectedToken<Identifier>(ti, ti->getThis(), "a while-condition");
		if (whileToken->name != whileKeyword)
			Error::makeError(ErrorType::Expected, "a \"while\" keyword", whileToken);
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, whileToken, "a condition expression");
		Token* condition = completeParenthesizedExpression(conditionBlock, nullptr);
		return new LoopStatement(nullptr, condition, nullptr, body.release(), false);
	} else if ((continueLoop = (keyword == continueKeyword)) || keyword == breakKeyword) {
		Token* t = parseExpectedToken<Token>(ti, keywordToken, "a semicolon or integer literal");
		IntConstant* levels;
		if ((levels = dynamic_cast<IntConstant*>(t)) == nullptr) {
			ti->getPrevious();
			t = keywordToken;
		} else
			ti->replaceThis(nullptr);
		Deleter<IntConstant> levelsDeleter (levels);
		Separator* s = parseExpectedToken<Separator>(ti, t, "a semicolon");
		if (s->separatorType != SeparatorType::Semicolon)
			Error::makeError(ErrorType::Expected, "a semicolon", s);
		return new LoopControlFlowStatement(continueLoop, levelsDeleter.release());
	}
	return nullptr;
}
//parse a single expression statement starting at the current token, including any variable definitions
//starting location: the given first token of the statement
//ending location: the end semicolon of the statement
//may return null (for empty statements)
//may throw
ExpressionStatement* ParseExpressions::parseExpressionStatement(Token* t, ArrayIterator<Token*>* ti) {
	Separator* s;
	return (s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon
		? nullptr
		: new ExpressionStatement(
			parseExpression(ti, (unsigned char)SeparatorType::Semicolon, ErrorType::General, nullptr, nullptr));
}
//get the right-most token of an operator tree, if there is one
Token* ParseExpressions::getLastToken(Token* t) {
	Operator* o;
	while ((o = dynamic_cast<Operator*>(t)) != nullptr && o->right != nullptr)
		t = o->right;
	return t;
}
//check if there is a semicolon- if so then this is a statement list, if not then it's part of a single statement
bool ParseExpressions::hasSemicolon(AbstractCodeBlock* a) {
	forEach(Token*, t, a->tokens, ti) {
		Separator* s;
		if ((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon)
			return true;
	}
	return false;
}
/*
//check if the string is a keyword
bool ParseExpressions::isKeyword(string s) {
	return
		//values
		s.compare("true") == 0 ||
		s.compare("false") == 0 ||
		s.compare("null") == 0 ||
		s.compare("recurse") == 0 ||
		//control flow
		s.compare(returnKeyword) == 0 ||
		s.compare(ifKeyword) == 0 ||
		s.compare(elseKeyword) == 0 ||
		s.compare(forKeyword) == 0 ||
		s.compare(whileKeyword) == 0 ||
		s.compare(doKeyword) == 0 ||
		s.compare(continueKeyword) == 0 ||
		s.compare(breakKeyword) == 0 ||
		s.compare("switch") == 0 ||
		s.compare("case") == 0 ||
		s.compare("default") == 0 ||
		s.compare("goto") == 0 ||
		//memory
		s.compare("new") == 0 ||
		s.compare("delete") == 0 ||
		//access modifiers
		//s.compare("public") == 0 ||
		s.compare("private") == 0 ||
		s.compare("readonly") == 0 ||
		s.compare("writeonly") == 0 ||
		s.compare("local") == 0 ||
		s.compare("global") == 0 ||
		s.compare("final") == 0 ||
		//s.compare("nonnull") == 0 ||
		//s.compare("nullable") == 0 ||
		s.compare("perthread") == 0 ||
		//types
		s.compare("is") == 0 ||
		s.compare(classKeyword) == 0 ||
		s.compare("enum") == 0 ||
		s.compare("operator") == 0 ||
		s.compare(rawKeyword) == 0 ||
		s.compare("partial") == 0 ||
		s.compare("abstract") == 0;
}
*/
//construct a comma-separated list of separator type names from the given mask
string ParseExpressions::buildExpectedSeparatorErrorMessage(unsigned char expectedSeparatorTypesMask, bool concludeList) {
	SeparatorType separatorTypes[] = {
		SeparatorType::LeftParenthesis,
		SeparatorType::RightParenthesis,
		SeparatorType::Comma,
		SeparatorType::Semicolon
	};
	const int separatorTypesCount = sizeof(separatorTypes) / sizeof(separatorTypes[0]);
	SeparatorType expectedSeparatorTypes[separatorTypesCount];
	int expectedSeparatorTypesCount = 0;
	for (int i = 0; i < separatorTypesCount; i++) {
		if ((expectedSeparatorTypesMask & (unsigned char)(separatorTypes[i])) != 0) {
			expectedSeparatorTypes[expectedSeparatorTypesCount] = separatorTypes[i];
			expectedSeparatorTypesCount++;
		}
	}
	string errorMessage = "";
	for (int i = 0; i < expectedSeparatorTypesCount; i++) {
		if (i == 0)
			errorMessage += Separator::separatorName(expectedSeparatorTypes[i], true);
		else if (i + 1 < expectedSeparatorTypesCount || !concludeList)
			errorMessage += ", " + Separator::separatorName(expectedSeparatorTypes[i], false);
		else if (expectedSeparatorTypesCount == 2)
			errorMessage += " or " + Separator::separatorName(expectedSeparatorTypes[i], false);
		else
			errorMessage += ", or " + Separator::separatorName(expectedSeparatorTypes[i], false);
	}
	return errorMessage;
}
