#include "Project.h"

/*
MEMPTR mstacktop (ESP, 0, -1, 0, false);
intptr_t stacktop = (intptr_t)(&mstacktop);

//get the high register of division for the given register type
int divhighreg(int rtype) {
	if (rtype == TREG8)
		return AH;
	return EDX;
}
//check if the context is a numerical context, include boolean if indicated
bool isNumerical(int context, bool includeboolean) {
	return context == TBYTE || context == TINT || (includeboolean && context == TBOOLEAN);
}
//get the register type for the given context
int getrtype(int context) {
	if (context == TBYTE || context == TBOOLEAN)
		return TREG8;
	return TREG32;
}
//get the register stack type for the given context
int getrtypestack(int context) {
	if (context == TBYTE || context == TBOOLEAN)
		return TREG16;
	return TREG32;
}
//get the bytes to pop for the given context
int getpopcount(int context) {
	if (context == TBYTE || context == TBOOLEAN)
		return 2;
	return 4;
}
//get the byte size of the given context
int getbytesize(int context) {
	if (context == TBYTE || context == TBOOLEAN)
		return 1;
	return 4;
}
//check if the expression is a separator and is the given type
bool isSeparatorType(Expression* e, int type) {
	return e->etype == ESEPARATOR && ((Separator*)(e))->val == type;
}
//check if the expression is a boolean operation and is the given type
bool isBooleanOperationType(Expression* e, int type) {
	return e->etype == EBOOLEANOPER && ((BooleanOperation*)(e))->oper == type;
}
//check if the expression is an operation and is a comparison
bool isCompareOperation(Expression* e) {
	return e->etype == EOPERATION && ((Operation*)(e))->prec == PCOMPARE;
}
//check if the expression is an operation and is a logical not
bool isLogicalNotOperation(Expression* e) {
	return e->etype == EOPERATION && ((Operation*)(e))->oper == OLNOT;
}
//check if the expression is an unknown value with the given text
bool isSpecificUnknownValue(Expression* e, char* c) {
	return e->etype == EUNKNOWNVALUE && ((UnknownValue*)(e))->val.compare(c) == 0;
}
//get an instruction representing the given operation
//if it's a jump, return a jump, otherwise return a set
AssemblyInstruction* comparisonInstruction(int oper, int type, int val, bool reverse, bool jump) {
	if (oper == OEQ || oper == ONEQ) {
		if (reverse == (oper == OEQ)) {
			if (jump)
				return new JNE(type, val);
			return new SETNE(type, val);
		}
		if (jump)
			return new JE(type, val);
		return new SETE(type, val);
	} else if (oper == OLEQ || oper == OGT) {
		if (reverse == (oper == OLEQ)) {
			if (jump)
				return new JG(type, val);
			return new SETG(type, val);
		}
		if (jump)
			return new JLE(type, val);
		return new SETLE(type, val);
	} else if (oper == OGEQ	|| oper == OLT) {
		if (reverse == (oper == OGEQ)) {
			if (jump)
				return new JL(type, val);
			return new SETL(type, val);
		}
		if (jump)
			return new JGE(type, val);
		return new SETGE(type, val);
	}
	//should never get here
	puts("Oh no! If you see this, there's a bug!");
	throw NULL;
	return NULL;
}
//form a conditional with the given arrays
Array<AssemblyInstruction*>* formConditional(Expression* condition,
	Array<AssemblyInstruction*>* aif, Array<AssemblyInstruction*>* aelse, bool deletable) {
	Array<AssemblyInstruction*> jumpslist;
	Array<AssemblyInstruction*>* a = getCondition(condition, false, &jumpslist, true);
	setJumps(&jumpslist, a->length, a->length + aif->length + 1);
	addAssembly(a, aif, deletable);
	//second jump: jump past everything
	a->add(new JMP(TINSTRUCTION, a->length + aelse->length + 1));
	return addAssembly(a, aelse, deletable);
}
//get the assembly of a conditional
//add any jumps to the jumpslist
Array<AssemblyInstruction*>* getCondition(Expression* condition, bool swap, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse) {
	//remove any extra nots
	while (isLogicalNotOperation(condition)) {
		swap = !swap;
		condition = ((Operation*)(condition))->left;
	}
	if (condition->etype == EBOOLEANOPER)
		return ((BooleanOperation*)(condition))->buildConditional(swap, jumpslist, jumpsonfalse);
	Array<AssemblyInstruction*>* a;
	AssemblyInstruction* jump;
	//most recent expression was a comparison operation: use a different jump instead
	if (isCompareOperation(condition)) {
		condition->toplevel = true;
		a = condition->getAssembly(false, EAX)
		->add(jump = comparisonInstruction(((Operation*)(condition))->oper, TINSTRUCTION, 0, swap != jumpsonfalse, true));
	} else {
		//most recent expression was a variable: compare it directly to 0 instead
		if (condition->etype == EVAR) {
			VariableData* v = ((Variable*)(condition))->inner;
			a = new Array<AssemblyInstruction*>(new CMP(v->ptype, v->ptr, TCONSTANT, 0));
		} else
			a = condition->getAssembly(false, EAX)
			->add(new CMP(getrtype(condition->context), EAX, TCONSTANT, 0));
		if (swap != jumpsonfalse)
			a->add(jump = new JE(TINSTRUCTION, 0));
		else
			a->add(jump = new JNE(TINSTRUCTION, 0));
	}
	jump->tag3 = jumpsonfalse ? 0 : 1;
	jumpslist->add(jump);
	return a;
}
//go through the jumps list and set them to jump to either the true destination or the false destination
void setJumps(Array<AssemblyInstruction*>* jumpslist, int truejmpdest, int falsejmpdest) {
	ArrayIterator<AssemblyInstruction*> ai (jumpslist);
	for (AssemblyInstruction* a = ai.getFirst(); ai.hasThis(); a = ai.getNext())
		a->set(TINSTRUCTION, a->tag3 != 0 ? truejmpdest : falsejmpdest);
}
//get all the assembly instructions from from and add them to to
Array<AssemblyInstruction*>* getAllAssembly(Array<AssemblyInstruction*>* to, Array<Expression*>* from) {
	ArrayIterator<Expression*> f (from);
	for (Expression* e = f.getFirst(); f.hasThis(); e = f.getNext())
		addAssembly(to, e->getAssembly(false, EAX), true);
	return to;
}
Expression::Expression(int theetype, bool thevalue, int thecontext, size_t thecontentpos):
ObjCounter("EXPN"),
	etype(theetype),
	value(thevalue),
	context(thecontext),
	contentpos(thecontentpos),
	toplevel(false) {
}
Expression::~Expression() {}
VariableData::VariableData(string thename, int thecontext, MEMPTR* theptr, int thetpos):
ObjCounter("VARD"),
	name(thename),
	ptr((intptr_t)(theptr)),
	ptype(ptrfromreg(getrtype(thecontext))),
	ptypestack(ptrfromreg(getrtypestack(thecontext))),
	context(thecontext),
	tpos(thetpos),
	fixedfunction(false) {
}
VariableData::VariableData(string thename, int thecontext, int datapos, int thetpos):
ObjCounter("VARD"),
	name(thename),
	ptr(datapos),
	ptype(dataptrfromptr(ptrfromreg(getrtype(thecontext)))),
	ptypestack(dataptrfromptr(ptrfromreg(getrtypestack(thecontext)))),
	context(thecontext),
	tpos(thetpos),
	fixedfunction(false) {
}
VariableData::VariableData(VariableData* v):
ObjCounter("VARD"),
	name(v->name),
	ptr(v->ptr),
	ptype(v->ptype),
	ptypestack(v->ptypestack),
	context(v->context),
	tpos(v->tpos),
	fixedfunction(false) {
}
VariableData::~VariableData() {}
Function::Function(int thecontext):
ObjCounter("FNCN"),
	context(thecontext),
	vnum(0) {
}
Function::~Function() {}
//get the instructions for the function
Array<AssemblyInstruction*>* Function::getInstructions() {
	return getAllAssembly(new Array<AssemblyInstruction*>(), &statements);
}
VariableStack::VariableStack(VariableData* v, Function* f, VariableStack* n):
ObjCounter("VSTK"),
	val(v),
	next(n),
	num(n != NULL ? n->num + 1 : 1) {
	if (num > f->vnum)
		f->vnum = num;
}
VariableStack::~VariableStack() {}
BlockStack::BlockStack(Expression* e, BlockStack* n):
ObjCounter("BSTK"),
	val(e),
	next(n) {
}
BlockStack::~BlockStack() {}
Thunk::Thunk(const char* thename, int thethunk):
ObjCounter("THNK"),
	used(false),
	thunk(to2bytes(thethunk).append(thename).append(1, 0)) {
}
Thunk::~Thunk() {}
Operation::Operation(string s, size_t thecontentpos, int thetpos):
	Expression(EOPERATION, false, TVOID, thecontentpos),
	right(NULL),
	testpart(NULL),
	tpos(thetpos) {
	if (s.compare("++") == 0)
		oper = OINC;
	else if (s.compare("--") == 0)
		oper = ODEC;
	else if (s.compare("~!") == 0)
		oper = OVARLNOT;
	else if (s.compare("~~") == 0)
		oper = OVARBNOT;
	else if (s.compare("~-") == 0)
		oper = OVARNEG;
	else if (s.compare("!") == 0)
		oper = OLNOT;
	else if (s.compare("~") == 0)
		oper = OBNOT;
	else if (s.compare("-|") == 0)
		oper = ONEG;
	else if (s.compare("*") == 0)
		oper = OMUL;
	else if (s.compare("/") == 0)
		oper = ODIV;
	else if (s.compare("%") == 0)
		oper = OMOD;
	else if (s.compare("+") == 0)
		oper = OADD;
	else if (s.compare("-") == 0)
		oper = OSUB;
	else if (s.compare("<<") == 0)
		oper = OSHL;
	else if (s.compare(">>") == 0)
		oper = OSHR;
	else if (s.compare(">>>") == 0)
		oper = OSAR;
	else if (s.compare("<-<") == 0)
		oper = OROL;
	else if (s.compare(">->") == 0)
		oper = OROR;
	else if (s.compare("&") == 0)
		oper = OAND;
	else if (s.compare("^") == 0)
		oper = OXOR;
	else if (s.compare("|") == 0)
		oper = OOR;
	else if (s.compare("==") == 0)
		oper = OEQ;
	else if (s.compare("!=") == 0)
		oper = ONEQ;
	else if (s.compare("<=") == 0)
		oper = OLEQ;
	else if (s.compare(">=") == 0)
		oper = OGEQ;
	else if (s.compare("<") == 0)
		oper = OLT;
	else if (s.compare(">") == 0)
		oper = OGT;
	else if (s.compare("&&") == 0)
		oper = OBAND;
	else if (s.compare("^^") == 0)
		oper = OBXOR;
	else if (s.compare("||") == 0)
		oper = OBOR;
	else if (s.compare("?") == 0)
		oper = OQMARK;
	else if (s.compare(":") == 0)
		oper = OCOLON;
	else if (s.compare("=") == 0)
		oper = OASSIGN;
	else if (s.compare("+=") == 0)
		oper = OASSIGNADD;
	else if (s.compare("-=") == 0)
		oper = OASSIGNSUB;
	else if (s.compare("*=") == 0)
		oper = OASSIGNMUL;
	else if (s.compare("/=") == 0)
		oper = OASSIGNDIV;
	else if (s.compare("%=") == 0)
		oper = OASSIGNMOD;
	else if (s.compare("<<=") == 0)
		oper = OASSIGNSHL;
	else if (s.compare(">>=") == 0)
		oper = OASSIGNSHR;
	else if (s.compare(">>>=") == 0)
		oper = OASSIGNSAR;
	else if (s.compare("<-<=") == 0)
		oper = OASSIGNROL;
	else if (s.compare(">->=") == 0)
		oper = OASSIGNROR;
	else if (s.compare("&=") == 0)
		oper = OASSIGNAND;
	else if (s.compare("^=") == 0)
		oper = OASSIGNXOR;
	else if (s.compare("|=") == 0)
		oper = OASSIGNOR;
	else if (s.compare("&&=") == 0)
		oper = OASSIGNBAND;
	else if (s.compare("^^=") == 0)
		oper = OASSIGNBXOR;
	else if (s.compare("||=") == 0)
		oper = OASSIGNBOR;
	else
		oper = 0;
	if (oper >= OASSIGN)
		prec = PASSIGN;
	else if (oper >= OQMARK)
		prec = PQMARK;
	else if (oper >= OBOR)
		prec = PBOR;
	else if (oper >= OBXOR)
		prec = PBXOR;
	else if (oper >= OBAND)
		prec = PBAND;
	else if (oper >= OEQ)
		prec = PCOMPARE;
	else if (oper >= OOR)
		prec = POR;
	else if (oper >= OXOR)
		prec = PXOR;
	else if (oper >= OAND)
		prec = PAND;
	else if (oper >= OSHL)
		prec = PSHIFT;
	else if (oper >= OADD)
		prec = PADDSUB;
	else if (oper >= OMUL)
		prec = PMULDIV;
	else if (oper >= OLNOT)
		prec = PUNARYPRE;
	else if (oper >= OINC)
		prec = PUNARYPOST;
	else
		prec = 0;
}
Operation::~Operation() {}
//get the instructions for the operation
Array<AssemblyInstruction*>* Operation::getAssembly(bool push, int reg) {
	//ternary operators
	if (testpart != NULL)
		return getAssemblyTernary(push, reg);
	//binary operators
	else if (right != NULL)
		return getAssemblyBinary(push, reg);
	//unary operators
	return getAssemblyUnary(push, reg);
}
//get the instructions for a ternary operation
Array<AssemblyInstruction*>* Operation::getAssemblyTernary(bool push, int reg) {
	return formConditional(testpart, left->getAssembly(push, reg), right->getAssembly(push, reg), true);
}
//get the instructions for a binary operation
Array<AssemblyInstruction*>* Operation::getAssemblyBinary(bool push, int reg) {
	Array<AssemblyInstruction*>* a = NULL;
	int thecontext = left->context;
	int rtype = getrtype(thecontext);
	int rtypestack = getrtypestack(thecontext);
	int popcount = getpopcount(thecontext);
	int ptype = ptrfromreg(rtype);
	int ptypestack = ptrfromreg(rtypestack);
	//type and val to hold the result
	int type2 = rtype;
	intptr_t val2 = reg;
	//binary: = += -= *= /= %= <<= >>= >>>= <-<= >->= &= ^= |=
	if (prec == PASSIGN) {
		VariableData* v = ((Variable*)(left))->inner;
		if (right->etype == EINTCONSTANT && isNumerical(right->context, true)) {
			type2 = TCONSTANT;
			if (oper == OASSIGNSHL || oper == OASSIGNSHR || oper == OASSIGNSAR || oper == OASSIGNROL || oper == OASSIGNROR)
				val2 = ((IntConstant*)(right))->ival & 0x1F;
			else
				val2 = ((IntConstant*)(right))->ival;
			a = new Array<AssemblyInstruction*>();
		//mul/div/mod puts it in ECX
		} else if (oper == OASSIGNMUL || oper == OASSIGNDIV || oper == OASSIGNMOD)
			a = right->getAssembly(false, ECX);
		//shift puts it in CL
		else if (oper == OASSIGNSHL || oper == OASSIGNSHR || oper == OASSIGNSAR || oper == OASSIGNROL || oper == OASSIGNROR) {
			a = right->getAssembly(false, ECX);
			type2 = TREG8;
			val2 = CL;
		//everything else goes in reg
		} else
			a = right->getAssembly(false, reg);
		//binary: =
		if (oper == OASSIGN) {
			a->add(new MOV(v->ptype, v->ptr, type2, val2));
			//if not toplevel, use regular value returning
			if (!toplevel) {
				if (type2 == TCONSTANT)
					return addAssembly(a, ((IntConstant*)(right))->getAssembly(push, reg), true);
				else if (push)
					return a->add(new PUSH(rtypestack, reg));
			}
			//ignore other non-toplevel handling
			return a;
		//binary: +=
		} else if (oper == OASSIGNADD) {
			if (type2 == TCONSTANT && (val2 == 1 || val2 == -1)) {
				if (val2 == 1)
					a->add(new INC(v->ptype, v->ptr));
				else
					a->add(new DEC(v->ptype, v->ptr));
			} else
				a->add(new ADD(v->ptype, v->ptr, type2, val2));
		//binary: -=
		} else if (oper == OASSIGNSUB) {
			if (type2 == TCONSTANT && (val2 == 1 || val2 == -1)) {
				if (val2 == 1)
					a->add(new DEC(v->ptype, v->ptr));
				else
					a->add(new INC(v->ptype, v->ptr));
			} else
				a->add(new SUB(v->ptype, v->ptr, type2, val2));
		//binary: *= /= %=
		} else if (oper == OASSIGNMUL || oper == OASSIGNDIV || oper == OASSIGNMOD) {
			a->add(new MOV(rtype, EAX, v->ptype, v->ptr));
			if (type2 == TCONSTANT)
				a->add(new MOV(rtype, ECX, type2, val2));
			int valreg = EAX;
			//binary: *
			if (oper == OASSIGNMUL)
				a->add(new IMUL(rtype, ECX));
			//binary: / %
			else if (oper == OASSIGNDIV || oper == OASSIGNMOD) {
				if (context == TBYTE)
					a->add(new CBW());
				else if (context == TINT)
					a->add(new CDQ());
				a->add(new IDIV(rtype, ECX));
				if (oper == OASSIGNMOD)
					valreg = divhighreg(rtype);
			}
			a->add(new MOV(v->ptype, v->ptr, rtype, valreg));
			//valreg already has the result, use that instead
			if (!toplevel) {
				if (push)
					return a->add(new PUSH(rtypestack, valreg));
				else if (valreg != reg)
					return a->add(new MOV(rtype, reg, rtype, valreg));
			}
			//ignore other non-toplevel handling
			return a;
		//binary: <<=
		} else if (oper == OASSIGNSHL)
			a->add(new SHL(v->ptype, v->ptr, type2, val2));
		//binary: >>
		else if (oper == OASSIGNSHR)
			a->add(new SHR(v->ptype, v->ptr, type2, val2));
		//binary: >>>
		else if (oper == OASSIGNSAR)
			a->add(new SAR(v->ptype, v->ptr, type2, val2));
		//binary: <-<
		else if (oper == OASSIGNROL)
			a->add(new ROL(v->ptype, v->ptr, type2, val2));
		//binary: >->
		else if (oper == OASSIGNROR)
			a->add(new ROR(v->ptype, v->ptr, type2, val2));
		//binary: &= &&=
		else if (oper == OASSIGNAND || oper == OASSIGNBAND)
			a->add(new AND(v->ptype, v->ptr, type2, val2));
		//binary: ^= ^^=
		else if (oper == OASSIGNXOR || oper == OASSIGNBXOR)
			a->add(new XOR(v->ptype, v->ptr, type2, val2));
		//binary: |= ||=
		else if (oper == OASSIGNOR || oper == OASSIGNBOR)
			a->add(new OR(v->ptype, v->ptr, type2, val2));
		//put the answer in the dest spot if it's not the top level assignment
		if (!toplevel) {
			if (push) {
				return a->add(new PUSH(v->ptypestack, v->ptr));
			} else
				return a->add(new MOV(rtype, reg, v->ptype, v->ptr));
		}
	//binary: * / %
	} else if (prec == PMULDIV) {
		int valreg = EAX;
		//binary: *
		if (oper == OMUL) {
			if (right->etype == EINTCONSTANT)
				a = left->getAssembly(false, EAX)
				->add(new MOV(rtype, ECX, TCONSTANT, ((IntConstant*)(right))->ival))
				->add(new IMUL(rtype, ECX));
			else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a = left->getAssembly(false, EAX)
				->add(new MOV(rtype, ECX, v->ptype, v->ptr))
				->add(new IMUL(rtype, ECX));
			} else {
				a = addAssembly(left->getAssembly(true, 0), right->getAssembly(false, EAX), true)
				->add(new IMUL(ptype, stacktop));
				if (push)
					return a->add(new MOV(ptype, stacktop, rtype, EAX));
				a->add(new ADD(TREG32, ESP, TCONSTANT, popcount));
			}
		//binary: / %
		} else if (oper == ODIV || oper == OMOD) {
			if (right->etype == EINTCONSTANT)
				a = left->getAssembly(false, EAX)
				->add(new MOV(rtype, ECX, TCONSTANT, ((IntConstant*)(right))->ival));
			else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a = left->getAssembly(false, EAX)
				->add(new MOV(rtype, ECX, v->ptype, v->ptr));
			} else
				a = addAssembly(left->getAssembly(true, 0), right->getAssembly(false, ECX), true)
				->add(new POP(rtypestack, EAX));
			if (context == TBYTE || context == TBOOLEAN)
				a->add(new CBW());
			else if (context == TINT)
				a->add(new CDQ());
			a->add(new IDIV(rtype, ECX));
			if (oper == OMOD)
				valreg = divhighreg(rtype);
		}
		if (push)
			return a->add(new PUSH(rtypestack, valreg));
		else if (reg != valreg)
			return a->add(new MOV(rtype, reg, rtype, valreg));
	} else {
		//preload the result for constant and variable right operands
		if (right->etype == EINTCONSTANT) {
			if (prec != PCOMPARE || left->etype != EVAR)
				a = left->getAssembly(false, reg);
		} else if (right->etype == EVAR)
			a = left->getAssembly(false, reg);
		else if (prec != PSHIFT)
			a = addAssembly(left->getAssembly(true, 0), right->getAssembly(false, reg), true);
		//binary: == != <= >= < >
		if (prec == PCOMPARE) {
			type2 = TREG8;
			//only true if there is already space on top of the stack for the result
			bool rightstacksize = false;
			if (right->etype == EINTCONSTANT) {
				//if left is a variable, just compare the variable to the constant
				if (left->etype == EVAR) {
					VariableData* v = ((Variable*)(left))->inner;
					a = new Array<AssemblyInstruction*>(new CMP(v->ptype, v->ptr, TCONSTANT, ((IntConstant*)(right))->ival));
				} else
					a->add(new CMP(rtype, reg, TCONSTANT, ((IntConstant*)(right))->ival));
			} else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a->add(new CMP(rtype, reg, v->ptype, v->ptr));
			} else {
				if ((left->context == TBYTE || left->context == TBOOLEAN) && (rightstacksize = push)) {
					a->add(new CMP(ptype, stacktop, rtype, reg));
					type2 = TBYTEPTR;
					val2 = stacktop;
				} else {
					int valreg = EAX;
					if (reg == EAX)
						valreg = ECX;
					a->add(new POP(rtypestack, valreg))
					->add(new CMP(rtype, valreg, rtype, reg));
				}
			}
			//conditional statements don't need the setCC
			if (!toplevel) {
				//binary: ==
				if (oper == OEQ)
					a->add(new SETE(type2, val2));
				//binary: !=
				else if (oper == ONEQ)
					a->add(new SETNE(type2, val2));
				//binary: <=
				else if (oper == OLEQ)
					a->add(new SETLE(type2, val2));
				//binary: >=
				else if (oper == OGEQ)
					a->add(new SETGE(type2, val2));
				//binary: <
				else if (oper == OLT)
					a->add(new SETL(type2, val2));
				//binary: >
				else if (oper == OGT)
					a->add(new SETG(type2, val2));
			}
			//ignore other handling if the last result was the right size and on the stack, and the answer is pushed
			if (rightstacksize)
				return a;
		//binary: |
		} else if (prec == POR) {
			if (right->etype == EINTCONSTANT)
				a->add(new OR(rtype, reg, TCONSTANT, ((IntConstant*)(right))->ival));
			else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a->add(new OR(rtype, reg, v->ptype, v->ptr));
			} else {
				//modify memory
				if (push)
					return a->add(new OR(ptype, stacktop, rtype, reg));
				//modify register
				else
					return a->add(new OR(rtype, reg, ptype, stacktop))
					->add(new ADD(TREG32, ESP, TCONSTANT, popcount));
			}
		//binary: ^ ^^
		} else if (prec == PXOR || prec == PBXOR) {
			if (right->etype == EINTCONSTANT)
				a->add(new XOR(rtype, reg, TCONSTANT, ((IntConstant*)(right))->ival));
			else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a->add(new XOR(rtype, reg, v->ptype, v->ptr));
			} else {
				//modify memory
				if (push)
					return a->add(new XOR(ptype, stacktop, rtype, reg));
				//modify register
				else
					return a->add(new XOR(rtype, reg, ptype, stacktop))
					->add(new ADD(TREG32, ESP, TCONSTANT, popcount));
			}
		//binary: &
		} else if (prec == PAND) {
			if (right->etype == EINTCONSTANT)
				a->add(new AND(rtype, reg, TCONSTANT, ((IntConstant*)(right))->ival));
			else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				a->add(new AND(rtype, reg, v->ptype, v->ptr));
			} else {
				//modify memory
				if (push)
					return a->add(new AND(ptype, stacktop, rtype, reg));
				//modify register
				else
					return a->add(new AND(rtype, reg, ptype, stacktop))
					->add(new ADD(TREG32, ESP, TCONSTANT, popcount));
			}
		//binary: << >> >>> <-< >->
		} else if (prec == PSHIFT) {
			int type1 = rtype;
			intptr_t val1 = reg;
			if (right->etype == EINTCONSTANT) {
				type2 = TCONSTANT;
				val2 = ((IntConstant*)(right))->ival & 0x1F;
			} else {
				type2 = TREG8;
				val2 = CL;
				if (right->etype == EVAR) {
					VariableData* v = ((Variable*)(right))->inner;
					a->add(new MOV(TREG8, CL, isptr(v->ptype) ? TBYTEPTR : TDATABYTEPTR, v->ptr));
					if (reg == ECX)
						val1 = EAX;
				} else {
					a = addAssembly(left->getAssembly(true, 0), right->getAssembly(false, ECX), true);
					if (push) {
						type1 = ptype;
						val1 = stacktop;
					} else {
						if (reg == ECX)
							val1 = EAX;
						a->add(new POP(rtypestack, val1));
					}
				}
			}
			//binary: <<
			if (oper == OSHL)
				a->add(new SHL(type1, val1, type2, val2));
			//binary: >>
			else if (oper == OSHR)
				a->add(new SHR(type1, val1, type2, val2));
			//binary: >>>
			else if (oper == OSAR)
				a->add(new SAR(type1, val1, type2, val2));
			//binary: <-<
			else if (oper == OROL)
				a->add(new ROL(type1, val1, type2, val2));
			//binary: >->
			else if (oper == OROR)
				a->add(new ROR(type1, val1, type2, val2));
			if (push) {
				if (right->etype == EVAR)
					return a->add(new PUSH(rtypestack, val1));
				//ignore regular push handling since the result is already pushed
				else if (right->etype != EINTCONSTANT)
					return a;
			//val1 is a register if push is false
			} else if (val1 != reg)
				return a->add(new MOV(rtype, reg, rtype, val1));
		//binary: + -
		} else if (prec == PADDSUB) {
			if (right->etype == EINTCONSTANT) {
				IntConstant* c = (IntConstant*)(right);
				//inc: add 1, sub -1
				if ((c->ival == 1 && oper == OADD) || (c->ival == -1 && oper == OSUB))
					a->add(new INC(rtype, reg));
				//dec: add -1, sub 1
				else if ((c->ival == -1 && oper == OADD) || (c->ival == 1 && oper == OSUB))
					a->add(new DEC(rtype, reg));
				//binary: +
				else if (oper == OADD)
					a->add(new ADD(rtype, reg, TCONSTANT, c->ival));
				//binary: -
				else if (oper == OSUB)
					a->add(new SUB(rtype, reg, TCONSTANT, c->ival));
			} else if (right->etype == EVAR) {
				VariableData* v = ((Variable*)(right))->inner;
				//binary: +
				if (oper == OADD)
					a->add(new ADD(rtype, reg, v->ptype, v->ptr));
				//binary: -
				else if (oper == OSUB)
					a->add(new SUB(rtype, reg, v->ptype, v->ptr));
			} else {
				//modify memory
				if (push) {
					if (oper == OSUB)
						return a->add(new SUB(ptype, stacktop, rtype, reg));
					else if (oper == OADD)
						return a->add(new ADD(ptype, stacktop, rtype, reg));
				//modify register
				} else {
					//add a NEG for subtractions
					if (oper == OSUB)
						a->add(new NEG(rtype, reg));
					return a->add(new ADD(rtype, reg, ptype, stacktop))
					->add(new ADD(TREG32, ESP, TCONSTANT, popcount));
				}
			}
		}
	}
	//push the argument if needed; not done for top level operations
	if (push)
		a->add(new PUSH(rtypestack, reg));
	return a;
}
//get the instructions for a unary operation
Array<AssemblyInstruction*>* Operation::getAssemblyUnary(bool push, int reg) {
	Array<AssemblyInstruction*>* a = NULL;
	int rtype = getrtype(context);
	int rtypestack = getrtypestack(context);
	//unary: ++ -- ~- ~~
	if (prec == PUNARYPOST) {
		a = new Array<AssemblyInstruction*>();
		VariableData* v = ((Variable*)(left))->inner;
		//unary: ++
		if (oper == OINC)
			a->add(new INC(v->ptype, v->ptr));
		//unary: --
		else if (oper == ODEC)
			a->add(new DEC(v->ptype, v->ptr));
		//unary: ~!
		else if (oper == OVARLNOT) {
			//boolean can xor with 1
			if (context == TBOOLEAN)
				a->add(new XOR(v->ptype, v->ptr, TCONSTANT, 1));
			else if (context == TBYTE) {
				a->add(new CMP(v->ptype, v->ptr, TCONSTANT, 0));
				if (toplevel)
					return a->add(new SETE(v->ptype, v->ptr));
				else {
					a->add(new SETE(TREG8, reg))
					->add(new MOV(v->ptype, v->ptr, rtype, reg));
					if (push)
						return a->add(new PUSH(rtypestack, reg));
				}
			} else {
				a->add(new CMP(v->ptype, v->ptr, TCONSTANT, 0))
				->add(new JNE(TINSTRUCTIONRELATIVE, 3));
				if (toplevel)
					return a->add(new MOV(v->ptype, v->ptr, TCONSTANT, 1))
					->add(new JMP(TINSTRUCTIONRELATIVE, 2))
					->add(new MOV(v->ptype, v->ptr, TCONSTANT, 0));
				else {
					a->add(new MOV(rtype, reg, TCONSTANT, 1))
					->add(new JMP(TINSTRUCTIONRELATIVE, 2))
					->add(new XOR(rtype, reg, rtype, reg))
					->add(new MOV(v->ptype, v->ptr, rtype, reg));
					if (push)
						return a->add(new PUSH(rtypestack, reg));
				}
			}
		//unary: ~~
		} else if (oper == OVARBNOT)
			a->add(new NOT(v->ptype, v->ptr));
		//unary: ~-
		else if (oper == OVARNEG)
			a->add(new NEG(v->ptype, v->ptr));
		//put the answer in the dest spot if it's not the top level assignment
		if (!toplevel) {
			if (push)
				return a->add(new PUSH(v->ptypestack, v->ptr));
			else
				return a->add(new MOV(rtype, reg, v->ptype, v->ptr));
		}
	//unary: ! ~ - -|
	} else if (prec == PUNARYPRE) {
		//put the expression in the destination spot
		//OLNOT with TINT puts it somewhere else
		if (oper != OLNOT || context != TINT)
			a = left->getAssembly(false, reg);
		//unary: !
		if (oper == OLNOT) {
			if (context == TBOOLEAN)
				a->add(new XOR(TREG8, reg, TCONSTANT, 1));
			else if (context == TBYTE) {
				if (push)
					return a->add(new SUB(TREG32, ESP, TCONSTANT, 2))
					->add(new CMP(TREG8, reg, TCONSTANT, 0))
					->add(new SETE(TBYTEPTR, stacktop));
				else
					return a->add(new CMP(TREG8, reg, TCONSTANT, 0))
					->add(new SETE(TREG8, reg));
			} else if (context == TINT) {
				int getreg = reg == ECX ? EAX : ECX;
				a = left->getAssembly(false, getreg)
				->add(new XOR(TREG32, reg, TREG32, reg))
				->add(new CMP(TREG32, getreg, TCONSTANT, 0))
				->add(new SETE(TREG8, reg));
			}
		//unary: ~
		} if (oper == OBNOT)
			a->add(new NOT(rtype, reg));
		//unary: - -|
		else if (oper == ONEG)
			a->add(new NEG(rtype, reg));
	}
	//push the argument if needed; not done for top level operations
	if (push)
		a->add(new PUSH(rtypestack, reg));
	return a;
}
BooleanOperation::BooleanOperation(int theoper, size_t thecontentpos):
	Expression(EBOOLEANOPER, true, TBOOLEAN, thecontentpos),
	oper(theoper),
	jumps(NULL) {
}
BooleanOperation::~BooleanOperation() {
	if (jumps != NULL)
		delete[] jumps;
}
//get the instructions for the boolean operation
Array<AssemblyInstruction*>* BooleanOperation::getAssembly(bool push, int reg) {
	//build the assembly array
	int length = inner.length - 1;
	Expression** innerinner = inner.inner;
	if (jumps == NULL)
		jumps = new AssemblyInstruction* [length];
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	bool isand = oper == OBAND;
	for (int i = 0; i < length; i += 1) {
		Expression* e = innerinner[i];
		addAssembly(a, e->getAssembly(false, reg), true);
		//if the last one was a comparison, no need to do it again
		if (isCompareOperation(e))
			a->add(jumps[i] = comparisonInstruction(((Operation*)(e))->oper, TINSTRUCTION, 0, isand, true));
		//not a comparison, it gets checked normally
		else {
			a->add(new CMP(TREG8, reg, TCONSTANT, 0));
			if (isand)
				a->add(jumps[i] = new JE(TINSTRUCTION, 0));
			else
				a->add(jumps[i] = new JNE(TINSTRUCTION, 0));
			//if it's a boolean operation, it can skip the compare and jump
			if (e->etype == EBOOLEANOPER) {
				BooleanOperation* b = (BooleanOperation*)(e);
				AssemblyInstruction** jumps2 = b->jumps;
				for (int i = b->inner.length - 2; i >= 0; i -= 1)
					jumps2[i]->tag2 += 2;
			}
		}
	}
	addAssembly(a, innerinner[length]->getAssembly(false, reg), true);
	int alength = a->length;
	for (int i = 0; i < length; i += 1)
		jumps[i]->set(TINSTRUCTION, alength);
	if (push)
		return a->add(new PUSH(TREG16, reg));
	return a;
}
//get the instructions for the boolean operation for use in a conditional
Array<AssemblyInstruction*>* BooleanOperation::buildConditional(bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse) {
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	addConditionalBody(a, not, jumpslist, jumpsonfalse);
	return addAssembly(a, getCondition(inner.inner[inner.length - 1], not, jumpslist, jumpsonfalse), true);
}
//for all the boolean operation's expressions except for the last one, get the instructions for use in a conditional
void BooleanOperation::addConditionalBody(Array<AssemblyInstruction*>* a, bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse) {
	//build the assembly array
	int length = inner.length - 1;
	Expression** innerinner = inner.inner;
	for (int i = 0; i < length; i += 1)
		handleBodyExpression(innerinner[i], false, a, not, jumpslist, jumpsonfalse);
}
//get the instructions of an expression that isn't the last expression in the boolean operation
void BooleanOperation::handleBodyExpression(Expression* e, bool swap, Array<AssemblyInstruction*>* a, bool not, Array<AssemblyInstruction*>* jumpslist, bool jumpsonfalse) {
	bool isand = oper == OBAND;
	//if it's in a condition, strip any nots
	while (isLogicalNotOperation(e)) {
		swap = !swap;
		e = ((Operation*)(e))->left;
	}
	//boolean operations are complicated- handle them based on their operation and the swap status
	if (e->etype == EBOOLEANOPER) {
		BooleanOperation* b = ((BooleanOperation*)(e));
		//swap with same oper or no swap with different oper
		//if it jumps, it jumps to the destination
		if ((b->oper == oper) != swap) {
			b->addConditionalBody(a, swap != not, jumpslist, isand);
			handleBodyExpression(b->inner.inner[b->inner.length - 1], swap, a, not, jumpslist, jumpsonfalse);
		//swap with different oper or no swap with same oper
		//if it jumps, it jumps to the next operation in the chain
		//its last expression, however, should be treated like one of this boolean operation's expressions
		} else {
			Array<AssemblyInstruction*> newjumpslist;
			b->addConditionalBody(a, swap != not, &newjumpslist, isand);
			handleBodyExpression(b->inner.inner[b->inner.length - 1], swap, a, not, jumpslist, jumpsonfalse);
			//set all the jumps to target the section after the last jump
			int nlength = newjumpslist.length;
			AssemblyInstruction** njumps = newjumpslist.inner;
			int alength = a->length;
			for (int j = 0; j < nlength; j += 1)
				njumps[j]->set(TINSTRUCTION, alength);
		}
	} else {
		AssemblyInstruction* jump;
		//most recent expression was a comparison operation: use a different jump instead
		if (isCompareOperation(e)) {
			e->toplevel = true;
			addAssembly(a, e->getAssembly(false, EAX), true)
			->add(jump = comparisonInstruction(((Operation*)(e))->oper, TINSTRUCTION, 0, isand != swap, true));
		} else {
			//most recent expression was a variable: compare it directly to 0 instead
			if (e->etype == EVAR) {
				VariableData* v = ((Variable*)(e))->inner;
				a->add(new CMP(v->ptype, v->ptr, TCONSTANT, 0));
			} else
				addAssembly(a, e->getAssembly(false, EAX), true)
				->add(new CMP(getrtype(e->context), EAX, TCONSTANT, 0));
			if (isand != swap)
				a->add(jump = new JE(TINSTRUCTION, 0));
			else
				a->add(jump = new JNE(TINSTRUCTION, 0));
		}
		//remember where to jump to
		jump->tag3 = not == isand ? 1 : 0;
		jumpslist->add(jump);
	}
}
Variable::Variable(VariableData* v, size_t thecontentpos):
	Expression(EVAR, true, v->context, thecontentpos),
	inner(v) {
}
Variable::~Variable() {}
//get the instructions for the variable
Array<AssemblyInstruction*>* Variable::getAssembly(bool push, int reg) {
	if (push)
		return new Array<AssemblyInstruction*>(new PUSH(inner->ptypestack, inner->ptr));
	return new Array<AssemblyInstruction*>(new MOV(getrtype(context), reg, inner->ptype, inner->ptr));
}
IntConstant::IntConstant(int i, size_t thecontentpos):
	Expression(EINTCONSTANT, true, TINT, thecontentpos),
	ival(i) {
}
IntConstant::IntConstant(bool b, size_t thecontentpos):
	Expression(EINTCONSTANT, true, TBOOLEAN, thecontentpos),
	ival(b ? 1 : 0) {
}
IntConstant::~IntConstant() {}
//get the instructions for the int constant
Array<AssemblyInstruction*>* IntConstant::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	if (context == TINT) {
		if (push)
			return a->add(new PUSH(TCONSTANT, ival));
		else
			return a->add(new MOV(TREG32, reg, TCONSTANT, ival));
	} else if (context == TBYTE || context == TBOOLEAN) {
		if (push) {
			return a->add(new SUB(TREG32, ESP, TCONSTANT, 2))
			->add(new MOV(TBYTEPTR, stacktop, TCONSTANT, ival));
		} else
			return a->add(new MOV(TREG8, reg, TCONSTANT, ival));
	}
	//only here to satisfy return requirements
	return a;
}
FloatConstant::FloatConstant(int e, BigInt* b, size_t thecontentpos):
	Expression(EFLOATCONSTANT, true, TFLOAT, thecontentpos),
	exp(e),
	bits(b),
	doubleprec(true),
	sign(false) {
}
FloatConstant::FloatConstant(size_t thecontentpos):
	Expression(EFLOATCONSTANT, true, TFLOAT, thecontentpos),
	exp(0),
	bits(10),
	doubleprec(true),
	sign(false) {
}
FloatConstant::~FloatConstant() {}
//get the instructions for the float constant
Array<AssemblyInstruction*>* FloatConstant::getAssembly(bool push, int reg) {
	int dataaddress = data.length();
	int expbias = doubleprec ? 1023 : 127;
	int floatprecision = doubleprec ? 53 : 24;
	//too high- use infinity
	if (exp > expbias) {
		puts("Warning: number will be represented as Infinity");
		if (doubleprec)
			data.append("\0\0\0\0\0\0\xF0\x7F", 8);
		else
			data.append("\0\0\x80\x7F", 4);
	} else {
		int shift = floatprecision - bits.bitCount();
		int round = 0;
		if (shift > 0)
			bits.lShift(shift);
		else if (shift < 0) {
			round = bits.getBit(-shift - 1);
			bits.rShift(-shift);
		}
		if (exp <= -expbias) {
			puts("Warning: loss of precision for denormal number");
			shift = -expbias + 1 - exp;
			bits.rShift(shift);
		}
		//construct the float
		if (doubleprec) {
			int getint = bits.getInt();
			bits.rShift(32);
			data.append(to4bytes(getint + round))
			.append(to4bytes((sign ? 0x80000000 : 0) |
				((exp + 1023 & 0x7FF) << 20) |
				((getint == -1 ? bits.getInt() + 1 : bits.getInt()) & 0xFFFFF)));
		} else
			data.append(to4bytes((sign ? 0x80000000 : 0) |
				((exp + 127 & 0xFF) << 23) |
				((bits.getInt() + round) & 0x7FFFFF)));
	}
	//wq
puts("Oh no! This isn't supported yet!");
throw NULL;
return NULL;
}
ObjectConstant::ObjectConstant(string s, size_t thecontentpos):
	Expression(EOBJECTCONSTANT, true, TSTRING, thecontentpos),
	sval(s) {
}
ObjectConstant::ObjectConstant(Function* f, string s, size_t thecontentpos):
	Expression(EOBJECTCONSTANT, true, TFUNCTION, thecontentpos),
	fval(f),
	sval(s) {
}
ObjectConstant::~ObjectConstant() {}
//get the instructions for the object constant
Array<AssemblyInstruction*>* ObjectConstant::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	if (context == TSTRING) {
		//get the heap location
		a->add(new MOV(TREG32, reg, TDATADWORDPTR, VDATACURRENTHEAPSPOT))
		//put the string on the heap
		->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(reg, 0, -1, STRING_LENGTH, true)), TCONSTANT, sval.length()));
		MEMPTR* m = new MEMPTR(reg, 0, -1, STRING_ADDR, false);
		a->add(new MOV(TDWORDPTR, (intptr_t)(m), TDATAADDRESS, data.length()));
		m->deletable = true;
		//bump up the heap
		a->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TCONSTANT, 8));
		//put the constant onto data
		data.append(sval);
		//load the address of the string
		if (push)
			return a->add(new PUSH(TREG32, reg));
	} else if (context == TFUNCTION) {
		//get the heap location
		a->add(new MOV(TREG32, reg, TDATADWORDPTR, VDATACURRENTHEAPSPOT));
		//put the string on the heap
		MEMPTR* m = new MEMPTR(reg, 0, -1, FUNCTION_ADDR, false);
		a->add(new MOV(TDWORDPTR, (intptr_t)(m), TCODEADDRESS, (intptr_t)(fval)));
		m->deletable = true;
		//bump up the heap
		a->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TCONSTANT, 4));
		//load the address of the function variable
		if (push)
			return a->add(new PUSH(TREG32, reg));
	}
	return a;
}
StringConcatenation::StringConcatenation(size_t thecontentpos):
	Expression(ESTRINGCONCAT, true, TSTRING, thecontentpos) {
}
StringConcatenation::~StringConcatenation() {}
//get the instructions for the string concatenation
Array<AssemblyInstruction*>* StringConcatenation::getAssembly(bool push, int reg) {
	//compact strings
	Expression** sinner = strings.inner;
	int smax = strings.length - 1;
	//there are at least two expressions, and at least one is not a constant
	//search for consecutive constants and concatenate them
	for (int i = 0; i < smax; i += 1) {
		if (sinner[i]->etype == EOBJECTCONSTANT && sinner[i + 1]->etype == EOBJECTCONSTANT) {
			((ObjectConstant*)(sinner[i]))->sval.append(((ObjectConstant*)(sinner[i + 1]))->sval);
			strings.remove(i + 1);
			smax = strings.length - 1;
		}
	}
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	int vars = 0;
	//push all the string addresses
	ArrayIterator<Expression*> s (&strings);
	for (Expression* e = s.getFirst(); s.hasThis(); e = s.getNext()) {
		if (e->etype != EOBJECTCONSTANT) {
			addAssembly(a, e->getAssembly(true, 0), true);
			vars += 1;
		}
	}
	//add up all of the string sizes to ECX
	a->add(new XOR(TREG32, ECX, TREG32, ECX));
	int var = vars;
	for (Expression* e = s.getFirst(); s.hasThis(); e = s.getNext()) {
		if (e->etype != EOBJECTCONSTANT) {
			var -= 1;
			//pointer value has string address
			a->add(new MOV(TREG32, EAX, TDWORDPTR, (intptr_t)(new MEMPTR(ESP, 0, -1, var * 4, true))))
			//pointer value has size of string
			->add(new ADD(TREG32, ECX, TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_LENGTH, true))));
		} else
			a->add(new ADD(TREG32, ECX, TCONSTANT, ((ObjectConstant*)(e))->sval.length()));
	}
	//allocate a new string with its size and array location
	//get the heap location
	int heapreg = (reg != ECX && reg != ESI && reg != EDI) ? reg : EAX;
	a->add(new MOV(TREG32, heapreg, TDATADWORDPTR, VDATACURRENTHEAPSPOT))
	//put the string on the heap
	//store size
	->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(heapreg, 0, -1, STRING_LENGTH, true)), TREG32, ECX))
	//store the location, use the destination so that it's already set
	->add(new LEA(TREG32, EDI, TDWORDPTR, (intptr_t)(new MEMPTR(heapreg, 0, -1, 8, true))))
	->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(heapreg, 0, -1, STRING_ADDR, true)), TREG32, EDI))
	//bump up the heap
	->add(new ADD(TREG32, ECX, TCONSTANT, 8))
	->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TREG32, ECX))
	//clear direction flag
	->add(new CLD());
	//move all the strings into the new array
	var = vars;
	for (Expression* e = s.getFirst(); s.hasThis(); e = s.getNext()) {
		if (e->etype != EOBJECTCONSTANT) {
			var -= 1;
			//get the string address
			a->add(new MOV(TREG32, ESI, TDWORDPTR, (intptr_t)(new MEMPTR(ESP, 0, -1, var * 4, true))))
			//get the length of the string
			->add(new MOV(TREG32, ECX, TDWORDPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, STRING_LENGTH, true))))
			//get the address of the character array from the current string
			->add(new MOV(TREG32, ESI, TDWORDPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, STRING_ADDR, true))));
		} else {
			//get the length of the string
			a->add(new MOV(TREG32, ECX, TCONSTANT, ((ObjectConstant*)(e))->sval.length()))
			//get the address of the string
			->add(new MOV(TREG32, ESI, TDATAADDRESS, data.length()));
			//add to data
			data.append(((ObjectConstant*)(e))->sval);
		}
		//begin string moving
		a->add(new REPMOVSB());
	}
	//pop all strings
	a->add(new ADD(TREG32, ESP, TCONSTANT, vars * 4));
	//push heapreg if needed, the answer was put in heapreg a while ago
	if (push)
		a->add(new PUSH(TREG32, heapreg));
	else if (heapreg != reg)
		a->add(new MOV(TREG32, reg, TREG32, heapreg));
	return a;
}
Cast::Cast(int to, Expression* from):
	Expression(ECAST, true, to, from->contentpos),
	inner(from) {
}
Cast::Cast(int to, size_t thecontentpos):
	Expression(ECAST, false, to, thecontentpos) {}
Cast::~Cast() {}
//check if this cast is valid
bool Cast::validCast() {
	return (isNumerical(inner->context, true) && isNumerical(context, true)) ||
		context == TBOOLEAN;
}
//get the instructions for the cast
Array<AssemblyInstruction*>* Cast::getAssembly(bool push, int reg) {
	//no casting needed
	if (inner->context == context)
		return inner->getAssembly(push, reg);
	Array<AssemblyInstruction*>* a = NULL;
	//logic negation can result in multiple types, no need to cast for some cases
	if (inner->etype == EOPERATION) {
		Operation* o = (Operation*)(inner);
		//logical not-ing, less work needed to resize cast
		//if no cast found, continue to regular casting
		if (o->oper == OLNOT || o->oper == OVARLNOT) {
			if (inner->context == TINT) {
				if (context == TBOOLEAN || context == TBYTE) {
					a = inner->getAssembly(false, reg);
					if (push)
						return a->add(new PUSH(TREG16, reg));
					return a;
				}
			} else if (inner->context == TBYTE) {
				if (context == TBOOLEAN)
					return inner->getAssembly(push, reg);
			}
		}
	}
	if (context == TBOOLEAN) {
		//cast by byte size
		int bsize = getbytesize(inner->context);
		//cast from byte
		if (bsize == 1) {
			a = inner->getAssembly(false, reg)
			->add(new CMP(TREG8, reg, TCONSTANT, 0))
			->add(new SETNE(TREG8, reg));
			if (push)
				return a->add(new PUSH(TREG16, reg));
		//cast from int or object
		} else if (bsize == 4) {
			int getreg = reg == ECX ? EAX : ECX;
			a = inner->getAssembly(false, getreg)
			->add(new XOR(TREG32, reg, TREG32, reg))
			->add(new CMP(TREG32, getreg, TCONSTANT, 0))
			->add(new SETE(TREG8, reg));
			if (push)
				return a->add(new PUSH(TREG16, reg));
		}
	} else if (context == TBYTE) {
		if (inner->context == TBOOLEAN)
			return inner->getAssembly(push, reg);
		else if (inner->context == TINT) {
			//negation can result in multiple types, no need to cast for some cases
			if (inner->etype == EOPERATION) {
				Operation* o = (Operation*)(inner);
				if (o->oper == OLNOT || o->oper == OVARLNOT)
					return inner->getAssembly(push, reg);
			}
			a = inner->getAssembly(false, reg);
			if (push)
				a->add(new PUSH(TREG16, reg));
		}
	} else if (context == TINT) {
		if (inner->context == TBOOLEAN || inner->context == TBYTE) {
			a = inner->getAssembly(false, reg)
			->add(new MOVSX(TREG32, reg, TREG8, reg));
			if (push)
				return a->add(new PUSH(TREG32, reg));
		}
	}
	return a;
}
FunctionCall::FunctionCall(Expression* e):
	Expression(EFUNCTIONCALL, true, TVOID, e->contentpos) {
	if (e->etype == EOBJECTCONSTANT) {
		finner = ((ObjectConstant*)(e))->fval;
		context = finner->context;
	} else {
		einner = e;
		finner = NULL;
	}
}
FunctionCall::~FunctionCall() {}
//get the instructions for the function call
Array<AssemblyInstruction*>* FunctionCall::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	ArrayIterator<Expression*> ai (&params);
	//push all the parameters
	for (Expression* e = ai.getFirst(); ai.hasThis(); e = ai.getNext()) {
		addAssembly(a, e->getAssembly(true, 0), true);
	}
	//set the function if there is one
	if (finner == NULL && einner->etype == EVAR) {
		VariableData* v = ((Variable*)(einner))->inner;
		if (v->fixedfunction)
			finner = ((FixedFunction*)(v))->inner;
	}
	//run the function
	if (finner != NULL)
		a->add(new CALL(finner));
	else
		addAssembly(a, einner->getAssembly(false, EAX), true)
		->add(new CALL(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, FUNCTION_ADDR, true))));
	if (push)
		return a->add(new PUSH(TREG32, EAX));
	return a;
}
AssemblySequence::AssemblySequence():
	Expression(EASSEMBLYSEQUENCE, false, TNONE, 0) {
}
AssemblySequence::AssemblySequence(AssemblyInstruction* a):
	Expression(EASSEMBLYSEQUENCE, false, TNONE, 0),
	inner(a) {
}
AssemblySequence::~AssemblySequence() {}
//get the instructions for the assembly sequence
Array<AssemblyInstruction*>* AssemblySequence::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*>* a = new Array<AssemblyInstruction*>();
	a->resize(inner.length);
	ArrayIterator<AssemblyInstruction*> ai (&inner);
	for (AssemblyInstruction* i = ai.getFirst(); ai.hasThis(); i = ai.getNext())
		a->add(i->clone());
	return a;
}
UnknownValue::UnknownValue(string s, size_t thecontentpos):
	Expression(EUNKNOWNVALUE, true, TNONE, thecontentpos),
	val(s) {
}
UnknownValue::~UnknownValue() {}
//get the instructions for the unknown value
Array<AssemblyInstruction*>* UnknownValue::getAssembly(bool push, int reg) {
	puts("Oh no! If you see this, there's a bug!");
	throw NULL;
	return NULL;
}
Separator::Separator(char c, size_t thecontentpos):
	Expression(ESEPARATOR, false, TNONE, thecontentpos) {
	if (c == '(')
		val = SLPAREN;
	else if (c == ')')
		val = SRPAREN;
	else if (c == ',')
		val = SCOMMA;
	else
		val = SSEMICOLON;
}
Separator::~Separator() {}
//get the instructions for the separator
Array<AssemblyInstruction*>* Separator::getAssembly(bool push, int reg) {
	puts("Oh no! If you see this, there's a bug!");
	throw NULL;
	return NULL;
}
Return::Return(Expression* e, Function* f):
	Expression(ERETURN, false, e->context, e->contentpos),
	einner(e),
	finner(f) {
}
Return::~Return() {}
//get the instructions for the return
Array<AssemblyInstruction*>* Return::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*>* a = (einner == NULL) ? 
		new Array<AssemblyInstruction*>() : einner->getAssembly(false, EAX);
	int pnum = finner->params.length * 4;
	//function-specific variables
	if (finner->variables.length > 0)
		a->add(new MOV(TREG32, ESP, TREG32, EBP))
		->add(new POP(TREG32, EBP));
	else if (pnum > 0)
		a->add(new POP(TREG32, EBP));
	a->add(new RET(pnum));
	return a;
}
IfBranch::IfBranch(size_t thecontentpos):
	Expression(EIFBRANCH, false, TNONE, thecontentpos) {
}
IfBranch::~IfBranch() {}
//get the instructions for the if branch
Array<AssemblyInstruction*>* IfBranch::getAssembly(bool push, int reg) {
	Array<AssemblyInstruction*> aif;
	getAllAssembly(&aif, &ifpart);
	Array<AssemblyInstruction*> aelse;
	getAllAssembly(&aelse, &elsepart);
	bool useif;
	//one of them has no length: account for this in jumping
	if ((useif = aelse.length == 0) || aif.length == 0) {
		Array<AssemblyInstruction*> jumpslist;
		Array<AssemblyInstruction*>* a = getCondition(condition, false, &jumpslist, useif);
		Array<AssemblyInstruction*>* abody = useif ? &aif : &aelse;
		int truejmpdest = a->length;
		addAssembly(a, abody, false);
		if (useif)
			setJumps(&jumpslist, truejmpdest, a->length);
		else
			setJumps(&jumpslist, a->length, truejmpdest);
		return a;
	}
	//both have something: make a regular conditional
	return formConditional(condition, &aif, &aelse, false);
}
ControlFlow::ControlFlow(string s, size_t thecontentpos):
	Expression(ECONTROLFLOW, false, TNONE, thecontentpos),
	code(NULL) {
	if (s.compare("break") == 0)
		val = CBREAK;
	else if (s.compare("continue") == 0)
		val = CCONTINUE;
	else if (s.compare("case") == 0)
		val = CCASE;
	else
		val = CDEFAULT;
}
ControlFlow::~ControlFlow() {}
//get the instructions for the control flow
Array<AssemblyInstruction*>* ControlFlow::getAssembly(bool push, int reg) {
	//if the loop is a for loop and an infinite loop, and this is a continue, apply the increment
	return code == NULL ? new Array<AssemblyInstruction*>(jump = new JMP(TINSTRUCTION, 0)) :
		code->getAssembly(false, EAX)->add(jump = new JMP(TINSTRUCTION, 0));
}
MultiLoop::MultiLoop(size_t thecontentpos, bool thedowhile):
	Expression(EMULTILOOP, false, TNONE, thecontentpos),
	initialization(NULL),
	dowhile(thedowhile),
	increment(NULL) {
}
MultiLoop::~MultiLoop() {}
//get the instructions for the loop
Array<AssemblyInstruction*>* MultiLoop::getAssembly(bool push, int reg) {
	//get the condition
	//if the condition is a constant, make a special jump
	if (condition->etype == EINTCONSTANT) {
		//no looping
		if (((IntConstant*)(condition))->ival == 0) {
			//for false or while false
			if (!dowhile)
				return new Array<AssemblyInstruction*>();
			//get the body statements
			Array<AssemblyInstruction*>* a = getAllAssembly(new Array<AssemblyInstruction*>(), &statements);
			ArrayIterator<ControlFlow*> aicontrolflows (&controlflows);
			int breakjmpdest = a->length;
			for (ControlFlow* c = aicontrolflows.getFirst(); aicontrolflows.hasThis(); c = aicontrolflows.getNext())
				c->jump->set(TINSTRUCTION, breakjmpdest);
			return a;
		}
		//infinite loop
		//get the initialization if there is one
		Array<AssemblyInstruction*>* a = initialization == NULL ?
			new Array<AssemblyInstruction*>() : initialization->getAssembly(false, EAX);
		int continuejmpdest = a->length;
		//get the body
		getAllAssembly(a, &statements);
		//add the increment if there is one
		if (increment != NULL)
			addAssembly(a, increment->getAssembly(false, EAX), true);
		a->add(new JMP(TINSTRUCTION, continuejmpdest));
		int breakjmpdest = a->length;
		ArrayIterator<ControlFlow*> aicontrolflows (&controlflows);
		for (ControlFlow* c = aicontrolflows.getFirst(); aicontrolflows.hasThis(); c = aicontrolflows.getNext())
			c->jump->set(TINSTRUCTION, c->val == CCONTINUE ? continuejmpdest : breakjmpdest);
		return a;
	}
	//get the initalization
	Array<AssemblyInstruction*>* a = initialization == NULL ?
		new Array<AssemblyInstruction*>() : initialization->getAssembly(false, EAX);
	Array<AssemblyInstruction*> jumpslist;
	//add an extra condition if it's not do-while
	if (!dowhile)
		addAssembly(a, getCondition(condition, false, &jumpslist, true), true);
	int truejmpdest = a->length;
	//get the body statements
	getAllAssembly(a, &statements);
	int continuejmpdest = a->length;
	//add the increment if there is one
	if (increment != NULL)
		addAssembly(a, increment->getAssembly(false, EAX), true);
	//add the ending condition
	addAssembly(a, getCondition(condition, false, &jumpslist, false), true);
	int falsebreakjmpdest = a->length;
	//set all the condition jumps and control flow jumps to jump to the right places
	setJumps(&jumpslist, truejmpdest, falsebreakjmpdest);
	ArrayIterator<ControlFlow*> aicontrolflows (&controlflows);
	for (ControlFlow* c = aicontrolflows.getFirst(); aicontrolflows.hasThis(); c = aicontrolflows.getNext())
		c->jump->set(TINSTRUCTION, c->val == CCONTINUE ? continuejmpdest : falsebreakjmpdest);
	return a;
}
FixedFunction::FixedFunction(VariableData* it):
	VariableData(it),
	inner(NULL) {
	fixedfunction = true;
	delete it;
}
FixedFunction::~FixedFunction() {}
MainFunction::MainFunction(Function* f, string s, size_t thecontentpos):
	ObjectConstant(f, s, thecontentpos),
	added(false) {
}
MainFunction::~MainFunction() {}
*/
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
SourceFile::SourceFile(string pFilename)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("SRCFILE")))
filename(pFilename)
, contents(nullptr)
, contentsLength(0)
, abstractContents(nullptr)
, rowStarts(new Array<int>())
, includedFiles(new AVLTree<SourceFile*, bool>())
, inclusionListeners(new Array<SourceFile*>()) {
	rowStarts->add(0);
	//load the file
	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "rb");
	if (file == nullptr) {
		printf("Unable to open file \"%s\"\n", filename.c_str());
		return;
	}
	fseek(file, 0, SEEK_END);
	contentsLength = ftell(file);
	contents = new char[contentsLength + 1];
	rewind(file);
	fread(contents, 1, contentsLength, file);
	contents[contentsLength] = '\0';
	fclose(file);
}
SourceFile::~SourceFile() {
	delete[] contents;
	delete abstractContents;
	delete rowStarts;
	//don't delete the source files, they will get deleted through the main source file list
	delete includedFiles;
	delete inclusionListeners;
}
CDirective::CDirective()
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs("DIRECTV"))) {
}
CDirective::~CDirective() {}
CDirectiveReplace::CDirectiveReplace(string pToReplace, Array<string>* pInput, AbstractCodeBlock* pReplacement)
: CDirective()
, toReplace(pToReplace)
, input(pInput)
, replacement(pReplacement) {
}
CDirectiveReplace::~CDirectiveReplace() {
	delete input;
	delete replacement;
}
CDirectiveInclude::CDirectiveInclude(string pFilename, bool pIncludeAll)
: CDirective()
, filename(pFilename)
, includeAll(pIncludeAll) {
}
CDirectiveInclude::~CDirectiveInclude() {}
