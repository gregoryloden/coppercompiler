#include "Project.h"
#ifdef WIN32
	#include <Windows.h>
	#undef min
	#undef max
#endif

//build the final executable file
//TODO: finalize byte sizes for primitive types

//TODO:
//variable data: global variable data stored in tree
//each scope has a PrefixTrieUnion with the data
//when adding variable data:
//use the PrefixTrie check to see if it's got one already
//if not: make a new one, and if the parent has one (getFromParent), port the data from that one to the new one

//TODO: analyze stack shifts, and adjust memory pointers accodringly, except for memory pointers that expect a shifted stack

//TODO: function definitions
//-build assembly
//-stack modifications before + after function calls will only make space for parameters
//-nested function calls will not track stack offets from outer function calls
//-figure out how much stack space we need
//-**modify** stack modifications to be the right value (turn just-parameters constants into the whole stack)
//-arguments (MOV memory pointer destinations) don't get modified since they are already correctly relative to the stack pointer


Thunk TExitProcess ("ExitProcess", 0x9B);
Thunk TGetStdHandle ("GetStdHandle", 0x16A);
Thunk TWriteFile ("WriteFile", 0x2F7);
Thunk TGetProcessHeap ("GetProcessHeap", 0x156);
Thunk THeapAlloc ("HeapAlloc", 0x1BD);
Thunk THeapReAlloc ("HeapReAlloc", 0x1C4);
BuildInitialAssembly::FindUninitializedVariablesVisitor::FindUninitializedVariablesVisitor(
	PrefixTrie<char, CVariableData*>* pVariableData, bool pErrorForUninitializedVariables)
