#include "Project.h"

//verify that all types are correct, variable names match, etc.
//specific primitive byte sizes are not handled here

thread_local bool Semant::shouldSemantStatements = false;
//validate the semantics for all files
void Semant::semant(Pliers* pliers) {
	//start by gathering the trie of all global variables, as well as which ones are initialized
	if (pliers->printProgress)
		printf("Gathering all global variables...\n");
	PrefixTrie<char, CVariableDefinition*> allGlobalVariables;
	forEach(SourceFile*, s, pliers->allFiles, si1) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			VariableDeclarationList* v;
			if ((v = dynamic_cast<VariableDeclarationList*>(t)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
			else if ((o = dynamic_cast<Operator*>(t)) != nullptr &&
					(v = dynamic_cast<VariableDeclarationList*>(o->left)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
		}
	}

	//then go through all the files to find global variables
	Array<Operator*> redoVariables;
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileDefinitions(s, &allGlobalVariables, &redoVariables);
	}

	//TODO: ?????????????????????????

	//next go through the redo variables and assign final types where possible
	//variables with generic-version types might be this way when we go to build, which is fine
	bool fixedVariable = true;
	while (fixedVariable) {
		fixedVariable = false;
		for (int i = redoVariables.length - 1; i >= 0; i--) {
			bool needsRedo = false;
			Operator* o = redoVariables.get(i);
			semantToken(o->right, &allGlobalVariables, SemantExpressionLevel::TopLevel);
			VariableDeclarationList* v = dynamic_cast<VariableDeclarationList*>(o->left);
			//check if we can find a specific version of the variable's type
			//if we still have a generic version, we may need to check again
			forEach(CVariableDefinition*, c, v->variables, ci) {
				if (c->type == CDataType::functionType) {
					if (dynamic_cast<CSpecificFunction*>(o->right->dataType) != nullptr)
						c->type = o->right->dataType;
					else
						needsRedo = true;
				}
				//TODO: generic classes and groups
			}
			if (!needsRedo) {
				redoVariables.remove(i);
				fixedVariable = true;
			}
		}
	}

	//TODO: ?????????????????????????

	//and now go through and semant all the rest of the code
	forEach(SourceFile*, s, pliers->allFiles, si3) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileStatements(s, &allGlobalVariables);
	}

	//TODO: ?????????????????????????
}
//add variables from the definition list into the trie
void Semant::addVariablesToTrie(VariableDeclarationList* v, PrefixTrie<char, CVariableDefinition*>* variables) {
	forEach(CVariableDefinition*, c, v->variables, ci) {
		CVariableDefinition* old = variables->set(c->name->name.c_str(), c->name->name.length(), c);
		if (old != nullptr) {
			variables->set(old->name->name.c_str(), old->name->name.length(), old);
			string errorMessage = "\"" + old->name->name + "\" has already been defined";
			Error::logError(ErrorType::General, errorMessage.c_str(), c->name);
		}
	}
}
//validate the semantics of the global variables in the given file
//does not look at function body statements, since we need to finalize global variable types first
void Semant::semantFileDefinitions(
	SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, Array<Operator*>* redoVariables)
{
	shouldSemantStatements = false;
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o = nullptr;
		VariableDeclarationList* v;
		if ((v = dynamic_cast<VariableDeclarationList*>(t)) != nullptr)
			;
		else if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
			Error::logError(ErrorType::Expected, "a variable definition", t);
			continue;
		} else if ((v = dynamic_cast<VariableDeclarationList*>(o->left)) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable declaration list", o->left);
			continue;
		} else
			semantToken(o, variables, SemantExpressionLevel::TopLevel);

		//if we have an assignment without a specific type, we have to re-semant it
		if (o != nullptr && !tokenHasKnownType(o->right))
			redoVariables->add(o);
	}
}
//????????????????????????????
void Semant::semantFileStatements(SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables) {
	shouldSemantStatements = true;
	//??????????????????????????????????
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything under it is also valid
void Semant::semantToken(
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
	if ((i = dynamic_cast<Identifier*>(t)) != nullptr)
		semantIdentifier(i, variables);
	else if ((d = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
		semantDirectiveTitle(d, variables);
	else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr)
		semantToken(p->expression, variables, SemantExpressionLevel::TopLevelInParentheses);
	else if ((c = dynamic_cast<Cast*>(t)) != nullptr)
		semantCast(c, variables);
	else if ((s = dynamic_cast<StaticOperator*>(t)) != nullptr)
		semantStaticOperator(s, variables);
	else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		semantOperator(o, variables, semantExpressionLevel);
	else if ((fc = dynamic_cast<FunctionCall*>(t)) != nullptr)
		semantFunctionCall(fc, variables);
	else if ((fd = dynamic_cast<FunctionDefinition*>(t)) != nullptr)
		semantFunctionDefinition(fd, variables);
	else if ((g = dynamic_cast<Group*>(t)) != nullptr)
		semantGroup(g, variables);
	else if (dynamic_cast<IntConstant*>(t) != nullptr ||
			dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<BoolConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr)
		;
	else if (dynamic_cast<VariableDeclarationList*>(t) != nullptr) {
		if (semantExpressionLevel != SemantExpressionLevel::TopLevel)
			Error::logError(ErrorType::Expected, "a value", t);
		;
	} else {
		Error::logError(ErrorType::CompilerIssue, "determining the token type of this token", t);
		assert(false);
	}
}
//give this identifier the right variable
void Semant::semantIdentifier(Identifier* i, PrefixTrie<char, CVariableDefinition*>* variables) {
	i->variable = variables->get(i->name.c_str(), i->name.length());
	if (i->variable == nullptr) {
		string errorMessage = "\"" + i->name + "\" has not been defined";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
	i->dataType = i->variable->type;
}
//verify that this directive title ????????????????
void Semant::semantDirectiveTitle(DirectiveTitle* d, PrefixTrie<char, CVariableDefinition*>* variables) {
	Error::logError(ErrorType::CompilerIssue, "a directive title being used as an expression", d);
	assert(false); //TODO: do something with directive titles
	//TODO: ???????????
}
//verify that the sub-expression of this cast can actually be casted to the specified type
void Semant::semantCast(Cast* c, PrefixTrie<char, CVariableDefinition*>* variables) {
	semantToken(c->right, variables, SemantExpressionLevel::Subexpression);
	if (!tokenHasKnownType(c->right))
		return;
	bool canCast;
	//casting to a primitive type
	if (dynamic_cast<CPrimitive*>(c->dataType) != nullptr)
		//this is only OK for other primitive types or raw casts
		canCast = dynamic_cast<CPrimitive*>(c->right->dataType) != nullptr || c->isRaw;
	//any other cast must be to the same type
	//TODO: support class casting
	//TODO: allow raw casting from ints to pointers if those errors are turned off
	else
		canCast = c->dataType == c->right->dataType;
	if (!canCast) {
		string errorMessage = "cannot cast expression of type " + c->right->dataType->name + " to type " + c->dataType->name;
		Error::logError(ErrorType::General, errorMessage.c_str(), c);
	}
}
//find the right variable for this static operator
void Semant::semantStaticOperator(StaticOperator* s, PrefixTrie<char, CVariableDefinition*>* variables) {
	Identifier* i;
	if ((i = dynamic_cast<Identifier*>(s->right)) == nullptr) {
		Error::logError(ErrorType::ExpectedToFollow, "a member variable", s);
		return;
	}
	//TODO: currently only Main has support for member variables
	if (s->ownerType != CDataType::mainType) {
		Error::logError(ErrorType::General, "currently only Main has member variable support", s);
		return;
	}
	if (i->name == "str") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::intType);
		s->dataType = CGenericFunction::typeFor(CDataType::stringType, parameterTypes);
	} else if (i->name == "print") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::stringType);
		s->dataType = CGenericFunction::typeFor(CDataType::voidType, parameterTypes);
	} else if (i->name == "exit") {
		Array<CDataType*>* parameterTypes = new Array<CDataType*>();
		parameterTypes->add(CDataType::intType);
		s->dataType = CGenericFunction::typeFor(CDataType::voidType, parameterTypes);
	} else
		Error::logError(ErrorType::General, "currently Main only has support for print, str, and exit", i);
	//TODO: implement classes
}
//verify that this operator has the right types for its subexpressions
void Semant::semantOperator(
	Operator* o, PrefixTrie<char, CVariableDefinition*>* variables, SemantExpressionLevel semantExpressionLevel)
{
	//check which validations we need to perform
	OperatorSemanticsType operatorSemanticsType;
	bool comparisonOperator = false;
	bool assignment = false;
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
			comparisonOperator = true;
			break;
		//any-any non-mutating comparison operators, two types where one can be implicitly casted to the other
		case OperatorType::Equal:
		case OperatorType::NotEqual:
			operatorSemanticsType = OperatorSemanticsType::AnyAny;
			comparisonOperator = true;
			break;
		//non-mutating ternary operator, make sure the left side is a boolean and the right side is a colon with two types
		//	that share a common ancestor type
		case OperatorType::QuestionMark:
			operatorSemanticsType = OperatorSemanticsType::Ternary;
			break;
		//single boolean mutating operators
		case OperatorType::VariableLogicalNot:
			operatorSemanticsType = OperatorSemanticsType::SingleBoolean;
			assignment = true;
			break;
		//single integer mutating operators
		case OperatorType::Increment:
		case OperatorType::Decrement:
		case OperatorType::VariableBitwiseNot:
			operatorSemanticsType = OperatorSemanticsType::SingleInteger;
			assignment = true;
			break;
		//single number mutating operators
		case OperatorType::VariableNegate:
			operatorSemanticsType = OperatorSemanticsType::SingleNumber;
			assignment = true;
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
			assignment = true;
			break;
		//integer-integer mutating bit shifts, no casting necessary
		case OperatorType::AssignShiftLeft:
		case OperatorType::AssignShiftRight:
		case OperatorType::AssignShiftArithmeticRight:
//		case OperatorType::AssignRotateLeft:
//		case OperatorType::AssignRotateRight:
			operatorSemanticsType = OperatorSemanticsType::IntegerIntegerBitShift;
			assignment = true;
			break;
		//number-number mutating operators, casts where appropriate
		case OperatorType::AssignSubtract:
		case OperatorType::AssignMultiply:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
			operatorSemanticsType = OperatorSemanticsType::NumberNumber;
			assignment = true;
			break;
		//same as above but this one can be number-number or string-string
		case OperatorType::AssignAdd:
			operatorSemanticsType = OperatorSemanticsType::NumberNumberOrStringString;
			assignment = true;
			break;
		//any-any mutating operators, two types where the right can be implicitly casted to the left
		case OperatorType::Assign:
			operatorSemanticsType = OperatorSemanticsType::AnyAny;
			assignment = true;
			break;
		//this one isn't supposed to be handled on its own
		case OperatorType::Colon:
			Error::logError(ErrorType::General, "colon missing question mark in ternary operator", o);
			return;
		//TODO: classes
		case OperatorType::Dot:
		case OperatorType::ObjectMemberAccess:
			Error::logError(ErrorType::General, "member variables are currently not supported", o);
			return;
		//these should have already been handled
//		case OperatorType::None:
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
		case OperatorType::Cast:
		default:
			Error::logError(ErrorType::CompilerIssue, "determining the operator semantics for this operator", o);
			assert(false);
			return;
	}
	//rewrite prefix operators to be postfix
	if (o->left == nullptr) {
		o->left = o->right;
		o->right = nullptr;
	}
	//semant the inner tokens
	//stop if either one has an unknown type
	if (o->left == nullptr) {
		Error::logError(ErrorType::CompilerIssue, "resulting in an operator without operands", o);
		assert(false);
		return;
	} else if (!assignment) {
		semantToken(o->left, variables, SemantExpressionLevel::Subexpression);
		if (!tokenHasKnownType(o->left))
			return;
	}
	if ((o->right == nullptr) !=
		(operatorSemanticsType == OperatorSemanticsType::SingleBoolean ||
		operatorSemanticsType == OperatorSemanticsType::SingleInteger ||
		operatorSemanticsType == OperatorSemanticsType::SingleNumber))
	{
		if (o->right == nullptr)
			Error::logError(ErrorType::CompilerIssue, "resulting in a binary operator missing an operand", o);
		else
			Error::logError(ErrorType::CompilerIssue, "resulting in a unary operator with an extra operand", o);
		assert(false);
		return;
	} else if (operatorSemanticsType != OperatorSemanticsType::Ternary) {
		semantToken(o->right, variables, SemantExpressionLevel::Subexpression);
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
				Error::logError(ErrorType::Expected, "a boolean value", o->right);
				return;
			}
		case OperatorSemanticsType::SingleBoolean:
			if (o->left->dataType != CDataType::boolType) {
				Error::logError(ErrorType::Expected, "a boolean value", o->left);
				return;
			}
			break;
		case OperatorSemanticsType::IntegerInteger:
		case OperatorSemanticsType::IntegerIntegerBitShift:
			if (dynamic_cast<CIntegerPrimitive*>(o->right->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "an integer value", o->right);
				return;
			}
		case OperatorSemanticsType::SingleInteger:
			if (dynamic_cast<CIntegerPrimitive*>(o->left->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "an integer value", o->left);
				return;
			}
			break;
		case OperatorSemanticsType::NumberNumber:
			if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "a numeric value", o->right);
				return;
			}
		case OperatorSemanticsType::SingleNumber:
			if (dynamic_cast<CNumericPrimitive*>(o->left->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "a numeric value", o->left);
				return;
			}
			break;
		case OperatorSemanticsType::NumberNumberOrStringString:
			if (dynamic_cast<CNumericPrimitive*>(o->left->dataType) != nullptr) {
				if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) == nullptr) {
					Error::logError(ErrorType::Expected, "a numeric value", o->right);
					return;
				}
			} else if (o->left->dataType == CDataType::stringType) {
				if (o->right->dataType != CDataType::stringType) {
					Error::logError(ErrorType::Expected, "a String value", o->right);
					return;
				}
			} else {
				if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) != nullptr)
					Error::logError(ErrorType::Expected, "a numeric value", o->left);
				else if (o->right->dataType == CDataType::stringType)
					Error::logError(ErrorType::Expected, "a String value", o->left);
				else
					Error::logError(ErrorType::Expected, "a pair of numeric or String values", o);
				return;
			}
			break;
		case OperatorSemanticsType::AnyAny:
			//this is either a comparison or an assignment, those will be handled individually
			break;
		case OperatorSemanticsType::Ternary:
			if (o->left->dataType != CDataType::boolType) {
				Error::logError(ErrorType::Expected, "a boolean value", o->left);
				return;
			}
			if ((ternaryResult = dynamic_cast<Operator*>(o->right)) == nullptr ||
				ternaryResult->operatorType != OperatorType::Colon)
			{
				Error::logError(ErrorType::General, "question mark missing colon in ternary operator", o);
				return;
			}
			semantToken(ternaryResult->left, variables, SemantExpressionLevel::Subexpression);
			semantToken(ternaryResult->right, variables, SemantExpressionLevel::Subexpression);
			ternaryResultType = CDataType::bestCompatibleType(ternaryResult->left->dataType, ternaryResult->right->dataType);
			if (ternaryResultType == nullptr) {
				string errorMessage = "ternary operator results in mismatching types " + ternaryResult->left->dataType->name +
					" and " + ternaryResult->right->dataType->name;
				Error::logError(ErrorType::General, errorMessage.c_str(), o);
				return;
			}
			o->dataType = ternaryResultType;
			return;
		default:
			Error::logError(ErrorType::CompilerIssue, "validating the operator semantics for this operator", o);
			assert(false);
			return;
	}

	//finally, assign the operator its type
	//for assignments, also check that the assignment is valid
	if (assignment) {
		if (semantExpressionLevel == SemantExpressionLevel::Subexpression)
			Error::logError(
				ErrorType::General, "assignment operators must be parenthesized or be the root operator of a statement", o);
		//make sure it's something we can assign to
		Identifier* i;
		if ((i = dynamic_cast<Identifier*>(o->left)) != nullptr)
			semantIdentifier(i, variables);
		else if (dynamic_cast<VariableDeclarationList*>(o->left) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable or variable list", o->left);
			return;
		}
		if (CDataType::bestCompatibleType(o->left->dataType, o->right->dataType) != o->left->dataType) {
			string errorMessage = "cannot use a value of type " + o->right->dataType->name +
				" to assign to a variable of type " + o->left->dataType->name;
			Error::logError(ErrorType::General, errorMessage.c_str(), o);
			return;
		}
		//TODO: Groups
		o->dataType = o->left->dataType;
	//check that comparisons are valid
	} else if (comparisonOperator) {
		if (CDataType::bestCompatibleType(o->left->dataType, o->right->dataType) == nullptr) {
			string errorMessage = "cannot compare values of type " + o->left->dataType->name +
				" and " + o->right->dataType->name;
			Error::logError(ErrorType::General, errorMessage.c_str(), o);
		}
		o->dataType = CDataType::boolType;
	} else if (o->right == nullptr || operatorSemanticsType == OperatorSemanticsType::IntegerIntegerBitShift)
		o->dataType = o->left->dataType;
	else
		o->dataType = CDataType::bestCompatibleType(o->left->dataType, o->right->dataType);
}
//verify that this function call has the right function and argument types
void Semant::semantFunctionCall(FunctionCall* f, PrefixTrie<char, CVariableDefinition*>* variables) {
	//make sure we have the right function type
	semantToken(f->function, variables, SemantExpressionLevel::Subexpression);
	if (!tokenHasKnownType(f->function))
		return;
	CSpecificFunction* functionType = dynamic_cast<CSpecificFunction*>(f->function->dataType);
	if (functionType == nullptr) {
		Error::logError(ErrorType::Expected, "a function value", f);
		return;
	}
	//make sure the arguments match
	if (functionType->parameterTypes->length != f->arguments->length) {
		string errorMessage = "expected " + std::to_string(functionType->parameterTypes->length) +
			" arguments but got " + std::to_string(f->arguments->length);
		Error::logError(ErrorType::General, errorMessage.c_str(), f);
	}
	int argumentCount = Math::min(functionType->parameterTypes->length, f->arguments->length);
	bool argumentTypesAreKnown = true;
	for (int i = 0; i < argumentCount; i++) {
		Token* t = f->arguments->get(i);
		semantToken(t, variables, SemantExpressionLevel::TopLevelInParentheses);
		if (tokenHasKnownType(t))
			checkType(t, functionType->parameterTypes->get(i));
		else
			argumentTypesAreKnown = false;
		f->arguments->set(i, t);
	}
	if (argumentTypesAreKnown)
		f->dataType = functionType->returnType;
}
//check all the statements of this function definition
void Semant::semantFunctionDefinition(FunctionDefinition* f, PrefixTrie<char, CVariableDefinition*>* variables) {
	if (shouldSemantStatements)
		semantStatementList(f->body);
	return;
}
//verify that this group ????????????????
void Semant::semantGroup(Group* g, PrefixTrie<char, CVariableDefinition*>* variables) {
	Error::logError(ErrorType::CompilerIssue, "a Group appearing an expression", g);
	assert(false); //TODO: Groups
	//TODO: ???????????
}
//semant all the statments in the given statement list
void Semant::semantStatementList(Array<Statement*>* statements) {
	forEach(Statement*, s, statements, si) {
		//TODO: ???????????
	}
}
//returns true if we don't need to re-semant this token
bool Semant::tokenHasKnownType(Token* t) {
	return t->dataType != nullptr && dynamic_cast<CGenericPointerType*>(t->dataType) == nullptr;
}
//returns whether the types match, and logs an error if they don't
//TODO: implicit casting
bool Semant::checkType(Token* t, CDataType* expectedType) {
	if (t->dataType != nullptr && t->dataType != expectedType) {
		string errorMessage = "expected a value of type " + expectedType->name + " but got " + t->dataType->name;
		Error::logError(ErrorType::General, errorMessage.c_str(), t);
		return false;
	}
	return true;
}
