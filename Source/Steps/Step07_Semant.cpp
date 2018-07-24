#include "Project.h"

//verify that all types are correct, variable names match, etc.
//specific primitive byte sizes are not handled here

Semant::FindVisibleVariableDefinitionsVisitor::FindVisibleVariableDefinitionsVisitor(
	PrefixTrie<char, CVariableDefinition*>* pVariables, SourceFile* pOriginFile)
: TokenVisitor(onlyWhenTrackingIDs("FVDTVTR"))
, variables(pVariables)
, originFile(pOriginFile) {
}
Semant::FindVisibleVariableDefinitionsVisitor::~FindVisibleVariableDefinitionsVisitor() {
	//don't delete the variables, they're owned by something else
}
//search the token for variable initializations that will definitely happen and be visible to other expressions
void Semant::FindVisibleVariableDefinitionsVisitor::handleExpression(Token* t) {
	VariableDeclarationList* v;
	if (let(VariableDeclarationList*, v, t))
		addVariablesToTrie(v->variables, variables, originFile);
	else
		t->visitSubtokens(this);
}
//variable definitions in boolean right sides are not visible
bool Semant::FindVisibleVariableDefinitionsVisitor::shouldHandleBooleanRightSide() {
	return false;
}
Semant::SemantFileStatementsVisitor::SemantFileStatementsVisitor(PrefixTrie<char, CVariableDefinition*>* pVariables)
: TokenVisitor(onlyWhenTrackingIDs("SFSTVTR"))
, variables(pVariables) {
}
Semant::SemantFileStatementsVisitor::~SemantFileStatementsVisitor() {
	//don't delete the variables, they're owned by something else
}
//search the token for function definitions and semant their statements
void Semant::SemantFileStatementsVisitor::handleExpression(Token* t) {
	FunctionDefinition* fd;
	if (let(FunctionDefinition*, fd, t))
		semantFunctionDefinition(fd, variables);
	else
		t->visitSubtokens(this);
}
thread_local bool Semant::skipStatementsWhileFindingGlobalVariableDefinitions = true;
thread_local bool Semant::resemantingGenericTypeVariables = true;
//validate the semantics for all files
void Semant::semant(Pliers* pliers) {
	skipStatementsWhileFindingGlobalVariableDefinitions = true;
	resemantingGenericTypeVariables = true;

	//before we start, go through all the files and find what variables they declare
	forEach(SourceFile*, s, pliers->allFiles, si1) {
		PrefixTrie<char, CVariableDefinition*> variablesDeclaredInFile;
		forEach(Token*, t, s->globalVariables, ti) {
			FindVisibleVariableDefinitionsVisitor(&variablesDeclaredInFile, s).handleExpression(t);
		}
		s->variablesDeclaredInFile = variablesDeclaredInFile.getValues();
	}

	Array<Operator*> redoVariables;
	//now go through all files and give them an initial semant
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		//find all the variables visible to this file
		//add them in reverse order so that the deepest included file's variables get set first, and higher files will error
		Array<AVLNode<SourceFile*, bool>*>* allIncludedEntries = s->includedFiles->entrySet();
		for (int i = allIncludedEntries->length - 1; i >= 0; i--) {
			AVLNode<SourceFile* COMMA bool>* includedEntry = allIncludedEntries->get(i);
			addVariablesToTrie(includedEntry->key->variablesDeclaredInFile, s->variablesVisibleToFile, s);
		}
		delete allIncludedEntries;

		//then attempt to assign types to all global tokens (skipping function bodies)
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->path->fileName.c_str());
		semantFileDefinitions(s, s->variablesVisibleToFile, &redoVariables);
	}

	//next go through the redo variables and assign final types where possible
	//variables with generic-version types might be this way when we go to build, which is fine
	bool fixedVariable = true;
	while (fixedVariable) {
		fixedVariable = false;
		for (int i = redoVariables.length - 1; i >= 0; i--) {
			Operator* o = redoVariables.get(i);
			o->right = semantToken(o->right, o->owningFile->variablesVisibleToFile, SemantExpressionLevel::TopLevel);
			//check if we can find a specific version of the variable's type
			//if we don't have any generic versions, we don't need to check again
			if (finalizeTypes(dynamic_cast<VariableDeclarationList*>(o->left), o->right->dataType)) {
				redoVariables.remove(i);
				fixedVariable = true;
			}
		}
	}

	//go through any remaining redo variables and error if we tried to use a generic type token where we can't
	resemantingGenericTypeVariables = false;
	forEach(Operator*, o, &redoVariables, oi) {
		semantOperator(o, o->owningFile->variablesVisibleToFile, SemantExpressionLevel::TopLevel);
	}

	//and now go through and semant all the rest of the code
	skipStatementsWhileFindingGlobalVariableDefinitions = false;
	forEach(SourceFile*, s, pliers->allFiles, si3) {
		if (pliers->printProgress)
			printf("Analyzing semantics for statements in %s...\n", s->path->fileName.c_str());
		forEach(Token*, t, s->globalVariables, ti) {
			SemantFileStatementsVisitor(s->variablesVisibleToFile).handleExpression(t);
		}
	}
}
//add variables from the definition list into the trie
void Semant::addVariablesToTrie(
	Array<CVariableDefinition*>* v, PrefixTrie<char, CVariableDefinition*>* variables, SourceFile* originFile)
{
	forEach(CVariableDefinition*, c, v, ci) {
		CVariableDefinition* old = variables->set(c->name->name.c_str(), c->name->name.length(), c);
		if (old != nullptr) {
			//global variables may have already been added, so error only if we have a new definition for the same name
			//and don't error if we're re-adding variable definitions for an if condition, the error was already logged
			if (old == c)
				continue;
			variables->set(old->name->name.c_str(), old->name->name.length(), old);
			string errorMessage = "\"" + old->name->name + "\" has already been defined";
			logSemantErrorWithErrorSourceAndOriginFile(
				ErrorType::General, errorMessage.c_str(), c->name, old->name, originFile);
		}
	}
}
//use the provided type to turn generic types to specific types where possible
//returns whether all types are non-generic
bool Semant::finalizeTypes(VariableDeclarationList* v, CDataType* valueType) {
	bool allTypesAreKnown = true;
	//check if we can find a specific version of the variable's type
	//if we still have a generic version, we may need to check again
	//TODO: check for a Group valueType
	forEach(CVariableDefinition*, c, v->variables, ci) {
		if (c->type == CDataType::functionType) {
			if (istype(valueType, CSpecificFunction*)) {
				c->type = valueType;
				v->redetermineType();
			} else
				allTypesAreKnown = false;
		}
		//TODO: generic classes and groups
	}
	return allTypesAreKnown;
}
//validate the semantics of the global variables in the given file
//does not look at function body statements, since we need to finalize global variable types first
void Semant::semantFileDefinitions(
	SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, Array<Operator*>* redoVariables)
{
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o = nullptr;
		VariableDeclarationList* v;
		if (let(VariableDeclarationList*, v, t))
			continue;
		else if (!let(Operator*, o, t) || o->operatorType != OperatorType::Assign) {
			logSemantError(ErrorType::Expected, "a variable definition", t);
			continue;
		} else if (!let(VariableDeclarationList*, v, o->left)) {
			logSemantError(ErrorType::Expected, "a variable declaration list", o->left);
			continue;
		} else {
			semantOperator(o, variables, SemantExpressionLevel::TopLevel);
			//if we have an assignment without a specific type, we have to re-semant it
			if (!tokenHasKnownType(o))
				redoVariables->add(o);
		}
	}
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything under it is also valid
Token* Semant::semantToken(
	Token* t, PrefixTrie<char, CVariableDefinition*>* variables, SemantExpressionLevel semantExpressionLevel)
{
	Identifier* i;
	DirectiveTitle* d;
	ParenthesizedExpression* p;
	Cast* c;
	StaticOperator* s;
	Operator* o;
	FunctionCall* fc;
	FunctionDefinition* fd;
	Group* g;
	VariableDeclarationList* v;
	if (let(Identifier*, i, t))
		semantIdentifier(i, variables);
	else if (let(DirectiveTitle*, d, t))
		semantDirectiveTitle(d, variables);
	else if (let(ParenthesizedExpression*, p, t)) {
		t = semantToken(p->expression, variables, SemantExpressionLevel::TopLevelInParentheses);
		if (let(Operator*, o, t))
			o->wasParenthesized = true;
		p->expression = nullptr;
		delete p;
		return t;
	} else if (let(Operator*, o, t)) {
		if (let(Cast*, c, t))
			semantCast(c, variables);
		else if (let(StaticOperator*, s, t))
			semantStaticOperator(s, variables);
		else
			semantOperator(o, variables, semantExpressionLevel);
	} else if (let(FunctionCall*, fc, t))
		semantFunctionCall(fc, variables);
	else if (let(FunctionDefinition*, fd, t))
		semantFunctionDefinition(fd, variables);
	else if (let(Group*, g, t))
		semantGroup(g, variables);
	else if (istype(t, IntConstant*) || istype(t, FloatConstant*) || istype(t, BoolConstant*) || istype(t, StringLiteral*))
		;
	else if (let(VariableDeclarationList*, v, t)) {
		if (semantExpressionLevel != SemantExpressionLevel::TopLevel)
			logSemantError(ErrorType::General, "uninitialized variables must be declared on their own in a statement", v);
		addVariablesToTrie(v->variables, variables, nullptr);
	} else
		Error::logError(ErrorType::CompilerIssue, "determining the token type of this token", t);
	return t;
}
//give this identifier the right variable
void Semant::semantIdentifier(Identifier* i, PrefixTrie<char, CVariableDefinition*>* variables) {
	i->variable = variables->get(i->name.c_str(), i->name.length());
	if (i->variable == nullptr) {
		string errorMessage = "\"" + i->name + "\" has not been defined";
		logSemantError(ErrorType::General, errorMessage.c_str(), i);
		return;
	}
	i->dataType = i->variable->type;
}
//verify that this directive title ????????????????
void Semant::semantDirectiveTitle(DirectiveTitle* d, PrefixTrie<char, CVariableDefinition*>* variables) {
	Error::logError(ErrorType::CompilerIssue, "resulting in a directive title being used as an expression", d);
	assert(false); //TODO: do something with directive titles
	//TODO: ???????????
}
//verify that the sub-expression of this cast can actually be casted to the specified type
void Semant::semantCast(Cast* c, PrefixTrie<char, CVariableDefinition*>* variables) {
	c->right = semantToken(c->right, variables, SemantExpressionLevel::Subexpression);
	if (!tokenHasKnownType(c->right))
		return;
	bool canCast;
	//casting to a primitive type
	if (istype(c->castType, CPrimitive*))
		//this is only OK for other primitive types or raw casts
		canCast = istype(c->right->dataType, CPrimitive*) || c->isRaw;
	//any other cast must be to the same type
	//TODO: support pointer casting
	//TODO: allow raw casting from ints to pointers if those errors are turned off
	else
		canCast = c->castType == c->right->dataType;
	if (!canCast) {
		string errorMessage = "cannot cast expression of type " + c->right->dataType->name + " to type " + c->castType->name;
		logSemantError(ErrorType::General, errorMessage.c_str(), c);
		return;
	}
	c->dataType = c->castType;
}
//find the right variable for this static operator
void Semant::semantStaticOperator(StaticOperator* s, PrefixTrie<char, CVariableDefinition*>* variables) {
	Identifier* i;
	if (!let(Identifier*, i, s->right)) {
		logSemantError(ErrorType::ExpectedToFollow, "a member variable", s);
		return;
	}
	//TODO: currently only Main has support for member variables
	if (s->ownerType != CDataType::mainType) {
		logSemantError(ErrorType::General, "currently only Main has member variable support", s);
		return;
	}
	//TODO: support static member access
	if (s->operatorType != OperatorType::StaticDot) {
		logSemantError(ErrorType::General, "currently only the . member operator is supported", s);
		return;
	}
	if (i->name == "exit") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::intType);
		s->dataType = CGenericFunction::typeFor(CDataType::voidType, parameterTypes);
	} else if (i->name == "print") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::stringType);
		s->dataType = CGenericFunction::typeFor(CDataType::voidType, parameterTypes);
	} else if (i->name == "str") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::intType);
		s->dataType = CGenericFunction::typeFor(CDataType::stringType, parameterTypes);
	} else
		logSemantErrorWithErrorCheck(ErrorType::General, "currently Main only has support for exit, print, and str", i, s);
	//TODO: implement classes
}
//verify that this operator has the right types for its subexpressions
void Semant::semantOperator(
	Operator* o, PrefixTrie<char, CVariableDefinition*>* variables, SemantExpressionLevel semantExpressionLevel)
{
	//check which validations we need to perform
	OperatorSemanticsType operatorSemanticsType;
	switch (o->operatorType) {
		//single boolean non-mutating operators
		case OperatorType::LogicalNot:
			operatorSemanticsType = OperatorSemanticsType::SingleBoolean;
			break;
		//single integer non-mutating operators
		case OperatorType::BitwiseNot:
			operatorSemanticsType = OperatorSemanticsType::SingleInteger;
			break;
		//single number non-mutating operators
		case OperatorType::Negate:
			operatorSemanticsType = OperatorSemanticsType::SingleNumber;
			break;
		//boolean-boolean non-mutating operators
		case OperatorType::BooleanAnd:
		case OperatorType::BooleanOr:
			operatorSemanticsType = OperatorSemanticsType::BooleanBoolean;
			break;
		//integer-integer non-mutating operators, casts where appropriate
		case OperatorType::BitwiseAnd:
		case OperatorType::BitwiseXor:
		case OperatorType::BitwiseOr:
			operatorSemanticsType = OperatorSemanticsType::IntegerInteger;
			break;
		//integer-integer non-mutating bit shifts, no casting necessary
		case OperatorType::ShiftLeft:
		case OperatorType::ShiftRight:
		case OperatorType::ShiftArithmeticRight:
//		case OperatorType::RotateLeft:
//		case OperatorType::RotateRight:
			operatorSemanticsType = OperatorSemanticsType::IntegerIntegerBitShift;
			break;
		//number-number non-mutating operators, casts where appropriate
		case OperatorType::Multiply:
		case OperatorType::Divide:
		case OperatorType::Modulus:
		case OperatorType::Subtract:
			operatorSemanticsType = OperatorSemanticsType::NumberNumber;
			break;
		//same as above but this one can be number-number or string-string
		case OperatorType::Add:
			operatorSemanticsType = OperatorSemanticsType::NumberNumberOrStringString;
			break;
		//number-number non-mutating comparison operators, casts where appropriate
		case OperatorType::LessOrEqual:
		case OperatorType::GreaterOrEqual:
		case OperatorType::LessThan:
		case OperatorType::GreaterThan:
			operatorSemanticsType = OperatorSemanticsType::NumberNumber;
			break;
		//any-any non-mutating comparison operators, two types where one can be implicitly casted to the other
		case OperatorType::Equal:
		case OperatorType::NotEqual:
			operatorSemanticsType = OperatorSemanticsType::AnyAny;
			break;
		//non-mutating ternary operator, make sure the left side is a boolean and the right side is a colon with two types
		//	that share a common ancestor type
		case OperatorType::QuestionMark:
			operatorSemanticsType = OperatorSemanticsType::Ternary;
			break;
		//single boolean mutating operators
		case OperatorType::VariableLogicalNot:
			operatorSemanticsType = OperatorSemanticsType::SingleBoolean;
			break;
		//single integer mutating operators
		case OperatorType::Increment:
		case OperatorType::Decrement:
		case OperatorType::VariableBitwiseNot:
			operatorSemanticsType = OperatorSemanticsType::SingleInteger;
			break;
		//single number mutating operators
		case OperatorType::VariableNegate:
			operatorSemanticsType = OperatorSemanticsType::SingleNumber;
			break;
		//boolean-boolean mutating operators
//		case OperatorType::AssignBooleanAnd:
//		case OperatorType::AssignBooleanOr:
//			operatorSemanticsType = OperatorSemanticsType::BooleanBoolean;
//			assignment = true;
//			break;
		//integer-integer mutating operators, casts where appropriate
		case OperatorType::AssignBitwiseAnd:
		case OperatorType::AssignBitwiseXor:
		case OperatorType::AssignBitwiseOr:
			operatorSemanticsType = OperatorSemanticsType::IntegerInteger;
			break;
		//integer-integer mutating bit shifts, no casting necessary
		case OperatorType::AssignShiftLeft:
		case OperatorType::AssignShiftRight:
		case OperatorType::AssignShiftArithmeticRight:
//		case OperatorType::AssignRotateLeft:
//		case OperatorType::AssignRotateRight:
			operatorSemanticsType = OperatorSemanticsType::IntegerIntegerBitShift;
			break;
		//number-number mutating operators, casts where appropriate
		case OperatorType::AssignSubtract:
		case OperatorType::AssignMultiply:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
			operatorSemanticsType = OperatorSemanticsType::NumberNumber;
			break;
		//same as above but this one can be number-number or string-string
		case OperatorType::AssignAdd:
			operatorSemanticsType = OperatorSemanticsType::NumberNumberOrStringString;
			break;
		//any-any mutating operators, two types where the right can be implicitly casted to the left
		case OperatorType::Assign:
			operatorSemanticsType = OperatorSemanticsType::AnyAny;
			break;
		//this one isn't supposed to be handled on its own
		case OperatorType::Colon:
			logSemantError(ErrorType::General, "colon missing question mark in ternary operator", o);
			return;
		//TODO: classes
		case OperatorType::Dot:
		case OperatorType::ObjectMemberAccess:
			logSemantError(ErrorType::General, "member variables are currently not supported", o);
			return;
		//these should have already been handled
		case OperatorType::None:
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
		case OperatorType::Cast:
		default:
			Error::logError(ErrorType::CompilerIssue, "determining the operator semantics for this operator", o);
			return;
	}
	//rewrite prefix operators to be postfix
	if (o->left == nullptr) {
		o->left = o->right;
		o->right = nullptr;
		if (o->left == nullptr) {
			Error::logError(ErrorType::CompilerIssue, "resulting in an operator without operands", o);
			return;
		}
	}
	//semant the inner tokens
	//stop if either one has an unknown type
	if (o->operatorType != OperatorType::Assign) {
		o->left = semantToken(o->left, variables, SemantExpressionLevel::Subexpression);
		if (!tokenHasKnownType(o->left))
			return;
	}
	if (operatorSemanticsType == OperatorSemanticsType::SingleBoolean
		|| operatorSemanticsType == OperatorSemanticsType::SingleInteger
		|| operatorSemanticsType == OperatorSemanticsType::SingleNumber)
	{
		if (o->right != nullptr) {
			Error::logError(ErrorType::CompilerIssue, "resulting in a unary operator with an extra operand", o);
			return;
		}
	} else if (o->right == nullptr) {
		Error::logError(ErrorType::CompilerIssue, "resulting in a binary operator missing an operand", o);
		return;
	} else if (operatorSemanticsType != OperatorSemanticsType::Ternary) {
		//any variables that appear on the right side of booleans are unavailable outside of it
		if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
			PrefixTrieUnion<char, CVariableDefinition*> booleanOperatorVariables (variables);
			o->right = semantToken(o->right, &booleanOperatorVariables, SemantExpressionLevel::Subexpression);
		} else
			o->right = semantToken(o->right, variables, SemantExpressionLevel::Subexpression);
		if (!tokenHasKnownType(o->right))
			return;
	}
	//now make the specific operator checks
	//copper: switch with gotos
	Operator* ternaryResult;
	CDataType* ternaryResultType;
	switch (operatorSemanticsType) {
		case OperatorSemanticsType::BooleanBoolean:
			if (o->right->dataType != CDataType::boolType) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a boolean value", o->right, o);
				return;
			}
		case OperatorSemanticsType::SingleBoolean:
			if (o->left->dataType != CDataType::boolType) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a boolean value", o->left, o);
				return;
			}
			break;
		case OperatorSemanticsType::IntegerInteger:
		case OperatorSemanticsType::IntegerIntegerBitShift:
			if (!istype(o->right->dataType, CIntegerPrimitive*)) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "an integer value", o->right, o);
				return;
			}
		case OperatorSemanticsType::SingleInteger:
			if (!istype(o->left->dataType, CIntegerPrimitive*)) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "an integer value", o->left, o);
				return;
			}
			break;
		case OperatorSemanticsType::NumberNumber:
			if (!istype(o->right->dataType, CNumericPrimitive*)) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a numeric value", o->right, o);
				return;
			}
		case OperatorSemanticsType::SingleNumber:
			if (!istype(o->left->dataType, CNumericPrimitive*)) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a numeric value", o->left, o);
				return;
			}
			break;
		case OperatorSemanticsType::NumberNumberOrStringString:
			if (istype(o->left->dataType, CNumericPrimitive*)) {
				if (!istype(o->right->dataType, CNumericPrimitive*)) {
					logSemantErrorWithErrorCheck(ErrorType::Expected, "a numeric value", o->right, o);
					return;
				}
			} else if (o->left->dataType == CDataType::stringType) {
				if (o->right->dataType != CDataType::stringType) {
					logSemantErrorWithErrorCheck(ErrorType::Expected, "a String value", o->right, o);
					return;
				}
			} else {
				if (istype(o->right->dataType, CNumericPrimitive*))
					logSemantErrorWithErrorCheck(ErrorType::Expected, "a numeric value", o->left, o);
				else if (o->right->dataType == CDataType::stringType)
					logSemantErrorWithErrorCheck(ErrorType::Expected, "a String value", o->left, o);
				else
					logSemantError(ErrorType::Expected, "a pair of numeric or String values", o);
				return;
			}
			break;
		case OperatorSemanticsType::AnyAny:
			//this is either a comparison or an assignment, those will be handled separately
			break;
		case OperatorSemanticsType::Ternary: {
			if (o->left->dataType != CDataType::boolType) {
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a boolean value", o->left, o);
				return;
			}
			if (!let(Operator*, ternaryResult, o->right) || ternaryResult->operatorType != OperatorType::Colon) {
				logSemantError(ErrorType::General, "question mark missing colon in ternary operator", o);
				return;
			}
			PrefixTrieUnion<char, CVariableDefinition*> leftVariables (variables);
			PrefixTrieUnion<char, CVariableDefinition*> rightVariables (variables);
			ternaryResult->left = semantToken(ternaryResult->left, &leftVariables, SemantExpressionLevel::Subexpression);
			ternaryResult->right = semantToken(ternaryResult->right, &rightVariables, SemantExpressionLevel::Subexpression);
			if (!tokenHasKnownType(ternaryResult->left) || !tokenHasKnownType(ternaryResult->right))
				return;
			ternaryResultType = CDataType::bestCompatibleType(ternaryResult->left->dataType, ternaryResult->right->dataType);
			if (ternaryResultType == nullptr) {
				string errorMessage = "ternary operator results in mismatching types " + ternaryResult->left->dataType->name
					+ " and " + ternaryResult->right->dataType->name;
				logSemantError(ErrorType::General, errorMessage.c_str(), o);
				return;
			}
			o->dataType = ternaryResultType;
			return;
		}
		default:
			Error::logError(ErrorType::CompilerIssue, "validating the operator semantics for this operator", o);
			return;
	}

	//finally, assign the operator its type
	//for assignments, also check that the assignment is valid
	if (o->operatorType == OperatorType::Assign) {
		//make sure it's something we can assign to
		Identifier* i;
		VariableDeclarationList* v = nullptr;
		if (let(Identifier*, i, o->left)) {
			semantIdentifier(i, variables);
			if (!tokenHasKnownType(i))
				return;
		} else if (let(VariableDeclarationList*, v, o->left)) {
			addVariablesToTrie(v->variables, variables, nullptr);
			if (!finalizeTypes(v, o->right->dataType))
				return;
			//TODO: Groups - each variable gets a different initial value, also auto grouping and ungrouping
			forEach(CVariableDefinition*, vd, v->variables, vdi) {
				vd->initialValue = o->right;
			}
		} else {
			logSemantErrorWithErrorCheck(ErrorType::Expected, "a variable or variable list", o->left, o);
			return;
		}
		//make sure that either the type matches,
		//	or that it's a multiple-variable-definition-list with all the same type that accepts the right type
		if (CDataType::bestCompatibleType(o->left->dataType, o->right->dataType) != o->left->dataType) {
			bool singleTypeVariableDeclarationList = false;
			if (v != nullptr && v->variables->length > 1) {
				CSpecificGroup* groupType;
				if (!let(CSpecificGroup*, groupType, v->dataType)) {
					Error::logError(ErrorType::CompilerIssue, "determining the types of these variables", v);
					return;
				} else if (groupType->allSameType) {
					CDataType* groupInnerType = groupType->types->get(0)->type;
					if (CDataType::bestCompatibleType(groupInnerType, o->right->dataType) == groupInnerType)
						singleTypeVariableDeclarationList = true;
				}
			}
			if (!singleTypeVariableDeclarationList) {
				string errorMessage = "cannot use a value of type " + o->right->dataType->name
					+ " to assign to a variable of type " + o->left->dataType->name;
				logSemantError(ErrorType::General, errorMessage.c_str(), o);
				return;
			}
		}
		if (semantExpressionLevel == SemantExpressionLevel::Subexpression && !o->wasParenthesized) {
			logSemantError(
				ErrorType::General, "assignment operators must be parenthesized or be the root operator of a statement", o);
			return;
		}
		//TODO: Groups - figure out automatic grouping/ungrouping
		o->dataType = o->left->dataType;
	//check that comparisons are valid- as long as one is castable to the other we're good
	} else if (o->precedence == OperatorTypePrecedence::Comparison) {
		if (CDataType::bestCompatibleType(o->left->dataType, o->right->dataType) == nullptr) {
			string errorMessage = "cannot compare values of type " + o->left->dataType->name
				+ " and " + o->right->dataType->name;
			logSemantError(ErrorType::General, errorMessage.c_str(), o);
			return;
		}
		o->dataType = CDataType::boolType;
	//check that we have a variable to modify
	} else if (o->modifiesVariable) {
		if (!istype(o->left, Identifier*)) {
			logSemantErrorWithErrorCheck(ErrorType::Expected, "a variable", o->left, o);
			return;
		}
		o->dataType = o->left->dataType;
	//bitshifts result in the left type regardless what the right type is
	} else if (o->right == nullptr || operatorSemanticsType == OperatorSemanticsType::IntegerIntegerBitShift)
		o->dataType = o->left->dataType;
	//and everything else results in the best compatible type between the two of them
	else
		o->dataType = CDataType::bestCompatibleType(o->left->dataType, o->right->dataType);
}
//verify that this function call has the right function and argument types
void Semant::semantFunctionCall(FunctionCall* f, PrefixTrie<char, CVariableDefinition*>* variables) {
	//make sure we have the right function type
	f->function = semantToken(f->function, variables, SemantExpressionLevel::Subexpression);
	CSpecificFunction* functionType = dynamic_cast<CSpecificFunction*>(f->function->dataType);
	if (functionType == nullptr) {
		if (tokenHasKnownType(f->function)) {
			if (f->function->dataType != CDataType::functionType)
				logSemantErrorWithErrorCheck(ErrorType::Expected, "a function value", f->function, f);
			else if (!resemantingGenericTypeVariables)
				logSemantErrorWithErrorCheck(ErrorType::General, "cannot determine function signature", f->function, f);
		}
		return;
	}
	//make sure the arguments match
	if (functionType->parameterTypes->length != f->arguments->length) {
		string errorMessage = "expected " + std::to_string(functionType->parameterTypes->length)
			+ " arguments but got " + std::to_string(f->arguments->length);
		logSemantError(ErrorType::General, errorMessage.c_str(), f);
		return;
	}
	int argumentCount = Math::min(functionType->parameterTypes->length, f->arguments->length);
	bool argumentTypesAreKnown = true;
	for (int i = 0; i < argumentCount; i++) {
		Token* t = semantToken(f->arguments->get(i), variables, SemantExpressionLevel::TopLevelInParentheses);
		if (tokenHasKnownType(t)) {
			CDataType* expectedType = functionType->parameterTypes->get(i);
			if (CDataType::bestCompatibleType(expectedType, t->dataType) != expectedType) {
				string errorMessage = "expected a value of type " + expectedType->name + " but got " + t->dataType->name;
				logSemantErrorWithErrorCheck(ErrorType::General, errorMessage.c_str(), t, f);
			}
		} else
			argumentTypesAreKnown = false;
		f->arguments->set(i, t);
	}
	if (argumentTypesAreKnown)
		f->dataType = functionType->returnType;
}
//check all the statements of this function definition
void Semant::semantFunctionDefinition(FunctionDefinition* f, PrefixTrie<char, CVariableDefinition*>* variables) {
	if (skipStatementsWhileFindingGlobalVariableDefinitions)
		return;
	if (semantStatementList(f->body, variables, f->parameters, f->returnType) != ScopeExitType::Return
			&& f->returnType != CDataType::voidType)
		Error::logError(ErrorType::General, "not all paths return a value", f);
}
//verify that this group ????????????????
void Semant::semantGroup(Group* g, PrefixTrie<char, CVariableDefinition*>* variables) {
	Error::logError(ErrorType::CompilerIssue, "resulting in a Group appearing an expression", g);
	assert(false); //TODO: Groups
	//TODO: ???????????
}
//semant all the statments in the given statement list
//functions will pass a return type, to be recursively passed for other statement lists
//returns the strongest exit that definitely happens in the statement list
ScopeExitType Semant::semantStatementList(
	Array<Statement*>* statements,
	PrefixTrie<char, CVariableDefinition*>* previousVariables,
	Array<CVariableDefinition*>* functionParameters,
	CDataType* returnType)
{
	PrefixTrieUnion<char, CVariableDefinition*> variables (previousVariables);
	if (functionParameters != nullptr)
		addVariablesToTrie(functionParameters, &variables, nullptr);
	ScopeExitType exitType = ScopeExitType::None;
	forEach(Statement*, s, statements, si) {
		ExpressionStatement* e;
		ReturnStatement* r;
		IfStatement* i;
		LoopStatement* l;
		if (let(ExpressionStatement*, e, s))
			e->expression = semantToken(e->expression, &variables, SemantExpressionLevel::TopLevel);
		else if (let(ReturnStatement*, r, s)) {
			if (r->expression != nullptr) {
				r->expression = semantToken(r->expression, &variables, SemantExpressionLevel::TopLevel);
				if (CDataType::bestCompatibleType(r->expression->dataType, returnType) != returnType) {
					string errorMessage = "cannot return type " + r->expression->dataType->name
						+ " in a function that expects type " + returnType->name;
					Error::logError(ErrorType::General, errorMessage.c_str(), r->expression);
				}
			} else if (returnType != CDataType::voidType) {
				string errorMessage = "a returned value of type " + returnType->name;
				Error::logError(ErrorType::Expected, errorMessage.c_str(), r->returnKeywordToken);
			}
			exitType = ScopeExitType::Return;
		} else if (let(IfStatement*, i, s)) {
			i->condition = semantToken(i->condition, &variables, SemantExpressionLevel::TopLevelInParentheses);
			if (tokenHasKnownType(i->condition) && i->condition->dataType != CDataType::boolType)
				Error::logError(ErrorType::Expected, "a bool value", i->condition);
			PrefixTrieUnion<char, CVariableDefinition*> thenBodyVariables (&variables);
			PrefixTrieUnion<char, CVariableDefinition*> elseBodyVariables (&variables);
			//find any possible variable definitions from within the condition and apply them where possible
			PrefixTrieUnion<char, CVariableDefinition*> newScopeVariables (&variables);
			IfStatement::ConditionVisitor conditionVisitor (
				&newScopeVariables, new FindVisibleVariableDefinitionsVisitor(&newScopeVariables, nullptr));
			conditionVisitor.handleExpression(i->condition);
			Array<CVariableDefinition*>* newScopeVariablesList =
				newScopeVariables.isEmpty() ? nullptr : newScopeVariables.getValues();
			if (newScopeVariablesList != nullptr) {
				if (conditionVisitor.conditionBooleanType == OperatorType::BooleanAnd)
					addVariablesToTrie(newScopeVariablesList, &thenBodyVariables, nullptr);
				else if (conditionVisitor.conditionBooleanType == OperatorType::BooleanOr)
					addVariablesToTrie(newScopeVariablesList, &elseBodyVariables, nullptr);
			}
			//semant the bodies with our new variable tries
			ScopeExitType thenExitType = semantStatementList(i->thenBody, &thenBodyVariables, nullptr, returnType);
			ScopeExitType elseExitType = i->elseBody != nullptr
				? semantStatementList(i->elseBody, &elseBodyVariables, nullptr, returnType)
				: ScopeExitType::None;
			//if our extra variables are definitely visible after the if statement, add them to the trie
			if (newScopeVariablesList != nullptr
					&& ((conditionVisitor.conditionBooleanType == OperatorType::BooleanOr
							&& thenExitType != ScopeExitType::None)
						|| (conditionVisitor.conditionBooleanType == OperatorType::BooleanAnd
							&& elseExitType != ScopeExitType::None)))
				addVariablesToTrie(newScopeVariablesList, &variables, nullptr);
			ScopeExitType ifExitType =
				((unsigned char)elseExitType) < ((unsigned char)thenExitType) ? elseExitType : thenExitType;
			if ((unsigned char)exitType < (unsigned char)ifExitType)
				exitType = ifExitType;
			delete newScopeVariablesList;
		} else if (let(LoopStatement*, l, s)) {
			PrefixTrieUnion<char, CVariableDefinition*> loopBodyVariables (&variables);
			PrefixTrieUnion<char, CVariableDefinition*> conditionVariables (&loopBodyVariables);
			PrefixTrieUnion<char, CVariableDefinition*> incrementVariables (&loopBodyVariables);
			if (l->initialization != nullptr)
				l->initialization->expression =
					semantToken(l->initialization->expression, &loopBodyVariables, SemantExpressionLevel::TopLevel);
			l->condition = semantToken(l->condition, &conditionVariables, SemantExpressionLevel::TopLevel);
			if (tokenHasKnownType(l->condition) && l->condition->dataType != CDataType::boolType)
				Error::logError(ErrorType::Expected, "a bool value", l->condition);
			if (l->increment != nullptr)
				l->increment = semantToken(l->increment, &incrementVariables, SemantExpressionLevel::TopLevel);
			ScopeExitType loopExitType = semantStatementList(l->body, &loopBodyVariables, nullptr, returnType);
			if (loopExitType == ScopeExitType::Return)
				exitType = ScopeExitType::Return;
		} else if (istype(s, LoopControlFlowStatement*)) {
			if ((unsigned char)exitType < (unsigned char)ScopeExitType::LoopJump)
				exitType = ScopeExitType::LoopJump;
		}
	}
	return exitType;
}
//returns true if we don't need to re-semant this token
bool Semant::tokenHasKnownType(Token* t) {
	return t->dataType != nullptr
		&& t->dataType != CDataType::errorType
		&& (!resemantingGenericTypeVariables || !istype(t->dataType, CGenericPointerType*));
}
//log an error and give the token an error type, unless we've already done so
void Semant::logSemantError(ErrorType type, const char* message, Token* token) {
	logSemantErrorWithErrorCheck(type, message, token, token);
}
//log an error and give the token an error type, unless we've already done so
void Semant::logSemantErrorWithErrorSourceAndOriginFile(
	ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile)
{
	logSemantErrorWithErrorSourceAndOriginFileWithErrorCheck(type, message, token, errorSource, errorOriginFile, token);
}
//log an error and give the token an error type, unless we've already done so
void Semant::logSemantErrorWithErrorCheck(ErrorType type, const char* message, Token* token, Token* errorCheck) {
	logSemantErrorWithErrorSourceAndOriginFileWithErrorCheck(type, message, token, nullptr, nullptr, errorCheck);
}
//log an error and give the token an error type, unless we've already done so
void Semant::logSemantErrorWithErrorSourceAndOriginFileWithErrorCheck(
	ErrorType type, const char* message, Token* token, Token* errorSource, SourceFile* errorOriginFile, Token* errorCheck)
{
	if (errorCheck->dataType == CDataType::errorType)
		return;
	Error::logErrorWithErrorSourceAndOriginFile(type, message, token, errorSource, errorOriginFile);
	errorCheck->dataType = CDataType::errorType;
}