: TokenVisitor(onlyWhenTrackingIDs("FUVVTR"))
, allVariablesAreInitialized(true)
, errorForUninitializedVariables(pErrorForUninitializedVariables)
, variableData(pVariableData) {
}
BuildInitialAssembly::FindUninitializedVariablesVisitor::~FindUninitializedVariablesVisitor() {
	//don't delete the variable data, something else owns it
}
//go through the expression and make sure all of the variables it uses are initialized
void BuildInitialAssembly::FindUninitializedVariablesVisitor::handleExpression(Token* t) {
	Operator* o;
	Identifier* i;
	if (let(Operator*, o, t)) {
		if (o->operatorType == OperatorType::Assign) {
			handleExpression(o->right);
			if (!allVariablesAreInitialized)
				return;
			VariableDeclarationList* v;
			if (let(VariableDeclarationList*, v, o->left)) {
				forEach(CVariableDefinition*, c, v->variables, ci) {
					CVariableData::addToVariableData(variableData, c, CVariableData::isInitialized);
				}
			} else if (let(Identifier*, i, o->left))
				CVariableData::addToVariableData(variableData, i->variable, CVariableData::isInitialized);
			else
				Error::logError(ErrorType::CompilerIssue, "resulting in an assignment with a non-variable left side", o);
		} else if (o->operatorType == OperatorType::QuestionMark) {
			handleExpression(o->left);
			Operator* ternaryResult;
			if (!let(Operator*, ternaryResult, o->right)) {
				Error::logError(ErrorType::CompilerIssue, "resulting in a question mark without a corresponding colon", o);
				return;
			}
			//save any variable data in separate tries
			PrefixTrie<char, CVariableData*>* oldVariableData = variableData;
			PrefixTrieUnion<char, CVariableData*> leftVariableData (oldVariableData);
			variableData = &leftVariableData;
			handleExpression(ternaryResult->left);
			PrefixTrieUnion<char, CVariableData*> rightVariableData (oldVariableData);
			variableData = &rightVariableData;
			handleExpression(ternaryResult->right);
			variableData = oldVariableData;
			//any variable data that appears in both tries will go in the original trie
			Array<CVariableData*>* leftVariableDataList = leftVariableData.getValues();
			forEach(CVariableData*, c, leftVariableDataList, ci) {
				string& name = c->variable->name->name;
				CVariableData* other = rightVariableData.get(name.c_str(), name.length());
				if (other != nullptr) {
					//TODO: variable overloading
					if (c->variable != other->variable)
						Error::logError(ErrorType::CompilerIssue, "resulting in mismatched variables", other->variable->name);
					else {
						unsigned short combinedBitmask = c->dataBitmask & other->dataBitmask;
						if (combinedBitmask != 0)
							CVariableData::addToVariableData(variableData, c->variable, combinedBitmask);
					}
				}
				delete c;
			}
			delete leftVariableDataList;
			rightVariableData.deleteValues();
		} else if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
			handleExpression(o->left);
			PrefixTrie<char, CVariableData*>* oldVariableData = variableData;
			PrefixTrieUnion<char, CVariableData*> rightVariableData (oldVariableData);
			variableData = &rightVariableData;
			handleExpression(o->right);
			variableData = oldVariableData;
			rightVariableData.deleteValues();
		} else
			o->visitSubtokens(this);
	} else if (let(Identifier*, i, t)) {
		if (!CVariableData::variableDataContains(variableData, i->name, CVariableData::isInitialized)) {
			allVariablesAreInitialized = false;
			if (errorForUninitializedVariables) {
				string errorMessage = "\"" + i->name + "\" may not have been initialized";
				Error::logError(ErrorType::General, errorMessage.c_str(), i);
			}
		}
	} else
		t->visitSubtokens(this);
}
thread_local Array<AssemblyInstruction*>* BuildInitialAssembly::currentAssembly = nullptr;
thread_local Array<FunctionDefinition*>* BuildInitialAssembly::currentTempAssignmentDependencies = nullptr;
thread_local Array<AssemblyInstruction*>* BuildInitialAssembly::globalInitAssembly = nullptr;
thread_local Array<StringStaticStorage*>* BuildInitialAssembly::stringDefinitions = nullptr;
thread_local Array<FunctionStaticStorage*>* BuildInitialAssembly::functionDefinitions = nullptr;
thread_local Array<AssemblyStorage*>* BuildInitialAssembly::assemblyStorageToDelete = nullptr;
thread_local BitSize BuildInitialAssembly::cpuBitSize = BitSize::B32;
thread_local Register* BuildInitialAssembly::cpuARegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuCRegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuDRegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuBRegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuSPRegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuSIRegister = nullptr;
thread_local Register* BuildInitialAssembly::cpuDIRegister = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_exit = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_print = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_str = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable1 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable2 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable3 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable4 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::processHeapPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapNextFreeAddressPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapSizePointer = nullptr;
//build the final executable file for each bit size specified
void BuildInitialAssembly::buildInitialAssembly(Pliers* pliers) {
	buildInitialAssemblyForBitSize(pliers, BitSize::B32);
}
//build the final executable file for the specified bit size specified
void BuildInitialAssembly::buildInitialAssemblyForBitSize(Pliers* pliers, BitSize pCPUBitSize) {
	cpuBitSize = pCPUBitSize;
	if (cpuBitSize != BitSize::B32) {
		EmptyToken errorToken (0, pliers->allFiles->get(0));
		Error::logError(ErrorType::General, "only 32 bit compilation is supported", &errorToken);
		return;
	}
	if (pliers->printProgress)
		puts("Building executable...");

	//first: find out the order that global variables are initialized
	//gather all the variable initializations, initialize them if possible and save them for later if not
	PrefixTrie<char, CVariableData*> globalVariableData;
	Array<Operator*> initializationOrder;
	Array<Operator*> initializationsNotReady;
	forEach(SourceFile*, s, pliers->allFiles, si) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			if (!let(Operator*, o, t))
				continue;
			FindUninitializedVariablesVisitor visitor (&globalVariableData, false);
			visitor.handleExpression(o);
			(visitor.allVariablesAreInitialized ? &initializationOrder : &initializationsNotReady)->add(o);
		}
	}

	//next go through the initializations not ready and see if they're ready
	bool foundInitializedVariable = true;
	while (foundInitializedVariable) {
		foundInitializedVariable = false;
		for (int i = initializationsNotReady.length - 1; i >= 0; i--) {
			Operator* o = initializationsNotReady.get(i);
			FindUninitializedVariablesVisitor visitor (&globalVariableData, false);
			visitor.handleExpression(o);
			//if the variables are initialized now, add them to the ordered list
			if (visitor.allVariablesAreInitialized) {
				initializationsNotReady.remove(i);
				initializationOrder.add(o);
				foundInitializedVariable = true;
			}
		}
	}

	//error if there are any uninitialized global variables
	forEach(Operator*, o, &initializationsNotReady, oi) {
		FindUninitializedVariablesVisitor(&globalVariableData, true).handleExpression(o);
	}
	globalVariableData.deleteValues();

	setupAssemblyObjects();

	//now we have a list of global initializations in the order in which they occur
	//go get their assembly
	currentAssembly = globalInitAssembly;
	forEach(Operator*, o, &initializationOrder, oi2) {
		addTokenAssembly(o, nullptr, nullptr);
	}

	//now assign temps
	//start with the functions that have no other function dependencies, then slowly go up, that way we can
	//	possibly preserve register values across function calls
	//if we get to a point where dependencies are cyclic, just pick one and assume all registers are used
	Array<FunctionDefinition*>* functionDefinitionsToAssignTempsTo = new Array<FunctionDefinition*>();
	forEach(FunctionStaticStorage*, fss, functionDefinitions, fssi) {
		functionDefinitionsToAssignTempsTo->add(fss->val);
	}
	while (functionDefinitionsToAssignTempsTo->length > 0) {
		bool foundFunctionWithoutDependencies = false;
		for (int i = functionDefinitionsToAssignTempsTo->length - 1; i >= 0; i--) {
			if (assignTemps(functionDefinitionsToAssignTempsTo->get(i), true)) {
				functionDefinitionsToAssignTempsTo->remove(i);
				foundFunctionWithoutDependencies = true;
			}
		}
		//if we didn't find a function without dependencies, do the last one
		if (!foundFunctionWithoutDependencies)
			assignTemps(functionDefinitionsToAssignTempsTo->pop(), false);
	}
	delete functionDefinitionsToAssignTempsTo;

	//????????????????????????????

	//TODO: get the actual assembly bytes
	//TODO: optimize the assembly
	//	-remove unnecessary double jumps from boolean conditions
	//	-assign temps and remove MOVs where possible (same register or same memory address)
	//TODO: write the file

	cleanupAssemblyObjects();
}
//initialize everything used for building
void BuildInitialAssembly::setupAssemblyObjects() {
	globalInitAssembly = new Array<AssemblyInstruction*>();
	stringDefinitions = new Array<StringStaticStorage*>();
	functionDefinitions = new Array<FunctionStaticStorage*>();
	assemblyStorageToDelete = new Array<AssemblyStorage*>();
	cpuARegister = Register::aRegisterForBitSize(cpuBitSize);
	cpuCRegister = Register::cRegisterForBitSize(cpuBitSize);
	cpuDRegister = Register::dRegisterForBitSize(cpuBitSize);
	cpuBRegister = Register::bRegisterForBitSize(cpuBitSize);
	cpuSPRegister = Register::spRegisterForBitSize(cpuBitSize);
	cpuSIRegister = Register::siRegisterForBitSize(cpuBitSize);
	cpuDIRegister = Register::diRegisterForBitSize(cpuBitSize);
	if (cpuBitSize == BitSize::B32)
		build32BitMainFunctions();
	generalPurposeVariable1 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable2 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable3 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable4 = new ValueStaticStorage(cpuBitSize);
	processHeapPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapNextFreeAddressPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapSizePointer = new ValueStaticStorage(cpuBitSize);
}
//delete everything used for building
void BuildInitialAssembly::cleanupAssemblyObjects() {
	globalInitAssembly->deleteContents();
	delete globalInitAssembly;
	stringDefinitions->deleteContents();
	delete stringDefinitions;
	delete Main_exit->val;
	delete Main_print->val;
	delete Main_str->val;
	functionDefinitions->deleteContents();
	delete functionDefinitions;
	assemblyStorageToDelete->deleteContents();
	delete assemblyStorageToDelete;
	delete generalPurposeVariable1;
	delete generalPurposeVariable2;
	delete generalPurposeVariable3;
	delete generalPurposeVariable4;
	delete processHeapPointer;
	delete copperHeapPointer;
	delete copperHeapNextFreeAddressPointer;
	delete copperHeapSizePointer;
}
//build the Main.* functions
void BuildInitialAssembly::build32BitMainFunctions() {
	Identifier functionDefinitionSource ("", 0, 0, nullptr);
	AssemblyStorage* argumentStorage1 =
		globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 0, true, BitSize::B32));
	AssemblyStorage* argumentStorage2 =
		globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 4, true, BitSize::B32));
	AssemblyStorage* argumentStorage3 =
		globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 8, true, BitSize::B32));
	AssemblyStorage* argumentStorage4 =
		globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 12, true, BitSize::B32));
	AssemblyStorage* argumentStorage5 =
		globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 16, true, BitSize::B32));
	AssemblyConstant* constant32Bit4 = globalTrackedStorage(new AssemblyConstant(4, BitSize::B32));
	AssemblyConstant* constant32Bit12 = globalTrackedStorage(new AssemblyConstant(12, BitSize::B32));
	AssemblyConstant* constant8Bit0 = globalTrackedStorage(new AssemblyConstant(0, BitSize::B8));
	Register* blRegister = Register::bRegisterForBitSize(BitSize::B8);
	Register* dlRegister = Register::dRegisterForBitSize(BitSize::B8);

	CVariableDefinition* Main_exitIParameter = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_exitAssembly = new Array<AssemblyInstruction*>();
	Main_exitAssembly->add(new SUB(cpuSPRegister, constant32Bit4));
	addMemoryToMemoryMove(Main_exitAssembly, argumentStorage1, Main_exitIParameter->storage);
	Main_exitAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TExitProcess, BitSize::B32))));
	Main_exit = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::voidType,
			Array<CVariableDefinition*>::newArrayWith(Main_exitIParameter),
			new Array<Statement*>(),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_exit);
	Main_exitAssembly->add(new RET(Main_exit->val));
	Main_exit->val->instructions = Main_exitAssembly;

	CVariableDefinition* Main_printSParameter =
		new CVariableDefinition(CDataType::stringType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_printAssembly = new Array<AssemblyInstruction*>();
	Register* Main_printStringCharArrayRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B32));
	Register* Main_printStringCharArrayDataRegister =
		globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B32));
	//make space for WriteFile parameters
	Main_printAssembly->add(new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(20, BitSize::B32))));
	//get the StdOutputHandle
	Main_printAssembly->add(new SUB(cpuSPRegister, constant32Bit4));
	Main_printAssembly->add(
		new MOV(argumentStorage1, globalTrackedStorage(new AssemblyConstant(STD_OUTPUT_HANDLE, BitSize::B32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TGetStdHandle, BitSize::B32))));
	//load all the parameters for WriteFile
	Main_printAssembly->add(new MOV(argumentStorage1, cpuARegister));
	Main_printAssembly->add(new MOV(Main_printStringCharArrayRegister, Main_printSParameter->storage));
	Main_printAssembly->add(
		new MOV(
			Main_printStringCharArrayRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 4, BitSize::B32))));
	Main_printAssembly->add(
		new LEA(
			Main_printStringCharArrayDataRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 8, BitSize::B32))));
	Main_printAssembly->add(new MOV(argumentStorage2, Main_printStringCharArrayDataRegister));
	Main_printAssembly->add(
		new MOV(
			Main_printStringCharArrayDataRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 4, BitSize::B32))));
	Main_printAssembly->add(new MOV(argumentStorage3, Main_printStringCharArrayDataRegister));
	Main_printAssembly->add(
		new MOV(argumentStorage4, globalTrackedStorage(new StaticAddress(generalPurposeVariable1, BitSize::B32))));
	Main_printAssembly->add(new MOV(argumentStorage5, globalTrackedStorage(new AssemblyConstant(0, BitSize::B32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TWriteFile, BitSize::B32))));
	Main_print = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::voidType,
			Array<CVariableDefinition*>::newArrayWith(Main_printSParameter),
			new Array<Statement*>(),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_print);
	Main_printAssembly->add(new RET(Main_print->val));
	Main_print->val->instructions = Main_printAssembly;

	CVariableDefinition* Main_strIParameter = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_strAssembly = new Array<AssemblyInstruction*>();
	AssemblyLabel* Main_strWriteDigitLabel = new AssemblyLabel();
	AssemblyLabel* Main_addMinusSignLabel = new AssemblyLabel();
	AssemblyLabel* Main_allocateStringLabel = new AssemblyLabel();
	//setup initial values- dividend, buffer pointer, divisor, and negative flag
	Main_strAssembly->add(new MOV(cpuARegister, Main_strIParameter->storage));
	Main_strAssembly->add(new MOV(cpuSIRegister, cpuSPRegister));
	Main_strAssembly->add(new CDQ());
	Main_strAssembly->add(new MOV(cpuCRegister, globalTrackedStorage(new AssemblyConstant(10, BitSize::B32))));
	Main_strAssembly->add(new MOV(blRegister, dlRegister));
	//divide once and jump if we have a regular non-negative number
	Main_strAssembly->add(new IDIV(cpuCRegister));
	Main_strAssembly->add(new CMP(blRegister, constant8Bit0));
	Main_strAssembly->add(new JGE(Main_strWriteDigitLabel));
	//negative number- negate EAX and EDX then continue normally
	Main_strAssembly->add(new NEG(cpuARegister));
	Main_strAssembly->add(new NEG(cpuDRegister));
	//write our digit
	Main_strAssembly->add(Main_strWriteDigitLabel);
	Main_strAssembly->add(new DEC(cpuSIRegister));
	Main_strAssembly->add(new ADD(dlRegister, globalTrackedStorage(new AssemblyConstant('0', BitSize::B8))));
	Main_strAssembly->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuSIRegister, 0, BitSize::B8)), dlRegister));
	//if we reached 0, stop and go add a minus sign if it was negative
	Main_strAssembly->add(new CMP(cpuARegister, globalTrackedStorage(new AssemblyConstant(0, BitSize::B32))));
	Main_strAssembly->add(new JLE(Main_addMinusSignLabel));
	//divide again and write the digit
	Main_strAssembly->add(new CDQ());
	Main_strAssembly->add(new IDIV(cpuCRegister));
	Main_strAssembly->add(new JMP(Main_strWriteDigitLabel));
	Main_strAssembly->add(Main_addMinusSignLabel);
	//if we don't need to add the minus sign, go allocate the string
	Main_strAssembly->add(new CMP(blRegister, constant8Bit0));
	Main_strAssembly->add(new JGE(Main_allocateStringLabel));
	//write a minus sign
	Main_strAssembly->add(new DEC(cpuSIRegister));
	Main_strAssembly->add(
		new MOV(
			globalTrackedStorage(new MemoryPointer(cpuSIRegister, 0, BitSize::B8)),
			globalTrackedStorage(new AssemblyConstant('-', BitSize::B8))));
	//allocate space for the string
	Main_strAssembly->add(Main_allocateStringLabel);
	Main_strAssembly->add(new MOV(cpuARegister, copperHeapNextFreeAddressPointer));
	Main_strAssembly->add(new ADD(copperHeapNextFreeAddressPointer, constant32Bit12));
	//get the length of the string
	Main_strAssembly->add(new MOV(cpuCRegister, cpuSPRegister));
	Main_strAssembly->add(new SUB(cpuCRegister, cpuSIRegister));
	//allocate space for the char array
	Main_strAssembly->add(new MOV(cpuDRegister, copperHeapNextFreeAddressPointer));
	Main_strAssembly->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuDRegister, 4, BitSize::B32)), cpuCRegister));
	Main_strAssembly->add(new LEA(cpuBRegister, globalTrackedStorage(new MemoryPointer(cpuCRegister, 12, BitSize::B32))));
	Main_strAssembly->add(new AND(cpuBRegister, globalTrackedStorage(new AssemblyConstant(0xFFFFFFFC, BitSize::B32))));
	Main_strAssembly->add(new ADD(copperHeapNextFreeAddressPointer, cpuBRegister));
	//write to the char array
	Main_strAssembly->add(new CLD());
	Main_strAssembly->add(new LEA(cpuDIRegister, globalTrackedStorage(new MemoryPointer(cpuDRegister, 8, BitSize::B32))));
	Main_strAssembly->add(new REPMOVSB());
	//save the char array in the string
	Main_strAssembly->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32)), cpuDRegister));
	Main_str = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::stringType,
			Array<CVariableDefinition*>::newArrayWith(Main_strIParameter),
			new Array<Statement*>(),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_str);
	Main_strAssembly->add(new RET(Main_str->val));
	Main_str->val->instructions = Main_strAssembly;
}
//add the assembly that this token produces to the current assembly array
//returns the storage of the result of the token's assembly
AssemblyStorage* BuildInitialAssembly::addTokenAssembly(Token* t, CDataType* expectedType, ConditionLabelPair* jumpDests) {
	Identifier* i;
	DirectiveTitle* d;
	Cast* c;
	StaticOperator* so;
	Operator* o;
	FunctionCall* fc;
	FunctionDefinition* fd;
	Group* g;
	IntConstant* ic;
	FloatConstant* fcn;
	BoolConstant* b;
	StringLiteral* sl;
	if (let(Identifier*, i, t))
		return getIdentifierStorage(i);
	else if (let(DirectiveTitle*, d, t))
		//TODO: handle directive titles
		;
	else if (let(Operator*, o, t)) {
		if (let(Cast*, c, t))
			return addCastAssembly(c);
		else if (let(StaticOperator*, so, t))
			return getStaticOperatorStorage(so);
		else
			return getOperatorAssembly(o, jumpDests);
	} else if (let(FunctionCall*, fc, t))
		return getFunctionCallAssembly(fc);
	else if (let(FunctionDefinition*, fd, t))
		return getFunctionDefinitionStorage(fd);
	else if (let(Group*, g, t))
		//TODO: handle groups
		;
	else if (let(IntConstant*, ic, t))
		return getIntConstantStorage(ic, expectedType);
	else if (let(FloatConstant*, fcn, t))
		return getFloatConstantStorage(fcn, expectedType);
	else if (let(BoolConstant*, b, t))
		return globalTrackedStorage(new AssemblyConstant((int)(b->val), BitSize::B1));
	else if (let(StringLiteral*, sl, t)) {
		StringStaticStorage* sss = new StringStaticStorage(sl, cpuBitSize);
		stringDefinitions->add(sss);
		return sss;
	}
	//compiler error
	if (istype(t, ParenthesizedExpression*))
		Error::logError(ErrorType::CompilerIssue, "resulting in an unflattened parenthesized expression", t);
	else
		Error::logError(ErrorType::CompilerIssue, "obtaining assembly for this token", t);
	return cpuARegister;
}
//get the storage of the identifier
AssemblyStorage* BuildInitialAssembly::getIdentifierStorage(Identifier* i) {
	FunctionDefinition* f;
	if (let(FunctionDefinition*, f, i->variable->initialValue)) {
		if (currentTempAssignmentDependencies != nullptr) {
			//add this function as a temp assignment dependency if we haven't already
			for (int i = 0; true; i++) {
				if (i >= currentTempAssignmentDependencies->length) {
					currentTempAssignmentDependencies->add(f);
					break;
				} else if (currentTempAssignmentDependencies->get(i) == f)
					break;
			}
		}
		//if we know that this varible is never reassigned, we can return a static storage to the function
		if (f->staticallyAccessible)
			return globalTrackedStorage(new FunctionStaticStorage(f, cpuBitSize));
	}
	return i->variable->storage;
}
//get the assembly for the inner token and add any assembly needed to cast it to the expected type
AssemblyStorage* BuildInitialAssembly::addCastAssembly(Cast* c) {
	AssemblyStorage* result = addTokenAssembly(c->right, c->dataType, nullptr);
	BitSize targetBitSize = typeBitSize(c->dataType);
	BitSize valueBitSize = result->bitSize;
	if ((unsigned char)valueBitSize < (unsigned char)targetBitSize) {
		Register* newResult = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(targetBitSize));
		currentAssembly->add(new MOVSX(newResult, result));
		result = newResult;
	}
	//TODO: class type checking
	return result;
}
//get the storage pointed to by the static operator
AssemblyStorage* BuildInitialAssembly::getStaticOperatorStorage(StaticOperator* s) {
	Identifier* i;
	if (!let(Identifier*, i, s->right)) {
		Error::makeError(ErrorType::CompilerIssue, "getting assembly for this variable", s);
		return cpuARegister;
	}
	if (i->name == "exit")
		return Main_exit;
	else if (i->name == "print")
		return Main_print;
	else if (i->name == "str")
		return Main_str;
	else {
		Error::logError(ErrorType::CompilerIssue, "finding the function for this variable", i);
		return cpuARegister;
	}
}
//get the assembly for the inner tokens and the assembly to perform this operator on them
//returns nullptr if jump destinations were passed and they were used instead
Register* BuildInitialAssembly::getOperatorAssembly(Operator* o, ConditionLabelPair* jumpDests) {
	VariableDeclarationList* v = nullptr;
	CDataType* expectedType = o->precedence == OperatorTypePrecedence::Comparison
		? CDataType::bestCompatibleType(o->left->dataType, o->right->dataType)
		: o->dataType;
	//operators were rewritten to be postfix so the left token will always be present
	CDataType* leftExpectedType = typeBitSize(o->left->dataType) == BitSize::BInfinite ? expectedType : nullptr;
	CDataType* rightExpectedType =
		o->right != nullptr && typeBitSize(o->right->dataType) == BitSize::BInfinite ? expectedType : nullptr;
	AssemblyStorage* originalLeftStorage = nullptr;
	AssemblyStorage* leftStorage = nullptr;
	AssemblyStorage* rightStorage = nullptr;
	Register* resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(typeBitSize(expectedType)));
	//before we get the assembly for the left and right storages, we need to check for a few boolean comparison cases
	//if we do have jump destinations we'll return early
	if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
		bool andOperator = o->operatorType == OperatorType::BooleanAnd;
		AssemblyLabel* continueBooleanLabel = new AssemblyLabel();
		//get the left assembly
		AssemblyLabel* resultFinishedLabel =
			jumpDests != nullptr
				? andOperator ? jumpDests->falseJumpDest : jumpDests->trueJumpDest
				: new AssemblyLabel();
		if (andOperator) {
			ConditionLabelPair andJumpDests (continueBooleanLabel, resultFinishedLabel);
			leftStorage = addTokenAssembly(o->left, leftExpectedType, &andJumpDests);
		} else {
			ConditionLabelPair orJumpDests (resultFinishedLabel, continueBooleanLabel);
			leftStorage = addTokenAssembly(o->left, leftExpectedType, &orJumpDests);
		}
		//make sure we have a jump to our destination
		if (jumpDests != nullptr) {
			if (leftStorage != nullptr) {
				currentAssembly->add(new CMP(leftStorage, globalTrackedStorage(new AssemblyConstant(0, leftStorage->bitSize))));
				currentAssembly->add(
					andOperator
						? (AssemblyInstruction*)(new JE(resultFinishedLabel))
						: (AssemblyInstruction*)(new JNE(resultFinishedLabel)));
			}
		//make sure we have a value
		} else {
			//we got a value, jump if we don't need to check the rest of the boolean
			if (leftStorage != nullptr) {
				currentAssembly->add(new MOV(resultStorage, leftStorage));
				currentAssembly->add(
					new CMP(resultStorage, globalTrackedStorage(new AssemblyConstant(0, resultStorage->bitSize))));
				currentAssembly->add(
					andOperator
						? (AssemblyInstruction*)(new JE(resultFinishedLabel))
						: (AssemblyInstruction*)(new JNE(resultFinishedLabel)));
			//we didn't get a value, set the result and jump if we don't need to check the rest of the boolean
			} else {
				currentAssembly->add(resultFinishedLabel);
				resultFinishedLabel = new AssemblyLabel();
				currentAssembly->add(
					new MOV(
						resultStorage,
						globalTrackedStorage(new AssemblyConstant(andOperator ? 0 : 1, resultStorage->bitSize))));
				currentAssembly->add(new JMP(resultFinishedLabel));
			}
		}
		//add the right side normally
		currentAssembly->add(continueBooleanLabel);
		getConditionAssembly(o->right, resultStorage, rightExpectedType, jumpDests);
		if (jumpDests != nullptr)
			return nullptr;
		else {
			currentAssembly->add(resultFinishedLabel);
			return resultStorage;
		}
	} else if (o->operatorType == OperatorType::QuestionMark) {
		AssemblyLabel* ternaryTrueLabel = new AssemblyLabel();
		AssemblyLabel* ternaryFalseLabel = new AssemblyLabel();
		AssemblyLabel* ternaryAfterLabel = new AssemblyLabel();
		//check the condition and jump to the true or false assembly for this ternary operator
		ConditionLabelPair ternaryJumpDests (ternaryTrueLabel, ternaryFalseLabel);
		leftStorage = addTokenAssembly(o->left, leftExpectedType, &ternaryJumpDests);
		if (leftStorage != nullptr) {
			currentAssembly->add(new CMP(leftStorage, globalTrackedStorage(new AssemblyConstant(0, leftStorage->bitSize))));
			currentAssembly->add(new JE(ternaryFalseLabel));
		}
		Operator* ternaryResult = dynamic_cast<Operator*>(o->right);
		if (ternaryResult == nullptr) {
			Error::logError(ErrorType::CompilerIssue, "getting the results of this ternary operator", o);
			return cpuARegister;
		}
		//get the true and false assembly, using the jump dests if they were provided
		currentAssembly->add(ternaryTrueLabel);
		getConditionAssembly(ternaryResult->left, resultStorage, rightExpectedType, jumpDests);
		if (jumpDests == nullptr)
			currentAssembly->add(new JMP(ternaryAfterLabel));
		currentAssembly->add(ternaryFalseLabel);
		getConditionAssembly(ternaryResult->right, resultStorage, rightExpectedType, jumpDests);
		currentAssembly->add(ternaryAfterLabel);
		return jumpDests != nullptr ? nullptr : resultStorage;
	} else if (o->operatorType == OperatorType::LogicalNot && jumpDests != nullptr) {
		ConditionLabelPair reversedJumpDests (jumpDests->falseJumpDest, jumpDests->trueJumpDest);
		leftStorage = addTokenAssembly(o->left, leftExpectedType, &reversedJumpDests);
		if (leftStorage != nullptr) {
			currentAssembly->add(new CMP(leftStorage, globalTrackedStorage(new AssemblyConstant(0, leftStorage->bitSize))));
			//jump to the true dest if the inner expression is false
			currentAssembly->add(new JE(jumpDests->trueJumpDest));
			currentAssembly->add(new JMP(jumpDests->falseJumpDest));
		}
		return nullptr;
	}
	//we did not reach any of those special boolean comparison cases, we're just getting the value of the expression
	if (o->operatorType != OperatorType::Assign || !let(VariableDeclarationList*, v, o->left)) {
		originalLeftStorage = addTokenAssembly(o->left, leftExpectedType, nullptr);
		leftStorage = globalTrackedStorage(new TempStorage(originalLeftStorage->bitSize));
		//if both storages are memory addresses, we'll make them the same and remove the MOV
		currentAssembly->add(new MOV(leftStorage, originalLeftStorage));
	}
	rightStorage = o->right != nullptr ? addTokenAssembly(o->right, rightExpectedType, nullptr) : nullptr;
	if (o->operatorType != OperatorType::Assign)
		currentAssembly->add(new MOV(resultStorage, leftStorage));
	switch (o->operatorType) {
		//TODO: classes
		case OperatorType::Dot:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
		//TODO: classes
		case OperatorType::ObjectMemberAccess:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
		case OperatorType::Increment: currentAssembly->add(new INC(resultStorage)); break;
		case OperatorType::Decrement: currentAssembly->add(new DEC(resultStorage)); break;
		case OperatorType::VariableLogicalNot:
		case OperatorType::LogicalNot:
			currentAssembly->add(new CMP(resultStorage, globalTrackedStorage(new AssemblyConstant(0, resultStorage->bitSize))));
			currentAssembly->add(new SETE(resultStorage));
			break;
		case OperatorType::VariableBitwiseNot:
		case OperatorType::BitwiseNot:
			currentAssembly->add(new NOT(resultStorage));
			break;
		case OperatorType::VariableNegate:
		case OperatorType::Negate:
			currentAssembly->add(new NEG(resultStorage));
			break;
		case OperatorType::Multiply:
		case OperatorType::AssignMultiply: {
			//use as many operands as possible (as recommended by performance testing)
			AssemblyConstant* rightConstant;
			if (let(AssemblyConstant*, rightConstant, rightStorage))
				currentAssembly->add(new IMUL(resultStorage, resultStorage, rightConstant));
			else
				currentAssembly->add(new IMUL(resultStorage, rightStorage, nullptr));
			break;
		}
		case OperatorType::Divide:
		case OperatorType::Modulus:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
			resultStorage->specificRegister = Register::aRegisterForBitSize(resultStorage->bitSize)->specificRegister;
			if (resultStorage->bitSize == BitSize::B32)
				currentAssembly->add(new CDQ());
			else if (resultStorage->bitSize == BitSize::B16)
				currentAssembly->add(new CWD());
			else
				currentAssembly->add(new CBW());
			currentAssembly->add(new IDIV(resultStorage));
			if (o->operatorType == OperatorType::Modulus || o->operatorType == OperatorType::AssignModulus)
				resultStorage = Register::dRegisterForBitSize(resultStorage->bitSize);
			break;
		case OperatorType::Add:
		case OperatorType::AssignAdd:
			currentAssembly->add(new ADD(resultStorage, rightStorage));
			break;
		case OperatorType::Subtract:
		case OperatorType::AssignSubtract:
			currentAssembly->add(new SUB(resultStorage, rightStorage));
			break;
		case OperatorType::ShiftLeft:
		case OperatorType::AssignShiftLeft:
			if (istype(rightStorage, AssemblyConstant*))
				currentAssembly->add(new SHL(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentAssembly->add(new MOV(Register::cRegisterForBitSize(BitSize::B8), rightStorage));
				currentAssembly->add(new SHL(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftRight:
		case OperatorType::AssignShiftRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentAssembly->add(new SHR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentAssembly->add(new MOV(Register::cRegisterForBitSize(BitSize::B8), rightStorage));
				currentAssembly->add(new SHR(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftArithmeticRight:
		case OperatorType::AssignShiftArithmeticRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentAssembly->add(new SAR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentAssembly->add(new MOV(Register::cRegisterForBitSize(BitSize::B8), rightStorage));
				currentAssembly->add(new SAR(resultStorage, clRegister));
			}
			break;
//		case OperatorType::RotateLeft:
//			break;
//		case OperatorType::RotateRight:
//			break;
		case OperatorType::BitwiseAnd:
		case OperatorType::AssignBitwiseAnd:
			currentAssembly->add(new AND(resultStorage, rightStorage));
			break;
		case OperatorType::BitwiseXor:
		case OperatorType::AssignBitwiseXor:
			currentAssembly->add(new XOR(resultStorage, rightStorage));
			break;
		case OperatorType::BitwiseOr:
		case OperatorType::AssignBitwiseOr:
			currentAssembly->add(new OR(resultStorage, rightStorage));
			break;
		case OperatorType::Equal:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JE(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETE(resultStorage));
			break;
		case OperatorType::NotEqual:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JNE(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETNE(resultStorage));
			break;
		case OperatorType::LessOrEqual:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JLE(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETLE(resultStorage));
			break;
		case OperatorType::GreaterOrEqual:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JGE(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETGE(resultStorage));
			break;
		case OperatorType::LessThan:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JL(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETL(resultStorage));
			break;
		case OperatorType::GreaterThan:
			currentAssembly->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentAssembly->add(new JG(jumpDests->trueJumpDest));
				currentAssembly->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else
				currentAssembly->add(new SETG(resultStorage));
			break;
		case OperatorType::Assign: currentAssembly->add(new MOV(resultStorage, rightStorage)); break;
//		case OperatorType::AssignRotateLeft:
//			break;
//		case OperatorType::AssignRotateRight:
//			break;
//		case OperatorType::AssignBooleanAnd:
//			break;
//		case OperatorType::AssignBooleanOr:
//			break;
		//these should have already been handled
		case OperatorType::None:
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
		case OperatorType::Cast:
		case OperatorType::BooleanAnd:
		case OperatorType::BooleanOr:
		case OperatorType::Colon:
		case OperatorType::QuestionMark:
		default:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
	}
	if (o->modifiesVariable) {
		//multiple variable assignment
		if (v != nullptr) {
			forEach(CVariableDefinition*, vd, v->variables, vdi) {
				currentAssembly->add(new MOV(vd->storage, resultStorage));
			}
		//any other type of variable modifier
		} else
			currentAssembly->add(new MOV(leftStorage, resultStorage));
	}
	return resultStorage;
}
//get the assembly for a condition
//if we have jump destinations, ensure we jump to the right place
//if we do not, ensure resultStorage has the value of the expression
void BuildInitialAssembly::getConditionAssembly(
	Token* t, Register* resultStorage, CDataType* expectedType, ConditionLabelPair* jumpDests)
{
	AssemblyStorage* valueStorage = addTokenAssembly(t, expectedType, jumpDests);
	if (jumpDests != nullptr) {
		if (valueStorage != nullptr) {
			currentAssembly->add(new CMP(valueStorage, globalTrackedStorage(new AssemblyConstant(0, valueStorage->bitSize))));
			currentAssembly->add(new JNE(jumpDests->trueJumpDest));
			currentAssembly->add(new JMP(jumpDests->falseJumpDest));
		}
	} else if (resultStorage != nullptr && valueStorage != nullptr)
		currentAssembly->add(new MOV(resultStorage, valueStorage));
	else
		Error::logError(ErrorType::CompilerIssue, "getting assembly for the boolean value of this token", t);
}
//get the assembly for the arguments and the function and call it, returning the value
AssemblyStorage* BuildInitialAssembly::getFunctionCallAssembly(FunctionCall* f) {
	//get the function
	AssemblyStorage* functionStorage = addTokenAssembly(f->function, nullptr, nullptr);
	Identifier* i;
	FunctionDefinition* fd = dynamic_cast<FunctionDefinition*>(f->function);
	if (fd == nullptr && let(Identifier*, i, f->function))
		fd = dynamic_cast<FunctionDefinition*>(i->variable->initialValue);
	//if we don't have static function storage, store it in a temp somewhere while we compute our arguments
	if (!istype(functionStorage, FunctionStaticStorage*)) {
		//if both storages are memory addresses, we'll make them the same and remove the MOV
		TempStorage* functionTempStorage = globalTrackedStorage(new TempStorage(cpuBitSize));
		currentAssembly->add(new MOV(functionTempStorage, functionStorage));
		functionStorage = functionTempStorage;
	}
	//find out how much we need to shift the stack and shift it
	int shiftTotalBits = 0;
	forEach(Token*, t, f->arguments, ti) {
		shiftTotalBits += Math::max(8, (int)((unsigned char)typeBitSize(t->dataType)));
	}
	currentAssembly->add(
		new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant((shiftTotalBits / 8 + 3) & 0xFFFFFFFC, cpuBitSize))));
	AssemblyStorage* resultStorage;
	//then go through and add all the arguments
	//if we have a function definition, we can use its temps to save arguments, and return its result storage
	//sometime later we will adjust or remove the SUB for the stack if we were able to use register parameters
	if (fd != nullptr) {
		for (int i = 0; i < f->arguments->length; i++) {
			CVariableDefinition* vd = fd->parameters->get(i);
			addMemoryToMemoryMove(currentAssembly, vd->storage, addTokenAssembly(f->arguments->get(i), vd->type, nullptr));
		}
		resultStorage = fd->resultStorage;
	//if not, then they will have to be on the stack and return in the *A* register
	//get the positions that they will be
	} else {
		CSpecificFunction* sf = dynamic_cast<CSpecificFunction*>(f->function->dataType);
		if (sf == nullptr) {
			Error::logError(ErrorType::CompilerIssue, "determining the signature of this function", f->function);
			return cpuARegister;
		}
		Array<int>* parameterStackOrder = getParameterStackOrder(f->arguments);
		int stackOffset = 0;
		forEach(int, parameterIndex, parameterStackOrder, pi) {
			Token* argument = f->arguments->get(parameterIndex);
			AssemblyStorage* argumentStorage = addTokenAssembly(argument, sf->parameterTypes->get(parameterIndex), nullptr);
			addMemoryToMemoryMove(
				currentAssembly,
				globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, stackOffset, true, argumentStorage->bitSize)),
				argumentStorage);
			stackOffset += (int)((unsigned char)argumentStorage->bitSize) / 8;
		}
		delete parameterStackOrder;
		resultStorage = cpuARegister;
	}
	//if it's a static storage, we can just call it
	if (istype(functionStorage, FunctionStaticStorage*))
		currentAssembly->add(new CALL(functionStorage));
	else {
		Register* functionStorageRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
		currentAssembly->add(new MOV(functionStorageRegister, functionStorage));
		currentAssembly->add(
			new CALL(globalTrackedStorage(new MemoryPointer(functionStorageRegister, (unsigned char)cpuBitSize / 8, cpuBitSize))));
	}
	return resultStorage;
}
//get the storage of the function definition
FunctionStaticStorage* BuildInitialAssembly::getFunctionDefinitionStorage(FunctionDefinition* f) {
	FunctionStaticStorage* fss = new FunctionStaticStorage(f, cpuBitSize);
	if (f->instructions == nullptr) {
		Array<AssemblyInstruction*>* oldCurrentAssembly = currentAssembly;
		Array<FunctionDefinition*>* oldCurrentTempAssignmentDependencies = currentTempAssignmentDependencies;
		currentAssembly = (f->instructions = new Array<AssemblyInstruction*>());
		currentTempAssignmentDependencies = f->tempAssignmentDependencies;
		addStatementListAssembly(f->body, f);
		currentAssembly = oldCurrentAssembly;
		currentTempAssignmentDependencies = oldCurrentTempAssignmentDependencies;
		functionDefinitions->add(fss);
		return fss;
	} else
		return globalTrackedStorage(fss);
}
//get the int constant using the bit size of the expected type
AssemblyConstant* BuildInitialAssembly::getIntConstantStorage(IntConstant* i, CDataType* expectedType) {
	BitSize bitSize;
	//if we don't have an expected type then that must mean we're going to discard the result, just use the cpu bit size
	if (expectedType == nullptr)
		bitSize = cpuBitSize;
	else {
		bitSize = typeBitSize(expectedType);
		if (expectedType != nullptr && i->val->highBit() > (int)bitSize) {
			string errorMessage = "loss of precision when converting to " + expectedType->name;
			Error::logWarning(errorMessage.c_str(), i);
		}
	}
	int val = i->val->getInt();
	switch (bitSize) {
		case BitSize::B1: val = val != 0 ? 1 : 0; break;
		case BitSize::B8: val = (int)((char)val); break;
		case BitSize::B16: val = (int)((short)val); break;
		default: break;
	}
	return globalTrackedStorage(new AssemblyConstant(val, cpuBitSize));
}
//get the float constant using the bit size of the expected type
AssemblyConstant* BuildInitialAssembly::getFloatConstantStorage(FloatConstant* f, CDataType* expectedType) {
//TODO: get the storage
return globalTrackedStorage(new AssemblyConstant(0xAAAAAAAA, cpuBitSize));
}
//add the assembly from the statements to our current assembly array
void BuildInitialAssembly::addStatementListAssembly(Array<Statement*>* statements, FunctionDefinition* sourceFunction) {
	forEach(Statement*, s, statements, si) {
		ExpressionStatement* e;
		ReturnStatement* r;
		IfStatement* i;
		LoopStatement* l;
		if (let(ExpressionStatement*, e, s)) {
			if (!istype(e->expression, VariableDeclarationList*))
				addTokenAssembly(e->expression, sourceFunction->returnType, nullptr);
		} else if (let(ReturnStatement*, r, s)) {
			if (r->expression != nullptr) {
				AssemblyStorage* resultStorage = addTokenAssembly(r->expression, sourceFunction->returnType, nullptr);
				currentAssembly->add(new MOV(resultStorage, sourceFunction->resultStorage));
			}
			currentAssembly->add(new RET(sourceFunction));
		} else if (let(IfStatement*, i, s)) {
			AssemblyLabel* thenLabel = new AssemblyLabel();
			AssemblyLabel* elseLabel = new AssemblyLabel();
			ConditionLabelPair ifJumpDests (thenLabel, elseLabel);
			getConditionAssembly(i->condition, nullptr, nullptr, &ifJumpDests);
			currentAssembly->add(thenLabel);
			addStatementListAssembly(i->thenBody, sourceFunction);
			if (i->elseBody != nullptr) {
				AssemblyLabel* ifAfterLabel = new AssemblyLabel();
				currentAssembly->add(new JMP(ifAfterLabel));
				currentAssembly->add(elseLabel);
				addStatementListAssembly(i->elseBody, sourceFunction);
				currentAssembly->add(ifAfterLabel);
			} else
				currentAssembly->add(elseLabel);
		} else if (let(LoopStatement*, l, s)) {
			if (l->initialization != nullptr)
				addTokenAssembly(l->initialization, nullptr, nullptr);
			AssemblyLabel* loopStartLabel = new AssemblyLabel();
			AssemblyLabel* loopContinueLabel = new AssemblyLabel();
			AssemblyLabel* loopBreakLabel = new AssemblyLabel();
			ConditionLabelPair loopJumpDests (loopStartLabel, loopBreakLabel);
			getConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentAssembly->add(loopStartLabel);
			//TODO: pass this loop for breaks and continues
			addStatementListAssembly(l->body, sourceFunction);
			currentAssembly->add(loopContinueLabel);
			if (l->increment != nullptr)
				addTokenAssembly(l->increment, nullptr, nullptr);
			getConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentAssembly->add(loopBreakLabel);
		}
		//TODO: breaks and continues
	}
}
//assign registers or memory pointers to all temps of this function
//if we're checking dependencies and one of them hasn't assigned temps, don't assign temps for this function
//returns whether we assigned temps for this function
bool BuildInitialAssembly::assignTemps(FunctionDefinition* f, bool checkDependencies) {
	if (checkDependencies) {
		Array<FunctionDefinition*>* dependencies = f->tempAssignmentDependencies;
		while (dependencies->length > 0) {
			if (dependencies->last()->registersUsed == nullptr)
				return false;
			dependencies->pop();
		}
	}
	//TODO: assign temps
	return true;
}
//get the bit size to use for the provided type
BitSize BuildInitialAssembly::typeBitSize(CDataType* dt) {
	CPrimitive* p;
	return let(CPrimitive*, p, dt) ? p->bitSize : cpuBitSize;
}
//add the storage to the list and return it (to save callers from needing an extra local variable)
template <class AssemblyStorageType> AssemblyStorageType* BuildInitialAssembly::globalTrackedStorage(AssemblyStorageType* a) {
	assemblyStorageToDelete->add(a);
	return a;
}
//add two MOVs to transfer from (possible) memory pointer to (possible) memory pointer via a transfer register
//returns the register used in the transfer
Register* BuildInitialAssembly::addMemoryToMemoryMove(
	Array<AssemblyInstruction*>* assembly, AssemblyStorage* destination, AssemblyStorage* source)
{
	Register* transferRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(destination->bitSize));
	assembly->add(new MOV(transferRegister, source));
	assembly->add(new MOV(destination, transferRegister));
	return transferRegister;
}
//since smaller-bit-size parameters will be grouped together, get the indices that each parameter will appear in
Array<int>* BuildInitialAssembly::getParameterStackOrder(Array<Token*>* parameters) {
	Array<int>* parameterStackOrder = new Array<int>();
	if (parameters->length == 0)
		return parameterStackOrder;
	parameterStackOrder->add(0);
	for (int i = 1; i < parameters->length; i++) {
		unsigned char currentBitSize = (unsigned char)typeBitSize(parameters->get(i)->dataType);
		int insertionIndex = i;
		while (insertionIndex >= 1
				&& (unsigned char)typeBitSize(parameters->get(parameterStackOrder->get(insertionIndex - 1))->dataType)
					< currentBitSize)
			insertionIndex--;
		parameterStackOrder->insert(i, insertionIndex);
	}
	return parameterStackOrder;
}
