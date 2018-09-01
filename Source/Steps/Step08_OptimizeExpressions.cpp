#include "Project.h"

//perform any pre-build expression optimizations
//cache any information that may be useful later when we're building

//perform expression optimizations in all files
void OptimizeExpressions::optimizeExpressions(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		if (pliers->printProgress)
			printf("Optimizing %s...\n", s->path->fileName.c_str());
		forEach(Token*, t, s->globalVariables, ti) {
			optimizeExpression(t);
		}
		//TODO: classes
	}

	//also make sure we have a main function
	CVariableDefinition* mainFunction;
	FunctionDefinition* mainFunctionValue;
	CSpecificFunction* mainFunctionType;
	if (!let(CVariableDefinition*, mainFunction, pliers->allFiles->get(0)->variablesVisibleToFile->get("main", 4))) {
		EmptyToken errorToken (0, pliers->allFiles->get(0));
		Error::logError(ErrorType::General, "missing entry Function<void()> main", &errorToken);
	} else if (!let(CSpecificFunction*, mainFunctionType, mainFunction->type)
			|| mainFunctionType->returnType != CDataType::voidType
			|| mainFunctionType->parameterTypes->length > 0)
		Error::logError(ErrorType::General, "expected an entry Function<void()> main", mainFunction->name);
	else if (!let(FunctionDefinition*, mainFunctionValue, mainFunction->initialValue))
		Error::logError(
			ErrorType::General, "the entry function must be initialized with a function definition", mainFunction->name);
	else if (mainFunction->writtenTo)
		Error::logError(ErrorType::General, "the entry function cannot be mutated", mainFunction->name);
	else
		pliers->mainFunction = mainFunctionValue;
};
//optimize this token
Token* OptimizeExpressions::optimizeExpression(Token* t) {
	ParenthesizedExpression* p;
	Cast* c;
	Operator* o;
	FunctionCall* fc;
	FunctionDefinition* fd;
	GroupToken* g;
	if (let(ParenthesizedExpression*, p, t)) {
		t = optimizeExpression(p->expression);
		p->expression = nullptr;
		delete p;
	} else if (let(Operator*, o, t)) {
		if (let(Cast*, c, t))
			c->right = optimizeExpression(c->right);
		else if (istype(t, StaticOperator*))
			;
		else
			return optimizeOperator(o);
	} else if (let(FunctionCall*, fc, t))
		optimizeFunctionCall(fc);
	else if (let(FunctionDefinition*, fd, t))
		optimizeStatementList(fd->body);
	else if (let(GroupToken*, g, t))
		optimizeGroup(g);
	return t;
}
//optimize this operator
Token* OptimizeExpressions::optimizeOperator(Operator* o) {
	IntConstant* leftIntConstant;
	FloatConstant* leftFloatConstant;
	o->left = optimizeExpression(o->left);
	if (o->right != nullptr) {
		o->right = optimizeExpression(o->right);
		if (o->operatorType == OperatorType::Assign) {
			VariableDeclarationList* v;
			//now that parenthesized expressions are gone, set the initial value
			if (let(VariableDeclarationList*, v, o->left)) {
				//TODO: Groups - each variable gets a different initial value, also auto grouping and ungrouping
				forEach(CVariableDefinition*, vd, v->variables, vdi) {
					vd->initialValue = o->right;
				}
			}
		}
		//TODO: combine constants
	//if we have a unary operator on an int constant, we can edit the number and return that
	} else if (let(IntConstant*, leftIntConstant, o->left)) {
		if (o->operatorType == OperatorType::Negate)
			leftIntConstant->negative = !leftIntConstant->negative;
		else if (o->operatorType == OperatorType::BitwiseNot) {
			leftIntConstant->val->base = 1;
			leftIntConstant->val->digit(1);
			leftIntConstant->negative = !leftIntConstant->negative;
		//this should never happen, but don't error if it does
		} else
			return o;
		o->left = nullptr;
		delete o;
		return leftIntConstant;
	//if we have a unary operator on a float constant, we can edit the number and return that
	} else if (let(FloatConstant*, leftFloatConstant, o->left)) {
		if (o->operatorType == OperatorType::Negate)
			leftFloatConstant->negative = !leftFloatConstant->negative;
		//this should never happen, but don't error if it does
		else
			return o;
		o->left = nullptr;
		delete o;
		return leftFloatConstant;
	}
	return o;
}
//optimize this function call
void OptimizeExpressions::optimizeFunctionCall(FunctionCall* f) {
	f->function = optimizeExpression(f->function);
	for (int i = 0; i < f->arguments->length; i++) {
		f->arguments->set(i, optimizeExpression(f->arguments->get(i)));
	}
}
//optimize this statement
void OptimizeExpressions::optimizeStatementList(Array<Statement*>* statements) {
	forEach(Statement*, s, statements, si) {
		ExpressionStatement* e;
		ReturnStatement* r;
		IfStatement* i;
		LoopStatement* l;
		if (let(ExpressionStatement*, e, s))
			e->expression = optimizeExpression(e->expression);
		else if (let(ReturnStatement*, r, s))
			r->expression = optimizeExpression(r->expression);
		else if (let(IfStatement*, i, s)) {
			i->condition = optimizeExpression(i->condition);
			optimizeStatementList(i->thenBody);
			if (i->elseBody != nullptr)
				optimizeStatementList(i->elseBody);
		} else if (let(LoopStatement*, l, s)) {
			if (l->initialization != nullptr)
				l->initialization = optimizeExpression(l->initialization);
			l->condition = optimizeExpression(l->condition);
			if (l->increment != nullptr)
				l->increment = optimizeExpression(l->increment);
			optimizeStatementList(l->body);
		}
	}
}
//optimize this group
void OptimizeExpressions::optimizeGroup(GroupToken* g) {
	for (int i = 0; i < g->values->length; i++) {
		g->values->set(i, optimizeExpression(g->values->get(i)));
	}
}
