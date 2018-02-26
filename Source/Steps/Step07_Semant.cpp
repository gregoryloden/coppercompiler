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
	PrefixTrie<char, CVariableData*> allGlobalVariableData;
	forEach(SourceFile*, s, pliers->allFiles, si1) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			VariableDefinitionList* v;
			if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
				addVariablesToTrie(v, &allGlobalVariables);
			else if ((o = dynamic_cast<Operator*>(t)) != nullptr &&
				(v = dynamic_cast<VariableDefinitionList*>(o->left)) != nullptr)
			{
				addVariablesToTrie(v, &allGlobalVariables);
				forEach(CVariableDefinition*, c, v->variables, ci) {
					CVariableData::addToVariableData(&allGlobalVariableData, c->name->name, CVariableData::isInitialized);
				}
			}
		}
	}

	//then go through all the files to find global variables
	Array<Operator*> redoVariables;
	forEach(SourceFile*, s, pliers->allFiles, si2) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileDefinitions(s, &allGlobalVariables, &allGlobalVariableData, &redoVariables);
	}

	//next go through the redo variables and make sure that all variables end up with a specific type
	//at the end, any remaining generic types must be either circular definitions or assigned the wrong type
	//wrong type errors were already handled and circular definition errors will be handled later
	bool fixedVariable = true;
	while (fixedVariable) {
		fixedVariable = false;
		for (int i = redoVariables.length - 1; i >= 0; i--) {
			bool needsRedo = false;
			Operator* o = redoVariables.get(i);
			o->right = semantToken(o->right, &allGlobalVariables, &allGlobalVariableData, SemantExpressionLevel::TopLevel);
			VariableDefinitionList* v = dynamic_cast<VariableDefinitionList*>(o->left);
			forEach(CVariableDefinition*, c, v->variables, ci) {
				if (c->type == CDataType::functionType) {
					if (dynamic_cast<CSpecificFunction*>(o->right->dataType) != nullptr)
						c->type = o->right->dataType;
					else
						needsRedo = true;
				}
			}
			if (!needsRedo) {
				redoVariables.remove(i);
				fixedVariable = true;
			}
		}
	}

	//TODO: finalize byte sizes for primitive types

	//and now go through and semant all the rest of the code
	forEach(SourceFile*, s, pliers->allFiles, si3) {
		if (pliers->printProgress)
			printf("Analyzing semantics for global variables in %s...\n", s->filename.c_str());
		semantFileStatements(s, &allGlobalVariables, &allGlobalVariableData);
	}

	allGlobalVariableData.deleteValues();
}
//add variables from the definition list into the trie
void Semant::addVariablesToTrie(VariableDefinitionList* v, PrefixTrie<char, CVariableDefinition*>* variables) {
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
	SourceFile* sourceFile,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	Array<Operator*>* redoVariables)
{
	shouldSemantStatements = false;
	forEach(Token*, t, sourceFile->globalVariables, ti) {
		Operator* o = nullptr;
		VariableDefinitionList* v;
		if ((v = dynamic_cast<VariableDefinitionList*>(t)) != nullptr)
			;
		else if ((o = dynamic_cast<Operator*>(t)) == nullptr || o->operatorType != OperatorType::Assign) {
			Error::logError(ErrorType::Expected, "a variable definition", t);
			continue;
		} else if ((v = dynamic_cast<VariableDefinitionList*>(o->left)) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable declaration list", o->left);
			continue;
		} else
			semantToken(o, variables, variableData, SemantExpressionLevel::TopLevel);

		//if we have an assignment without a specific type, we have to re-semant it
		if (o != nullptr && !tokenHasKnownType(o->right))
			redoVariables->add(o);
		//check for any generic-type variables that will never get a specific type
		else {
			forEach(CVariableDefinition*, c, v->variables, ci) {
				//TODO: check for groups and classes
				if (c->type == CDataType::functionType) {
					string errorMessage = "could not determine specific " + c->type->name + " type for " + c->name->name;
					Error::logError(ErrorType::General, errorMessage.c_str(), c->name);
				}
			}
		}
	}
}
//????????????????????????????
void Semant::semantFileStatements(
	SourceFile* sourceFile, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	shouldSemantStatements = true;
	//??????????????????????????????????
}
//the heart of semantic analysis
//verify that this token has the right type, and that anything under it is also valid
Token* Semant::semantToken(
	Token* t,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	SemantExpressionLevel semantExpressionLevel)
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
		return semantIdentifier(i, variables, variableData);
	else if ((d = dynamic_cast<DirectiveTitle*>(t)) != nullptr)
		return semantDirectiveTitle(d, variables, variableData);
	else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr) {
		t = semantToken(p->expression, variables, variableData, SemantExpressionLevel::TopLevelInParentheses);
		p->expression = nullptr;
		delete p;
		return t;
	} else if ((c = dynamic_cast<Cast*>(t)) != nullptr)
		return semantCast(c, variables, variableData);
	else if ((s = dynamic_cast<StaticOperator*>(t)) != nullptr)
		return semantStaticOperator(s, variables, variableData);
	else if ((o = dynamic_cast<Operator*>(t)) != nullptr)
		return semantOperator(o, variables, variableData, semantExpressionLevel);
	else if ((fc = dynamic_cast<FunctionCall*>(t)) != nullptr)
		return semantFunctionCall(fc, variables, variableData);
	else if ((fd = dynamic_cast<FunctionDefinition*>(t)) != nullptr)
		return semantFunctionDefinition(fd, variables, variableData);
	else if ((g = dynamic_cast<Group*>(t)) != nullptr)
		return semantGroup(g, variables, variableData);
	else if (dynamic_cast<IntConstant*>(t) != nullptr ||
			dynamic_cast<FloatConstant*>(t) != nullptr ||
			dynamic_cast<BoolConstant*>(t) != nullptr ||
			dynamic_cast<StringLiteral*>(t) != nullptr)
		return t;
	else if (dynamic_cast<VariableDefinitionList*>(t) != nullptr) {
		if (semantExpressionLevel != SemantExpressionLevel::TopLevel)
			Error::logError(ErrorType::Expected, "a value", t);
		return t;
	} else {
		Error::logError(ErrorType::CompilerIssue, "determining the token type of this token", t);
		assert(false);
		return t;
	}
}
//give this identifier the right variable
Token* Semant::semantIdentifier(
	Identifier* i,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData)
{
	i->variable = variables->get(i->name.c_str(), i->name.length());
	if (i->variable == nullptr) {
		string errorMessage = "\"" + i->name + "\" has not been defined";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
	i->dataType = i->variable->type;
	if (!CVariableData::variableDataContains(variableData, i->name, CVariableData::isInitialized)) {
		string errorMessage = "\"" + i->name + "\" may not have been initialized";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
	return i;
}
//verify that this directive title ????????????????
Token* Semant::semantDirectiveTitle(
	DirectiveTitle* d, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	Error::logError(ErrorType::CompilerIssue, "a directive title being used as an expression", d);
	assert(false); //TODO: do something with directive titles
	//TODO: ???????????
	return d;
}
//verify that the sub-expression of this cast can actually be casted to the specified type
Token* Semant::semantCast(
	Cast* c, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	c->right = semantToken(c->right, variables, variableData, SemantExpressionLevel::Subexpression);
	if (!tokenHasKnownType(c->right))
		return c;
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
	return c;
}
//find the right variable for this static operator
Token* Semant::semantStaticOperator(
	StaticOperator* s, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	Identifier* i;
	if ((i = dynamic_cast<Identifier*>(s->right)) == nullptr) {
		Error::logError(ErrorType::ExpectedToFollow, "a member variable", s);
		return s;
	}
	//TODO: currently only Main has support for member variables
	if (s->ownerType != CDataType::mainType) {
		Error::logError(ErrorType::General, "currently only Main has member variable support", s);
		return s;
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
	} else {
		Error::logError(ErrorType::General, "currently Main only has support for print, str, and exit", i);
		return s;
	}
	//TODO: implement classes
	return s;
}
//verify that this operator has the right types for its subexpressions
Token* Semant::semantOperator(
	Operator* o,
	PrefixTrie<char, CVariableDefinition*>* variables,
	PrefixTrie<char, CVariableData*>* variableData,
	SemantExpressionLevel semantExpressionLevel)
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
			return o;
		//TODO: classes
		case OperatorType::Dot:
		case OperatorType::ObjectMemberAccess:
			Error::logError(ErrorType::General, "member variables are currently not supported", o);
			return o;
		//these should have already been handled
//		case OperatorType::None:
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
		case OperatorType::Cast:
		default:
			Error::logError(ErrorType::CompilerIssue, "determining the operator semantics for this operator", o);
			assert(false);
			return o;
	}
	//rewrite prefix operators to be postfix
	if (o->left == nullptr) {
		o->left = o->right;
		o->right = nullptr;
	}
	//semant the inner tokens
	if (o->left == nullptr) {
		Error::logError(ErrorType::CompilerIssue, "resulting in an operator without operands", o);
		assert(false);
		return o;
	} else if (!assignment)
		o->left = semantToken(o->left, variables, variableData, SemantExpressionLevel::Subexpression);
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
		return o;
	} else if (operatorSemanticsType != OperatorSemanticsType::Ternary)
		o->right = semantToken(o->right, variables, variableData, SemantExpressionLevel::Subexpression);
	//now make the specific operator checks
	//copper: switch with gotos
	Operator* ternaryResult;
	CDataType* ternaryResultType;
	switch (operatorSemanticsType) {
		case OperatorSemanticsType::BooleanBoolean:
			if (o->right->dataType != CDataType::boolType) {
				Error::logError(ErrorType::Expected, "a boolean value", o->right);
				return o;
			}
		case OperatorSemanticsType::SingleBoolean:
			if (o->left->dataType != CDataType::boolType) {
				Error::logError(ErrorType::Expected, "a boolean value", o->left);
				return o;
			}
			break;
		case OperatorSemanticsType::IntegerInteger:
		case OperatorSemanticsType::IntegerIntegerBitShift:
			if (dynamic_cast<CIntegerPrimitive*>(o->right->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "an integer value", o->right);
				return o;
			}
		case OperatorSemanticsType::SingleInteger:
			if (dynamic_cast<CIntegerPrimitive*>(o->left->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "an integer value", o->left);
				return o;
			}
			break;
		case OperatorSemanticsType::NumberNumber:
			if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "a numeric value", o->right);
				return o;
			}
		case OperatorSemanticsType::SingleNumber:
			if (dynamic_cast<CNumericPrimitive*>(o->left->dataType) == nullptr) {
				Error::logError(ErrorType::Expected, "a numeric value", o->left);
				return o;
			}
			break;
		case OperatorSemanticsType::NumberNumberOrStringString:
			if (dynamic_cast<CNumericPrimitive*>(o->left->dataType) != nullptr) {
				if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) == nullptr) {
					Error::logError(ErrorType::Expected, "a numeric value", o->right);
					return o;
				}
			} else if (o->left->dataType == CDataType::stringType) {
				if (o->right->dataType != CDataType::stringType) {
					Error::logError(ErrorType::Expected, "a String value", o->right);
					return o;
				}
			} else {
				if (dynamic_cast<CNumericPrimitive*>(o->right->dataType) != nullptr)
					Error::logError(ErrorType::Expected, "a numeric value", o->left);
				else if (o->right->dataType == CDataType::stringType)
					Error::logError(ErrorType::Expected, "a String value", o->left);
				else
					Error::logError(ErrorType::Expected, "a pair of numeric or String values", o);
				return o;
			}
			break;
		case OperatorSemanticsType::AnyAny:
			//this is either a comparison or an assignment, those will be handled individually
			break;
		case OperatorSemanticsType::Ternary:
			if ((ternaryResult = dynamic_cast<Operator*>(o->right)) == nullptr ||
				ternaryResult->operatorType != OperatorType::Colon)
			{
				Error::logError(ErrorType::General, "question mark missing colon in ternary operator", o);
				return o;
			}
			ternaryResultType = CDataType::bestCompatibleType(ternaryResult->left->dataType, ternaryResult->right->dataType);
			if (ternaryResultType == nullptr) {
				string errorMessage = "ternary operator results in mismatching types " + ternaryResult->left->dataType->name +
					" and " + ternaryResult->right->dataType->name;
				Error::logError(ErrorType::General, errorMessage.c_str(), o);
				return o;
			}
			o->dataType = ternaryResultType;
			return o;
			break;
		default:
			Error::logError(ErrorType::CompilerIssue, "validating the operator semantics for this operator", o);
			assert(false);
			return o;
	}

	//TODO: check for unknown types

	//finally, assign the operator its type
	//for assignments, also check that the assignment is valid
	if (assignment) {
		if (semantExpressionLevel == SemantExpressionLevel::Subexpression)
			Error::logError(
				ErrorType::General, "assignment operators must be parenthesized or be the root operator of a statement", o);
		//make sure it's something we can assign to
		Identifier* i;
		if ((i = dynamic_cast<Identifier*>(o->left)) != nullptr)
			o->left = semantIdentifier(i, variables, variableData);
		else if (dynamic_cast<VariableDefinitionList*>(o->left) == nullptr) {
			Error::logError(ErrorType::Expected, "a variable or variable list", o->left);
			return o;
		}
		if (CDataType::bestCompatibleType(o->left->dataType, o->right->dataType) != o->left->dataType) {
			string errorMessage = "cannot use a value of type " + o->right->dataType->name +
				" to assign to a variable of type " + o->left->dataType->name;
			Error::logError(ErrorType::General, errorMessage.c_str(), o);
			return o;
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
	return o;
}
//verify that this function call has the right function and argument types
Token* Semant::semantFunctionCall(
	FunctionCall* f, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	//make sure we have the right function type
	f->function = semantToken(f->function, variables, variableData, SemantExpressionLevel::Subexpression);
	if (!tokenHasKnownType(f->function))
		return f;
	CSpecificFunction* functionType = dynamic_cast<CSpecificFunction*>(f->function->dataType);
	if (functionType == nullptr) {
		Error::logError(ErrorType::Expected, "a function value", f);
		return f;
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
		Token* t = semantToken(f->arguments->get(i), variables, variableData, SemantExpressionLevel::TopLevelInParentheses);
		if (tokenHasKnownType(t))
			checkType(t, functionType->parameterTypes->get(i));
		else
			argumentTypesAreKnown = false;
		f->arguments->set(i, t);
	}
	if (argumentTypesAreKnown)
		f->dataType = functionType->returnType;
	return f;
}
//check all the statements of this function definition
Token* Semant::semantFunctionDefinition(
	FunctionDefinition* f, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	if (!shouldSemantStatements)
		return f;
	//TODO: ???????????
	return f;
}
//verify that this group ????????????????
Token* Semant::semantGroup(
	Group* g, PrefixTrie<char, CVariableDefinition*>* variables, PrefixTrie<char, CVariableData*>* variableData)
{
	Error::logError(ErrorType::CompilerIssue, "a Group appearing an expression", g);
	assert(false); //TODO: Groups
	//TODO: ???????????
	return g;
}
//returns true if we don't need to re-semant this token
bool Semant::tokenHasKnownType(Token* t) {
	return t->dataType != nullptr && dynamic_cast<CGenericFunction*>(t->dataType) == nullptr;
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
