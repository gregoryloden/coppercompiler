#include "Project.h"

//perform any pre-build expression optimizations

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
};
//optimize this token
Token* OptimizeExpressions::optimizeExpression(Token* t) {
	DirectiveTitle* d;
	ParenthesizedExpression* p;
	Cast* c;
	Operator* o;
	FunctionCall* fc;
	FunctionDefinition* fd;
	Group* g;
	if (let(DirectiveTitle*, d, t))
		;// return optimizeDirectiveTitle(d);
	else if (let(ParenthesizedExpression*, p, t)) {
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
	else if (let(Group*, g, t))
		optimizeGroup(g);
	return t;
}
//optimize this operator
Token* OptimizeExpressions::optimizeOperator(Operator* o) {
	o->left = optimizeExpression(o->left);
	if (o->right != nullptr) {
		o->right = optimizeExpression(o->right);
		//now that parenthesized expressions are gone, set the initial value
		VariableDeclarationList* v;
		if (o->operatorType == OperatorType::Assign && let(VariableDeclarationList*, v, o->left)) {
			//TODO: Groups - each variable gets a different initial value, also auto grouping and ungrouping
			forEach(CVariableDefinition*, vd, v->variables, vdi) {
				vd->initialValue = o->right;
			}
		}
		//TODO: combine constants
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
		}
	}
}
//optimize this group
void OptimizeExpressions::optimizeGroup(Group* g) {
	for (int i = 0; i < g->values->length; i++) {
		g->values->set(i, optimizeExpression(g->values->get(i)));
	}
}
