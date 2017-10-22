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
			parseGlobalDefinitions(s);
		} catch (...) {
		}
	}
}
//get the next token from the array as a token of the specified type
//parse location: the location of the resulting token
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
//parse all the definitions in the source file and store them in it
//parse location: EOF
void ParseExpressions::parseGlobalDefinitions(SourceFile* sf) {
	forEach(Token*, t, sf->abstractContents->tokens, ti) {
		try {
			ti.replaceThis(nullptr);
			Deleter<Token> tDeleter (t);
			Identifier* i;
			DirectiveTitle* dt;
			Separator* s;
			if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
				CDataType* cdt;
				if (i->name == classKeyword) {
					//TODO: handle type definitions
				} if ((cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) != nullptr) {
					//make sure this is a variable definition
					Identifier* variableName = parseExpectedToken<Identifier>(&ti, t, "a variable name");
					ti.replaceThis(nullptr);
					sf->globalVariables->add(
						completeVariableInitialization(i, cdt, variableName, &ti, SeparatorType::Semicolon));
					continue;
				}
			} else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
				//TODO: handle #enable directives
				continue;
			else if ((s = dynamic_cast<Separator*>(s)) != nullptr && s->separatorType == SeparatorType::Semicolon)
				continue;
			Error::makeError(ErrorType::Expected, "a type or variable definition", t);
		} catch (...) {
		}
	}
}
//we parsed a type name and a variable name, now get any other variables defined, possibly with an initialization
//parse location: the semicolon after the variable definitions/initialization | the end of the token array
//may throw
VariableInitialization* ParseExpressions::completeVariableInitialization(
	Identifier* typeToken, CDataType* type, Identifier* name, ArrayIterator<Token*>* ti, SeparatorType endingSeparatorType)
{
	Array<CVariableDefinition*>* variables = new Array<CVariableDefinition*>();
	ArrayContentDeleter<CVariableDefinition> variablesDeleter (variables);
	variables->add(new CVariableDefinition(type, name));
	Identifier* lastName = name;
	string expectedTokensMessage =
		buildExpectedSeparatorErrorMessage((unsigned char)endingSeparatorType | (unsigned char)SeparatorType::Comma, false) +
		", or assignment operator";
	while (true) {
		Token* t = parseExpectedToken<Token>(ti, lastName, expectedTokensMessage.c_str());
		Separator* s;
		Operator* o;
		if ((s = dynamic_cast<Separator*>(t)) != nullptr &&
			(s->separatorType == SeparatorType::Comma || s->separatorType == endingSeparatorType))
		{
			if (s->separatorType == SeparatorType::Comma) {
				Identifier* i = parseExpectedToken<Identifier>(ti, t, "a variable name or type name");
				CDataType* cdt;
				if ((cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) != nullptr) {
					type = cdt;
					i = parseExpectedToken<Identifier>(ti, i, "a variable name");
				}
				lastName = i;
				ti->replaceThis(nullptr);
				variables->add(new CVariableDefinition(type, lastName));
			} else
				return new VariableInitialization(variablesDeleter.release(), nullptr, lastName);
		} else if ((o = dynamic_cast<Operator*>(t)) != nullptr && o->operatorType == OperatorType::Assign) {
			ti->getNext();
			Token* initialization = parseExpression(
				ti,
				(unsigned char)endingSeparatorType,
				ErrorType::ExpectedToFollow,
				"an variable initialization expression",
				o);
			return new VariableInitialization(variablesDeleter.release(), initialization, getLastToken(initialization));
		} else
			Error::makeError(ErrorType::Expected, expectedTokensMessage.c_str(), t);
	}
}
//this is the main loop of parsing
//return the expression that starts at the current token
//fully expects a non-empty expression and nothing else- other functions will find things like parameters or keywords
//does not accept variable initializations
//parse location: one of the ending separator types | the end of the token array if we can end on a right parenthesis
//may throw
Token* ParseExpressions::parseExpression(ArrayIterator<Token*>* ti, unsigned char endingSeparatorTypes,
	ErrorType expectedExpressionErrorType, const char* expectedExpressionErrorMessage, Token* expectedExpressionErrorToken)
{
	Token* activeExpression = nullptr;
	try {
		forEachContinued(Token*, t, ti) {
			ti->replaceThis(nullptr);
			Deleter<Token> tDeleter (t);

			//before we do anything, see if we're done parsing
			Separator* s;
			if ((s = dynamic_cast<Separator*>(t)) != nullptr) {
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
				activeExpression = evaluateAbstractCodeBlock(a, activeExpression, ti);
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
//if it's an operator, recursively get a value expression if it's a prefix operator
//if it's an abstract code block, evaluate the value it represents
//TODO: if it's a directive title, see if it represents a value and get it if so
//if it's a value on it's own, we're done
//parse location: the location of the given token | the next value token after the location of the given operator
//may throw
Token* ParseExpressions::parseValueExpression(Token* t, ArrayIterator<Token*>* ti) {
	Operator* o;
	AbstractCodeBlock* a;
	DirectiveTitle* dt;
	//try to parse a prefix operator
	if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		return addToOperator(o, nullptr, ti);
	//parse a parenthesized expression or a cast
	else if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
		return evaluateAbstractCodeBlock(a, nullptr, ti);
	//a select few directives represent values or contain values like #file
	else if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr) {
		//TODO: but not yet
	//parse a regular single value
	} else if (dynamic_cast<Identifier*>(t) != nullptr ||
			dynamic_cast<IntConstant*>(t) != nullptr ||
			dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr)
		return t;
	assert(dynamic_cast<ParenthesizedExpression*>(t) == nullptr);
	Error::makeError(ErrorType::Expected, "a value", t);
	return nullptr;
}
//this is the meat of parsing, where retcon parsing happens
//if applicable, add a token to the right side of this operator, cascading as necessary
//it could be prefix, postfix, binary, or ternary, and it might have been lexed wrong
//whatever it is, figure it out with what we've got
//parse location: the next value token after this operator
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
		o->right = parseValueExpression(t, ti);
		//if it's an abstract code block then we're done with it and actually do want to delete it, but otherwise release it
		AbstractCodeBlock* a;
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) == nullptr)
			tDeleter.release();
	}

	//we have no active expression- this is ok for prefix operators but nothing else
	if (activeExpression == nullptr) {
		if (o->precedence != OperatorTypePrecedence::Prefix)
			Error::makeError(ErrorType::Expected, "a value", o);
		//since we already got the following value we can just return the token
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
//complete whatever expression we have using the abstract code block
//if there is no active expression, this could be a cast or a value expression
//if there is, this could be a function call or function parameters
//but it's not a statement list
//parse location: the last token of whatever expression uses this abstract code block
//may throw
Token* ParseExpressions::evaluateAbstractCodeBlock(AbstractCodeBlock* a, Token* activeExpression, ArrayIterator<Token*>* ti) {
	Identifier* i;
	CDataType* cdt;
	//since we have no active expression, this could be either a cast, a variable initialization, or a value
	if (activeExpression == nullptr) {
		if (a->tokens->length == 0)
			Error::makeError(ErrorType::Expected, "a value", a);
		Token* lastToken = a->tokens->last();
		if ((i = dynamic_cast<Identifier*>(lastToken)) != nullptr &&
				(cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) != nullptr)
			return completeCast(cdt, a, ti);
		else
			return completeParenthesizedExpression(a, true);
	}

	//since we have an active expression, find the rightmost value
	//this could be either a function definition or a function call
	Operator* oParent = nullptr;
	Operator* oNext;
	if ((oNext = dynamic_cast<Operator*>(activeExpression)) != nullptr) {
		do {
			oParent = oNext;
		} while ((oNext = dynamic_cast<Operator*>(oParent->right)) &&
			//function calls are like postfix operators, don't steal the right side of operators with a higher precedence
			oNext->precedence < OperatorTypePrecedence::Postfix);
	}
	Token* lastToken = oParent == nullptr ? activeExpression : oParent->right;
	Token* newToken;
	//in order for it to be a function definition, the type must be the last token and not in any parentheses
	if ((i = dynamic_cast<Identifier*>(lastToken)) != nullptr &&
		(cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) != nullptr)
	{
		newToken = completeFunctionDefinition(cdt, a, ti);
		delete lastToken;
	} else
		newToken = completeFunctionCall(lastToken, a);
	if (oParent != nullptr) {
		oParent->right = newToken;
		return activeExpression;
	} else
		return newToken;
}
//get a cast expression with the type, and check that the abstract code block is a proper cast
//parse location: the next value token after this cast
//may throw
Token* ParseExpressions::completeCast(CDataType* type, AbstractCodeBlock* castBody, ArrayIterator<Token*>* ti) {
	Identifier* i;
	bool rawCast = (castBody->tokens->length >= 2 &&
		(i = dynamic_cast<Identifier*>(castBody->tokens->first())) != nullptr &&
		i->name == rawKeyword);
	if (castBody->tokens->length > (rawCast ? 2 : 1))
		Error::makeError(ErrorType::General, "unexpected token in type cast", castBody->tokens->get(rawCast ? 1 : 0));
	Cast* cast = new Cast(type, rawCast, castBody);
	Deleter<Token> castDeleter (cast);
	Token* value = addToOperator(cast, nullptr, ti);
	assert(value == cast);
	return castDeleter.release();
}
//get an expression from the abstract code block, possibly a variable initialization (with a value required)
//parse location: no change (the location of the abstract code block)
//may throw
Token* ParseExpressions::completeParenthesizedExpression(AbstractCodeBlock* a, bool wrapExpression) {
	Identifier* typeName;
	CDataType* cdt;
	Identifier* variableName;
	ArrayIterator<Token*> ai (a->tokens);
	//check if we have a variable initialization
	if (a->tokens->length >= 2 &&
		(typeName = dynamic_cast<Identifier*>(a->tokens->first())) != nullptr &&
		(cdt = CDataType::globalDataTypes->get(typeName->name.c_str(), typeName->name.length())) != nullptr &&
		(variableName = dynamic_cast<Identifier*>(a->tokens->get(1))) != nullptr)
	{
		//don't forget to remove the variable name from the array
		ai.getNext();
		ai.replaceThis(nullptr);
		VariableInitialization* variableInitialization =
			completeVariableInitialization(typeName, cdt, variableName, &ai, SeparatorType::RightParenthesis);
		//in an expression, declared variables must be initialized
		if (variableInitialization->initialization == nullptr) {
			Deleter<VariableInitialization> variableInitializationDeleter (variableInitialization);
			Error::makeError(ErrorType::ExpectedToFollow, "a variable initialization", variableInitialization);
		}
		return variableInitialization;
	} else {
		EmptyToken errorToken (a->contentPos + 1, a->owningFile);
		Token* expression = parseExpression(
			&ai, (unsigned char)SeparatorType::RightParenthesis, ErrorType::Expected, "an expression", &errorToken);
		return wrapExpression ? new ParenthesizedExpression(expression, a) : expression;
	}
}
//get a function definition from the given abstract code block as parameters and then parse a following function body
//parse location: the function body
//may throw
Token* ParseExpressions::completeFunctionDefinition(
	CDataType* type, AbstractCodeBlock* parametersBlock, ArrayIterator<Token*>* ti)
{
	Array<CVariableDefinition*>* parameters = new Array<CVariableDefinition*>();
	ArrayContentDeleter<CVariableDefinition> parametersDeleter (parameters);
	//parse parameters
	Token* comma = nullptr;
	forEach(Token*, t, parametersBlock->tokens, pi) {
		Identifier* i;
		CDataType* cdt;
		if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
				(cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) == nullptr)
			Error::makeError(ErrorType::Expected, "a type name", t);
		Identifier* variableName = parseExpectedToken<Identifier>(&pi, t, "a variable name");
		pi.replaceThis(nullptr);
		Deleter<Identifier> variableNameDeleter (variableName);
		comma = pi.getNext();
		if (pi.hasThis()) {
			Separator* s;
			if ((s = dynamic_cast<Separator*>(comma)) == nullptr || s->separatorType != SeparatorType::Comma) {
				string errorMessage = buildExpectedSeparatorErrorMessage(
					(unsigned char)SeparatorType::Comma | (unsigned char)SeparatorType::RightParenthesis, true);
				Error::makeError(ErrorType::Expected, errorMessage.c_str(), comma);
			}
		} else
			comma = nullptr;
		parameters->add(new CVariableDefinition(cdt, variableNameDeleter.release()));
	}
	if (comma != nullptr)
		Error::makeError(ErrorType::General, "trailing comma in parentheses list", comma);
	Array<Statement*>* statements = parseStatementList(ti, parametersBlock, "a function body", false);
	return new FunctionDefinition(type, parametersDeleter.release(), statements, ti->getThis());
}
//convert the given arguments into a list of arguments
//parse location: no change (the location of the given abstract code block)
//may throw
Token* ParseExpressions::completeFunctionCall(Token* function, AbstractCodeBlock* argumentsBlock) {
	Array<Token*>* arguments = new Array<Token*>();
	ArrayContentDeleter<Token> argumentsDeleter (arguments);
	forEach(Token*, t, argumentsBlock->tokens, ti) {
		arguments->add(parseExpression(
			&ti,
			(unsigned char)SeparatorType::Comma | (unsigned char)SeparatorType::RightParenthesis,
			//we already know that the array isn't empty so we don't have to worry about an empty expression error
			ErrorType::General,
			nullptr,
			nullptr));
	}
	return new FunctionCall(function, argumentsDeleter.release(), argumentsBlock);
}
//get a statement list, erroring if there's nothing left
//if it's a single statement and we accept those, return just that
//if it's a single statement and we don't accept it, error because we require a parenthesized statement list
//parse location: the statement list (statement list) | the semicolon of the statement (single statement)
//may throw
Array<Statement*>* ParseExpressions::parseStatementList(
	ArrayIterator<Token*>* ti, Token* noValueErrorToken, const char* statementDescription, bool acceptSingleStatement)
{
	Token* t = parseExpectedToken<Token>(ti, noValueErrorToken, statementDescription);
	Array<Statement*>* statements = new Array<Statement*>();
	ArrayContentDeleter<Statement> statementsDeleter (statements);
	//if this is not an abstract code block or it is but it lacks semicolons, then this is a single statement
	AbstractCodeBlock* a;
	if ((a = dynamic_cast<AbstractCodeBlock*>(t)) == nullptr || (!hasSemicolon(a) && a->tokens->length > 0)) {
		if (acceptSingleStatement) {
			Statement* s;
			if ((s = parseStatement(ti)) != nullptr)
				statements->add(s);
			return statementsDeleter.release();
		} else
			Error::makeError(ErrorType::Expected, statementDescription, t);
	}
	//we do have an abstract code block and it represents a statement list
	forEach(Token*, t, a->tokens, ai) {
		Statement* s;
		if ((s = parseStatement(&ai)) != nullptr)
			statements->add(s);
	}
	return statementsDeleter.release();
}
//parse a single statement starting at the current token
//includes expression statements, keyword statements and empty statements (which return null)
//assumes there is a token available
//parse location: the last token of the statement
//may throw
Statement* ParseExpressions::parseStatement(ArrayIterator<Token*>* ti) {
	DirectiveTitle* dt;
	if ((dt = dynamic_cast<DirectiveTitle*>(ti->getThis())) != nullptr) {
		//TODO: handle #enable stuff
		return nullptr;
	}
	Statement* keywordStatement = parseKeywordStatement(ti);
	return keywordStatement != nullptr ? keywordStatement : parseExpressionStatement(ti);
}
//parse a statement if the current token is a keyword identifier, return null if not
//parse location: no change | the last token of the keyword statement
//may throw
Statement* ParseExpressions::parseKeywordStatement(ArrayIterator<Token*>* ti) {
	Identifier* keywordToken;
	if ((keywordToken = dynamic_cast<Identifier*>(ti->getThis())) == nullptr)
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
		Deleter<Token> condition (completeParenthesizedExpression(conditionBlock, false));
		Deleter<Array<Statement*>> thenBody (parseStatementList(ti, conditionBlock, "if-statement body", true));
		Array<Statement*>* elseBody = nullptr;
		Token* elseToken = ti->getNext();
		Identifier* i;
		if (ti->hasThis() && (i = dynamic_cast<Identifier*>(elseToken)) != nullptr && i->name == elseKeyword)
			elseBody = parseStatementList(ti, i, "else-statement body", true);
		//go back if it's not an else keyword
		else
			ti->getPrevious();
		return new IfStatement(condition.release(), thenBody.release(), elseBody);
	} else if (keyword == forKeyword) {
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, keywordToken, "a for-loop condition");
		ArrayIterator<Token*> ai (conditionBlock->tokens);
		if (!ai.hasThis())
			Error::makeError(ErrorType::Expected, "a for-loop initialization", conditionBlock);
		Deleter<ExpressionStatement> initialization (parseExpressionStatement(&ai));
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
		Array<Statement*>* body = parseStatementList(ti, conditionBlock, "a for-loop body", true);
		return new LoopStatement(initialization.release(), condition.release(), incrementDeleter.release(), body, true);
	} else if (keyword == whileKeyword) {
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, keywordToken, "a condition expression");
		Deleter<Token> condition (completeParenthesizedExpression(conditionBlock, false));
		Array<Statement*>* body = parseStatementList(ti, conditionBlock, "a while-loop body", true);
		return new LoopStatement(nullptr, condition.release(), nullptr, body, true);
	} else if (keyword == doKeyword) {
		ArrayContentDeleter<Statement> body (parseStatementList(ti, keywordToken, "a do-while-loop body", true));
		Identifier* whileToken = parseExpectedToken<Identifier>(ti, ti->getThis(), "a \"while\" keyword");
		if (whileToken->name != whileKeyword)
			Error::makeError(ErrorType::Expected, "a \"while\" keyword", whileToken);
		AbstractCodeBlock* conditionBlock = parseExpectedToken<AbstractCodeBlock>(ti, whileToken, "a condition expression");
		Token* condition = completeParenthesizedExpression(conditionBlock, false);
		return new LoopStatement(nullptr, condition, nullptr, body.release(), false);
	} else if ((continueLoop = (keyword == continueKeyword)) || keyword == breakKeyword) {
		Token* t = parseExpectedToken<Token>(ti, keywordToken, "a semicolon or integer");
		IntConstant* levels;
		if ((levels = dynamic_cast<IntConstant*>(t)) == nullptr)
			ti->getPrevious();
		else
			ti->replaceThis(nullptr);
		Deleter<IntConstant> levelsDeleter (levels);
		Separator* s = parseExpectedToken<Separator>(ti, ti->getThis(), "a semicolon");
		if (s->separatorType != SeparatorType::Semicolon)
			Error::makeError(ErrorType::Expected, "a semicolon", s);
		return new LoopControlFlowStatement(continueLoop, levelsDeleter.release());
	} else
		return nullptr;
}
//parse a single expression statement starting at the current token, including any variable definitions
//returns null for empty statements
//parse location: the end semicolon/right parenthesis of the statement
//may throw
ExpressionStatement* ParseExpressions::parseExpressionStatement(ArrayIterator<Token*>* ti) {
	Token* t = ti->getThis();
	Separator* s;
	if ((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Semicolon)
		return nullptr;
	Identifier* i;
	CDataType* cdt;
	//if it's a type, see if it's a variable initialization vs a function definition
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr &&
		(cdt = CDataType::globalDataTypes->get(i->name.c_str(), i->name.length())) != nullptr)
	{
		t = ti->getNext();
		Identifier* variableName;
		//if a variable name followed, create a variable initialization
		if (ti->hasThis() && (variableName = dynamic_cast<Identifier*>(t)) != nullptr) {
			ti->replaceThis(nullptr);
			return new ExpressionStatement(
				completeVariableInitialization(i, cdt, variableName, ti, SeparatorType::Semicolon));
		//otherwise, go back so that we can parse an expression
		} else
			ti->getPrevious();
	}
	//if we get here then it's a plain, non-empty expression
	//we don't have to worry about an empty expression error message
	return new ExpressionStatement(
		parseExpression(ti, (unsigned char)SeparatorType::Semicolon, ErrorType::General, nullptr, nullptr));
	return nullptr;
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
