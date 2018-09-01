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

//TODO: if a static function is only called once, insert its instructions into where it's called


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
thread_local FunctionDefinition* BuildInitialAssembly::currentOwningFunction = nullptr;
thread_local Array<FunctionDefinition*>* BuildInitialAssembly::currentTempAssignmentDependencies = nullptr;
thread_local FunctionDefinition* BuildInitialAssembly::globalInit = nullptr;
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
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable1 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable2 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable3 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::generalPurposeVariable4 = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::processHeapPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapNextFreeAddressPointer = nullptr;
thread_local ValueStaticStorage* BuildInitialAssembly::copperHeapSizePointer = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_exit = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_print = nullptr;
thread_local FunctionStaticStorage* BuildInitialAssembly::Main_str = nullptr;
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
	Identifier globalInitName ("", 0, 0, pliers->allFiles->get(0));
	globalInit = new FunctionDefinition(
		CDataType::voidType,
		new Array<CVariableDefinition*>(),
		new Array<Statement*>(),
		&globalInitName);
	currentAssembly = (globalInit->instructions = new Array<AssemblyInstruction*>());
	currentOwningFunction = globalInit;
	forEach(Operator*, o, &initializationOrder, oi2) {
		addTokenAssembly(o, nullptr, nullptr);
	}
	currentAssembly->add(
		new CALL(
			globalTrackedStorage(new FunctionStaticStorage(pliers->mainFunction, cpuBitSize)),
			Register::aRegisterForBitSize(cpuBitSize)));
	currentAssembly->add(new RET(globalInit));

	//now assign temps
	//start with the functions that have no other function dependencies, then slowly go up, that way we can
	//	possibly preserve register values across function calls
	//if we get to a point where dependencies are cyclic, just pick one and assume all registers are used
	Array<FunctionDefinition*>* functionsToAssignTempsTo = new Array<FunctionDefinition*>();
	functionsToAssignTempsTo->add(globalInit);
	forEach(FunctionStaticStorage*, fss, functionDefinitions, fssi) {
		functionsToAssignTempsTo->add(fss->val);
	}
	while (functionsToAssignTempsTo->length > 0) {
		bool foundFunctionToAssignTempsTo = false;
		for (int i = functionsToAssignTempsTo->length - 1; i >= 0; i--) {
			FunctionDefinition* f = functionsToAssignTempsTo->get(i);
			Array<FunctionDefinition*>* dependencies = f->tempAssignmentDependencies;
			while (true) {
				//all the dependencies have assigned temps so assign temps for this function
				if (dependencies->length == 0) {
					assignTemps(f);
					functionsToAssignTempsTo->remove(i);
					foundFunctionToAssignTempsTo = true;
					break;
				//we still have dependencies but the last one has assigned temps so keep going
				} else if (dependencies->last()->registersUsed != nullptr)
					dependencies->pop();
				//we have a dependency that hasn't assign temps so don't do this one yet
				else
					break;
			}
		}
		//if we didn't find a function without dependencies, assign temps on the last one
		if (!foundFunctionToAssignTempsTo)
			assignTemps(functionsToAssignTempsTo->pop());
	}
	delete functionsToAssignTempsTo;

	//????????????????????????????

	//TODO: modify memory pointers with stack shifts
	//TODO: get the actual assembly bytes
	//TODO: optimize the assembly
	//	-remove unnecessary double jumps from boolean conditions
	//	-assign temps and remove MOVs where possible (same register or same memory address)
	//TODO: write the file

	cleanupAssemblyObjects();
}
//initialize everything used for building
void BuildInitialAssembly::setupAssemblyObjects() {
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
	generalPurposeVariable1 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable2 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable3 = new ValueStaticStorage(cpuBitSize);
	generalPurposeVariable4 = new ValueStaticStorage(cpuBitSize);
	processHeapPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapNextFreeAddressPointer = new ValueStaticStorage(cpuBitSize);
	copperHeapSizePointer = new ValueStaticStorage(cpuBitSize);
	if (cpuBitSize == BitSize::B32)
		build32BitMainFunctions();
}
//delete everything used for building
void BuildInitialAssembly::cleanupAssemblyObjects() {
	delete globalInit;
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
	Array<AssemblyInstruction*>* Main_exitAssembly = (currentAssembly = new Array<AssemblyInstruction*>());
	Main_exitAssembly->add(new SUB(cpuSPRegister, constant32Bit4));
	Main_exitAssembly->add(new MOV(cpuARegister, Main_exitIParameter->storage));
	Main_exitAssembly->add(new MOV(argumentStorage1, cpuARegister));
	Main_exitAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TExitProcess, BitSize::B32)), nullptr));
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
	//make space for WriteFile parameters
	Main_printAssembly->add(new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(20, BitSize::B32))));
	//get the StdOutputHandle
	Main_printAssembly->add(new SUB(cpuSPRegister, constant32Bit4));
	Main_printAssembly->add(
		new MOV(argumentStorage1, globalTrackedStorage(new AssemblyConstant(STD_OUTPUT_HANDLE, BitSize::B32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TGetStdHandle, BitSize::B32)), cpuARegister));
	//load all the parameters for WriteFile
	//argument 1: STD_OUTPUT_HANDLE in cpuARegister
	Main_printAssembly->add(new MOV(argumentStorage1, cpuARegister));
	//put the string address in cpuARegister
	Main_printAssembly->add(new MOV(cpuARegister, Main_printSParameter->storage));
	//put the char array address in cpuARegister
	Main_printAssembly->add(new MOV(cpuARegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32))));
	//put the address of the first char in cpuCRegister
	Main_printAssembly->add(new LEA(cpuCRegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 8, BitSize::B32))));
	//argument 2: address of string to print
	Main_printAssembly->add(new MOV(argumentStorage2, cpuCRegister));
	//put the length of the string in cpuCRegister
	Main_printAssembly->add(new MOV(cpuCRegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32))));
	//argument 3: length of string
	Main_printAssembly->add(new MOV(argumentStorage3, cpuCRegister));
	//argument 4: int address to write number of bytes written
	Main_printAssembly->add(
		new MOV(argumentStorage4, globalTrackedStorage(new StaticAddress(generalPurposeVariable1, BitSize::B32))));
	//argument 5: null OVERLAPPED pointer
	Main_printAssembly->add(new MOV(argumentStorage5, globalTrackedStorage(new AssemblyConstant(0, BitSize::B32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TWriteFile, BitSize::B32)), cpuARegister));
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
	Main_strAssembly->add(new REPMOVSB(cpuBitSize));
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
	GroupToken* g;
	IntConstant* ic;
	FloatConstant* fcn;
	BoolConstant* b;
	StringLiteral* sl;
	if (let(Identifier*, i, t))
		return getIdentifierStorage(i, false);
	else if (let(DirectiveTitle*, d, t))
		//TODO: handle directive titles
		;
	else if (let(Operator*, o, t)) {
		if (let(Cast*, c, t))
			return addCastAssembly(c);
		else if (let(StaticOperator*, so, t))
			return getStaticOperatorStorage(so);
		else
			return addOperatorAssembly(o, jumpDests);
	} else if (let(FunctionCall*, fc, t))
		return addFunctionCallAssembly(fc);
	else if (let(FunctionDefinition*, fd, t))
		return getFunctionDefinitionStorage(fd, false);
	else if (let(GroupToken*, g, t))
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
AssemblyStorage* BuildInitialAssembly::getIdentifierStorage(Identifier* i, bool isBeingFunctionCalled) {
	FunctionDefinition* f;
	if (let(FunctionDefinition*, f, i->variable->initialValue)) {
		//add this function as a temp assignment dependency if we haven't already
		if (currentTempAssignmentDependencies != nullptr)
			currentTempAssignmentDependencies->addNonDuplicate(f);
		//if we ever read a function variable or write to it, it's not eligible for register parameters
		if (!isBeingFunctionCalled || i->variable->writtenTo)
			f->eligibleForRegisterParameters = false;
		//otherwise we can directly get the function definition's storage
		else
			return getFunctionDefinitionStorage(f, isBeingFunctionCalled);
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
Register* BuildInitialAssembly::addOperatorAssembly(Operator* o, ConditionLabelPair* jumpDests) {
	VariableDeclarationList* v = nullptr;
	CDataType* expectedType = o->precedence == OperatorTypePrecedence::Comparison
		? CDataType::bestCompatibleType(o->left->dataType, o->right->dataType)
		: o->dataType;
	//operators were rewritten to be postfix so the left token will always be present
	CDataType* leftExpectedType = typeBitSize(o->left->dataType) == BitSize::BInfinite ? expectedType : nullptr;
	CDataType* rightExpectedType =
		o->right != nullptr && typeBitSize(o->right->dataType) == BitSize::BInfinite ? expectedType : nullptr;
	AssemblyStorage* leftStorage = nullptr;
	AssemblyStorage* rightStorage = nullptr;
	Register* resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(typeBitSize(expectedType)));
	//before we get the assembly for the left and right storages, we need to check for a few boolean comparison cases
	//if we do have jump destinations we'll return early
	if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
		bool andOperator = o->operatorType == OperatorType::BooleanAnd;
		AssemblyLabel* continueBooleanLabel = new AssemblyLabel();
		AssemblyLabel* resultFinishedLabel;
		ConditionLabelPair* booleanJumpDests;
		if (andOperator) {
			resultFinishedLabel = jumpDests != nullptr ? jumpDests->falseJumpDest : new AssemblyLabel();
			booleanJumpDests = new ConditionLabelPair(continueBooleanLabel, resultFinishedLabel);
		} else {
			resultFinishedLabel = jumpDests != nullptr ? jumpDests->trueJumpDest : new AssemblyLabel();
			booleanJumpDests = new ConditionLabelPair(resultFinishedLabel, continueBooleanLabel);
		}
		//get the left assembly
		leftStorage = addTokenAssembly(o->left, leftExpectedType, booleanJumpDests);
		//make sure we have a jump to our destination
		if (jumpDests != nullptr) {
			if (leftStorage != nullptr)
				addBooleanJump(leftStorage, booleanJumpDests);
		//make sure we have a value
		} else {
			//we got a value, jump if we don't need to check the rest of the boolean
			if (leftStorage != nullptr) {
				currentAssembly->add(new MOV(resultStorage, leftStorage));
				addBooleanJump(resultStorage, booleanJumpDests);
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
		delete booleanJumpDests;
		//add the right side normally
		currentAssembly->add(continueBooleanLabel);
		addConditionAssembly(o->right, resultStorage, rightExpectedType, jumpDests);
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
		addConditionAssembly(o->left, nullptr, leftExpectedType, &ternaryJumpDests);
		Operator* ternaryResult = dynamic_cast<Operator*>(o->right);
		if (ternaryResult == nullptr) {
			Error::logError(ErrorType::CompilerIssue, "getting the results of this ternary operator", o);
			return cpuARegister;
		}
		//get the true and false assembly, using the jump dests if they were provided
		currentAssembly->add(ternaryTrueLabel);
		addConditionAssembly(ternaryResult->left, resultStorage, rightExpectedType, jumpDests);
		if (jumpDests == nullptr)
			currentAssembly->add(new JMP(ternaryAfterLabel));
		currentAssembly->add(ternaryFalseLabel);
		addConditionAssembly(ternaryResult->right, resultStorage, rightExpectedType, jumpDests);
		currentAssembly->add(ternaryAfterLabel);
		return jumpDests != nullptr ? nullptr : resultStorage;
	} else if (o->operatorType == OperatorType::LogicalNot && jumpDests != nullptr) {
		ConditionLabelPair reversedJumpDests (jumpDests->falseJumpDest, jumpDests->trueJumpDest);
		addConditionAssembly(o->left, nullptr, leftExpectedType, &reversedJumpDests);
		return nullptr;
	}
	bool leftStorageIsAddress = false;
	//anything but a variable initialization means that we need to get the assembly for the left token
	if (o->operatorType != OperatorType::Assign || !let(VariableDeclarationList*, v, o->left)) {
		AssemblyStorage* originalLeftStorage = addTokenAssembly(o->left, leftExpectedType, nullptr);
		//it modifies a variable so the left storage will be a destination
		if (o->modifiesVariable) {
			MemoryPointer* leftMemoryPointer;
//TODO: how do we make sure the GC doesn't collect the array before we write to it?
//we need to make sure the array value itself is available somewhere
//void(int[][] array1) ( array1[0] = new int[1000000000]; );
//	-maybe arrays must be on the stack?
//	-RetainedAddress, stores a derived address and also the storage of the value where it came from, for arrays, Groups, etc.
			//if it's a memory pointer, save its address in a temp
			if (let(MemoryPointer*, leftMemoryPointer, originalLeftStorage)) {
				Register* transferStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
				leftStorage = globalTrackedStorage(new TempStorage(cpuBitSize));
				currentAssembly->add(new LEA(transferStorage, leftMemoryPointer));
				currentAssembly->add(new MOV(leftStorage, transferStorage));
				leftStorageIsAddress = true;
			//anything else (necessarily a temp) can stay how it is
			} else
				leftStorage = originalLeftStorage;
		//it doesn't modify a variable so it's just a value
		//if it's a register or memory pointer, put it in a temp to ensure it could be a stack value if it needs to be
		} else {
			if (istype(originalLeftStorage, Register*)) {
				leftStorage = globalTrackedStorage(new TempStorage(originalLeftStorage->bitSize));
				currentAssembly->add(new MOV(leftStorage, originalLeftStorage));
			} else if (istype(originalLeftStorage, MemoryPointer*)) {
				leftStorage = globalTrackedStorage(new TempStorage(originalLeftStorage->bitSize));
				addMemoryToMemoryMove(leftStorage, originalLeftStorage);
			//if it's another temp or some constant storage, we don't need to do anything with it
			} else
				leftStorage = originalLeftStorage;
		}
	}
	rightStorage = o->right != nullptr ? addTokenAssembly(o->right, rightExpectedType, nullptr) : nullptr;
	//if the left storage is a value, load it into the result
	if (o->operatorType != OperatorType::Assign) {
		//if we stored an address, we need to that and then get the value
		if (leftStorageIsAddress) {
			Register* transferStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
			currentAssembly->add(new MOV(transferStorage, leftStorage));
			currentAssembly->add(
				new MOV(resultStorage, globalTrackedStorage(new MemoryPointer(transferStorage, 0, resultStorage->bitSize))));
		} else
			currentAssembly->add(new MOV(resultStorage, leftStorage));
	}
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
			//we needed to use the *A* register to hold the dividend
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
				currentAssembly->add(new MOV(clRegister, rightStorage));
				currentAssembly->add(new SHL(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftRight:
		case OperatorType::AssignShiftRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentAssembly->add(new SHR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentAssembly->add(new MOV(clRegister, rightStorage));
				currentAssembly->add(new SHR(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftArithmeticRight:
		case OperatorType::AssignShiftArithmeticRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentAssembly->add(new SAR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentAssembly->add(new MOV(clRegister, rightStorage));
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
		} else {
			//if it was an address, store it there
			if (leftStorageIsAddress) {
				Register* transferRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
				currentAssembly->add(new MOV(transferRegister, leftStorage));
				currentAssembly->add(
					new MOV(
						globalTrackedStorage(new MemoryPointer(transferRegister, 0, resultStorage->bitSize)),
						resultStorage));
			} else
				currentAssembly->add(new MOV(leftStorage, resultStorage));
		}
	}
	return resultStorage;
}
//get the assembly for the arguments and the function and call it, returning the value
AssemblyStorage* BuildInitialAssembly::addFunctionCallAssembly(FunctionCall* f) {
	//get the function
	AssemblyStorage* functionStorage;
	FunctionStaticStorage* functionStaticStorage;
	FunctionDefinition* fd;
	if (let(FunctionDefinition*, fd, f->function))
		functionStorage = (functionStaticStorage = getFunctionDefinitionStorage(fd, true));
	else {
		Identifier* i;
		functionStorage = let(Identifier*, i, f->function)
			? getIdentifierStorage(i, true)
			: addTokenAssembly(f->function, nullptr, nullptr);
		//if we have a function static storage, we can use the function directly
		if (let(FunctionStaticStorage*, functionStaticStorage, functionStorage))
			fd = functionStaticStorage->val;
		//if we don't have static function storage, store it in a temp while we compute our arguments
		else {
			fd = nullptr;
			//it's not a temp, put it in one
			if (!istype(functionStorage, TempStorage*)) {
				TempStorage* functionTempStorage = globalTrackedStorage(new TempStorage(cpuBitSize));
				addMemoryToMemoryMove(functionTempStorage, functionStorage);
				functionStorage = functionTempStorage;
			}
		}
	}
	//then go through and add all the arguments
	//if we have a function definition, we can use its temps to save arguments, and return its result storage
	//sometime later we will adjust or remove the SUB for the stack if we were able to use register parameters
	if (fd != nullptr) {
		currentAssembly->add(new StackShift(currentOwningFunction, fd, 0, true));
		for (int i = 0; i < f->arguments->length; i++) {
			CVariableDefinition* vd = fd->parameters->get(i);
			addMemoryToMemoryMove(vd->storage, addTokenAssembly(f->arguments->get(i), vd->type, nullptr));
		}
		currentAssembly->add(new CALL(functionStaticStorage, fd->resultStorage));
		currentAssembly->add(new StackShift(currentOwningFunction, fd, 0, false));
		return fd->resultStorage;
	//if not, then they will have to be on the stack and return in the *A* register
	//get the positions that they will be
	} else {
		//first make sure we have the function
		CSpecificFunction* sf = dynamic_cast<CSpecificFunction*>(f->function->dataType);
		if (sf == nullptr) {
			Error::logError(ErrorType::CompilerIssue, "determining the signature of this function", f->function);
			return cpuARegister;
		}
		//find out how much we need to shift the stack and shift it
		int shiftTotalBits = 0;
		Array<BitSize> argumentBitSizes;
		forEach(Token*, t, f->arguments, ti) {
			BitSize b = typeBitSize(t->dataType);
			shiftTotalBits += Math::max(8, (int)((unsigned char)b));
			argumentBitSizes.add(b);
		}
		int shiftTotalBytes = ((shiftTotalBits + (int)((unsigned char)cpuBitSize - 1)) / 8) & 0xFFFFFFFC;
		currentAssembly->add(new StackShift(currentOwningFunction, nullptr, shiftTotalBytes, true));
		Array<int>* argumentStackOffsets = getParameterStackOffsets(&argumentBitSizes);
		//then get all the arguments
		for (int i = 0; i < argumentStackOffsets->length; i++) {
			Token* argument = f->arguments->get(i);
			AssemblyStorage* argumentStorage = addTokenAssembly(argument, sf->parameterTypes->get(i), nullptr);
			addMemoryToMemoryMove(
				globalTrackedStorage(
					new MemoryPointer(cpuSPRegister, 0, nullptr, argumentStackOffsets->get(i), true, argumentStorage->bitSize)),
				argumentStorage);
		}
		delete argumentStackOffsets;
		Register* functionStorageRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
		Register* resultRegister = Register::aRegisterForBitSize(typeBitSize(sf->returnType));
		currentAssembly->add(new MOV(functionStorageRegister, functionStorage));
		currentAssembly->add(
			new CALL(
				globalTrackedStorage(new MemoryPointer(functionStorageRegister, (unsigned char)cpuBitSize / 8, cpuBitSize)),
				resultRegister));
		currentAssembly->add(new StackShift(currentOwningFunction, nullptr, shiftTotalBytes, false));
		return resultRegister;
	}
}
//get the storage of the function definition
FunctionStaticStorage* BuildInitialAssembly::getFunctionDefinitionStorage(FunctionDefinition* f, bool isBeingFunctionCalled) {
	//if it's part of an expression (like a ternary), it's not eligible for register parameters
	if (!isBeingFunctionCalled)
		f->eligibleForRegisterParameters = false;
	FunctionStaticStorage* fss = new FunctionStaticStorage(f, cpuBitSize);
	if (f->instructions == nullptr) {
		Array<AssemblyInstruction*>* oldCurrentAssembly = currentAssembly;
		FunctionDefinition* oldOwningFunction = currentOwningFunction;
		Array<FunctionDefinition*>* oldCurrentTempAssignmentDependencies = currentTempAssignmentDependencies;
		currentAssembly = (f->instructions = new Array<AssemblyInstruction*>());
		currentOwningFunction = f;
		currentTempAssignmentDependencies = f->tempAssignmentDependencies;
		if (f->dataType != CDataType::voidType) {
			f->resultStorage = Register::newUndecidedRegisterForBitSize(typeBitSize(f->dataType));
			if (!f->eligibleForRegisterParameters)
				f->resultStorage->specificRegister = Register::aRegisterForBitSize(f->resultStorage->bitSize)->specificRegister;
		}
		addStatementListAssembly(f->body);
		if (f->dataType == CDataType::voidType)
			f->instructions->add(new RET(f));
		currentAssembly = oldCurrentAssembly;
		currentOwningFunction = oldOwningFunction;
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
void BuildInitialAssembly::addStatementListAssembly(Array<Statement*>* statements) {
	forEach(Statement*, s, statements, si) {
		ExpressionStatement* e;
		ReturnStatement* r;
		IfStatement* i;
		LoopStatement* l;
		if (let(ExpressionStatement*, e, s)) {
			if (!istype(e->expression, VariableDeclarationList*))
				addTokenAssembly(e->expression, nullptr, nullptr);
		} else if (let(ReturnStatement*, r, s)) {
			if (r->expression != nullptr) {
				AssemblyStorage* resultStorage = addTokenAssembly(r->expression, currentOwningFunction->returnType, nullptr);
				currentAssembly->add(new MOV(currentOwningFunction->resultStorage, resultStorage));
			}
			currentAssembly->add(new RET(currentOwningFunction));
		} else if (let(IfStatement*, i, s)) {
			AssemblyLabel* thenLabel = new AssemblyLabel();
			AssemblyLabel* elseLabel = new AssemblyLabel();
			ConditionLabelPair ifJumpDests (thenLabel, elseLabel);
			addConditionAssembly(i->condition, nullptr, nullptr, &ifJumpDests);
			currentAssembly->add(thenLabel);
			addStatementListAssembly(i->thenBody);
			if (i->elseBody != nullptr) {
				AssemblyLabel* ifAfterLabel = new AssemblyLabel();
				currentAssembly->add(new JMP(ifAfterLabel));
				currentAssembly->add(elseLabel);
				addStatementListAssembly(i->elseBody);
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
			if (l->initialConditionCheck)
				addConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentAssembly->add(loopStartLabel);
			//TODO: pass this loop for breaks and continues
			addStatementListAssembly(l->body);
			currentAssembly->add(loopContinueLabel);
			if (l->increment != nullptr)
				addTokenAssembly(l->increment, nullptr, nullptr);
			addConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentAssembly->add(loopBreakLabel);
		}
		//TODO: breaks and continues
	}
}
//add two MOVs to transfer from (possible) memory pointer to (possible) memory pointer via a transfer register
//if the source is a register, we can use that one, if not then move it to a transfer register
//returns the register used in the transfer
Register* BuildInitialAssembly::addMemoryToMemoryMove(AssemblyStorage* destination, AssemblyStorage* source) {
	Register* transferRegister;
	if (!let(Register*, transferRegister, source)) {
		transferRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(destination->bitSize));
		currentAssembly->add(new MOV(transferRegister, source));
	}
	currentAssembly->add(new MOV(destination, transferRegister));
	return transferRegister;
}
//get the assembly for a condition
//if we have jump destinations, ensure we jump to the right place
//if we do not, ensure resultStorage has the value of the expression
void BuildInitialAssembly::addConditionAssembly(
	Token* t, Register* resultStorage, CDataType* expectedType, ConditionLabelPair* jumpDests)
{
	AssemblyStorage* valueStorage = addTokenAssembly(t, expectedType, jumpDests);
	if (jumpDests != nullptr) {
		if (valueStorage != nullptr)
			addBooleanJump(valueStorage, jumpDests);
	} else if (resultStorage != nullptr && valueStorage != nullptr)
		currentAssembly->add(new MOV(resultStorage, valueStorage));
	else
		Error::logError(ErrorType::CompilerIssue, "getting assembly for the boolean value of this token", t);
}
//add instructions to jump based on whether the source is non-zero
//if the source isn't a register or temp storage, create a temp to use in the comparison
void BuildInitialAssembly::addBooleanJump(AssemblyStorage* source, ConditionLabelPair* jumpDests) {
	AssemblyConstant* zeroConstant = globalTrackedStorage(new AssemblyConstant(0, source->bitSize));
	AssemblyConstant* a;
	if (let(AssemblyConstant*, a, source))
		//we can just do an unconditional jump instead of comparing constants (used for things like infinite loops)
		currentAssembly->add(new JMP(a->val != 0 ? jumpDests->trueJumpDest : jumpDests->falseJumpDest));
	else {
		currentAssembly->add(new CMP(source, zeroConstant));
		currentAssembly->add(new JNE(jumpDests->trueJumpDest));
		currentAssembly->add(new JMP(jumpDests->falseJumpDest));
	}
}
//assign registers or memory pointers to all temps of the provided instructions
void BuildInitialAssembly::assignTemps(FunctionDefinition* source) {
	Array<AssemblyInstruction*>* instructions = source->instructions;
	//first things first- give each instruction an index and an empty list of storages used
	Array<Array<AssemblyStorage*>*> storagesUsed;
	for (int i = instructions->length - 1; i >= 0; i--) {
		instructions->get(i)->instructionArrayIndex = i;
		storagesUsed.add(new Array<AssemblyStorage*>());
	}

	AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*> potentialSameStoragesByStorage;
	Array<CALL*> calls;
	trackStoragesUsed(instructions, &storagesUsed, &potentialSameStoragesByStorage, &calls);

	InsertionOrderedAVLTree<AssemblyStorage*, Array<AssemblyStorage*>*> conflictsByStorage;
	markConflicts(&storagesUsed, &conflictsByStorage, &calls, source);

	//make sure we won't try to match storage to another storage that conflicts with it
	Array<AVLNode<AssemblyStorage*, Array<AssemblyStorage*>*>*>* storagesWithPotentialSameStorages =
		potentialSameStoragesByStorage.entrySet();
	forEach(
		AVLNode<AssemblyStorage* COMMA Array<AssemblyStorage*>*>*,
		storageWithPotentialSameStorages,
		storagesWithPotentialSameStorages,
		dontMatchI)
	{
		Array<AssemblyStorage*>* conflicts = conflictsByStorage.get(storageWithPotentialSameStorages->key);
		if (conflicts == nullptr)
			continue;
		Array<AssemblyStorage*>* potentialSameStorages = storageWithPotentialSameStorages->value;
		forEach(AssemblyStorage*, conflict, conflicts, ci) {
			potentialSameStorages->removeItem(conflict);
		}
	}

	//if our function definition isn't eligible for register parameters, assign it stack pointers now
	if (!source->eligibleForRegisterParameters) {
		Array<BitSize> argumentBitSizes;
		forEach(CVariableDefinition*, c, source->parameters, ci) {
			argumentBitSizes.add(c->storage->bitSize);
		}
		Array<int>* parameterStackOffsets = getParameterStackOffsets(&argumentBitSizes);
		for (int i = 0; i < parameterStackOffsets->length; i++)
			source->parameters->get(i)->storage->finalStorage =
				new MemoryPointer(cpuSPRegister, parameterStackOffsets->get(i), argumentBitSizes.get(i));
		delete parameterStackOffsets;
	}

	//now we have a complete list of conflicts
	//find all the temps and undecided registers to assign
	Array<AssemblyStorage*> storagesToAssign;
	Array<AssemblyStorage*> storagesToAssignWithGuaranteedRegister;
	Array<TempStorage*> tempsToAssignOnStack;
	forEach(AssemblyStorage*, storage, conflictsByStorage.insertionOrder, insertionOrderedConflictsI) {
		Register* r;
		if (istype(storage, TempStorage*)
				|| (let(Register*, r, storage)
					&& (r->specificRegister == SpecificRegister::Undecided8BitRegister
						|| r->specificRegister == SpecificRegister::Undecided16BitRegister
						|| r->specificRegister == SpecificRegister::Undecided32BitRegister)))
			storagesToAssign.add(storage);
	}
	//then go through all the temps and assign registers where possible
	int minStackOffset = 0;
	bool conflictRegisters[(unsigned char)ConflictRegister::ConflictRegisterCount];
	while (storagesToAssign.length > 0) {
		//first, see if there are any storages that share a MOV with an assigned register
		bool foundSameStorage = false;
		for (int i = 0; i < storagesWithPotentialSameStorages->length; i++) {
			AVLNode<AssemblyStorage*, Array<AssemblyStorage*>*>* storageWithPotentialSameStorages =
				storagesWithPotentialSameStorages->get(i);
			AssemblyStorage* storageToAssign = storageWithPotentialSameStorages->key;
			setConflictRegisters(conflictRegisters, conflictsByStorage.get(storageToAssign));
			forEach(AssemblyStorage*, potentialSameStorage, storageWithPotentialSameStorages->value, pi) {
				Register* otherRegister;
				TempStorage* otherTemp;
				//we found a register with a known specific register which does not conflict with this register
				if ((let(Register*, otherRegister, potentialSameStorage)
						|| (let(TempStorage*, otherTemp, potentialSameStorage)
							&& let(Register*, otherRegister, otherTemp->finalStorage)))
					&& !otherRegister->isConflictOrUnknown(conflictRegisters))
				{
					setStorageToRegister(storageToAssign, otherRegister->specificRegister, source);
					foundSameStorage = true;
					storagesWithPotentialSameStorages->remove(i);
					storagesToAssign.removeItem(storageToAssign);
					i--;
					break;
				}
			}
		}
		if (foundSameStorage)
			continue;

		//we did not find any storages with a clear register to give them
		//go through our storages, and either
		//	-mark them as definitely able to be a register
		//	-mark them as definitely having to go on the stack
		//	-give them a register
		AssemblyStorage* nextStorageToAssign = nullptr;
		int nextStorageToAssignIndex = -1;
		ConflictRegister nextFirstAvailableRegister = ConflictRegister::ConflictRegisterCount;
		int nextUndecidedConflictsCount = 0;
		int assignedStoragesCount = 0;
		for (int i = 0; i < storagesToAssign.length; i++) {
			//shift entries backwards as we iterate instead of removing them from the middle to save time
			AssemblyStorage* storageToAssign = storagesToAssign.get(i);
			storagesToAssign.set(i - assignedStoragesCount, storageToAssign);

			int undecidedConflictsCount = setConflictRegisters(conflictRegisters, conflictsByStorage.get(storageToAssign));
			Group2<ConflictRegister, int> firstAvailableRegisterAndTakenRegisterConflicts =
				getFirstAvailableRegister(storageToAssign->bitSize, conflictRegisters);

			//we found a storage that can definitely be a register without conflict, save it for later
			if (undecidedConflictsCount + firstAvailableRegisterAndTakenRegisterConflicts.val2 < 8) {
				storagesToAssignWithGuaranteedRegister.add(storageToAssign);
				assignedStoragesCount++;
				continue;
			}

			//if we can't give it a register, save it for later
			ConflictRegister firstAvailableRegister = firstAvailableRegisterAndTakenRegisterConflicts.val1;
			if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount) {
				TempStorage* tempToAssign;
				if (let(TempStorage*, tempToAssign, storageToAssign))
					tempsToAssignOnStack.add(tempToAssign);
				else {
					Error::logError(ErrorType::CompilerIssue, "finding assembly stack storage for variables", source);
					return;
				}
				assignedStoragesCount++;
				continue;
			}

			//we found a storage that has too many conflicts to be certain we can give it a register,
			//	but we might be able to
			//track whichever one has the lowest-index register, or on a tie whichever one has more undecided conflicts
			//but don't replace an undecided register with an undecided temp
			if ((istype(storageToAssign, Register*) || !istype(nextStorageToAssign, Register*))
				&& ((unsigned char)firstAvailableRegister < (unsigned char)nextFirstAvailableRegister
					|| (firstAvailableRegister == nextFirstAvailableRegister
						&& undecidedConflictsCount > nextUndecidedConflictsCount)))
			{
				nextStorageToAssign = storageToAssign;
				nextStorageToAssignIndex = i;
				nextFirstAvailableRegister = firstAvailableRegister;
				nextUndecidedConflictsCount = undecidedConflictsCount;
			}
		}
		if (assignedStoragesCount > 0)
			storagesToAssign.remove(storagesToAssign.length - assignedStoragesCount, assignedStoragesCount);
		//we found a register that needs assigning, so assign it
		if (nextStorageToAssignIndex >= 0) {
			setStorageToRegister(
				nextStorageToAssign,
				Register::specificRegisterFor(nextFirstAvailableRegister, nextStorageToAssign->bitSize), source);
			storagesToAssign.remove(nextStorageToAssignIndex);
		}
	}

	//we finished assigning all the storages we weren't sure about
	//give registers to all the storages that can definitely have them
	forEach(AssemblyStorage*, storageToAssign, &storagesToAssignWithGuaranteedRegister, guaranteedRegisterI) {
		setConflictRegisters(conflictRegisters, conflictsByStorage.get(storageToAssign));
		ConflictRegister firstAvailableRegister = getFirstAvailableRegister(storageToAssign->bitSize, conflictRegisters).val1;
		if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount) {
			Error::logError(ErrorType::CompilerIssue, "finding final register storage for variables", source);
			return;
		}
		setStorageToRegister(
			storageToAssign, Register::specificRegisterFor(firstAvailableRegister, storageToAssign->bitSize), source);
	}
	//give stack memory pointers to all the temps that need to be on the stack
	int cpuByteSize = (int)((unsigned char)cpuBitSize / 8);
	int lowestStackOffset = 0;
	forEach(TempStorage*, tempToAssign, &tempsToAssignOnStack, ti) {
		Array<AssemblyStorage*>* tempConflicts = conflictsByStorage.get(tempToAssign);
		//for convenience, first extract any temp internal storages
		for (int i = 0; i < tempConflicts->length; i++) {
			TempStorage* tempConflict;
			if (let(TempStorage*, tempConflict, tempConflicts->get(i)) && tempConflict->finalStorage != nullptr)
				tempConflicts->set(i, tempConflict->finalStorage);
		}

		//now go through all storages and find the first free stack offset
		int nextStackOffset = -cpuByteSize;
		for (int i = tempConflicts->length - 1; true; i--) {
			MemoryPointer* memoryPointerConflict;
			//we passed all the conflicts and none were stack variables at this offset, so use a memory pointer here
			if (i == 0) {
				tempToAssign->finalStorage = new MemoryPointer(cpuSPRegister, nextStackOffset, tempToAssign->bitSize);
				lowestStackOffset = Math::min(lowestStackOffset, nextStackOffset);
				break;
			//we found a memory pointer at this stack offset, restart the loop but bump the stack offset
			} else if (let(MemoryPointer*, memoryPointerConflict, tempConflicts->get(i))
				&& memoryPointerConflict->constant == nextStackOffset)
			{
				i = tempConflicts->length;
				nextStackOffset -= cpuByteSize;
			}
		}
	}
	source->stackBytesUsed = -lowestStackOffset;

	//finally, go through all the storages and mark registers as used
	Array<SpecificRegister>* registersUsed = new Array<SpecificRegister>();
	forEach(AssemblyStorage*, storage, conflictsByStorage.insertionOrder, usedRegistersI) {
		Register* registerStorage;
		TempStorage* tempStorage;
		if (let(Register*, registerStorage, storage)
				|| (let(TempStorage*, tempStorage, storage) && let(Register*, registerStorage, tempStorage->finalStorage)))
			registersUsed->addNonDuplicate(registerStorage->specificRegister);
	}
	//in the event that we have a void function that takes 1 parameter that's unused, it will never have gotten a final storage
	//give it an *A* register if we can, or the first stack register if we can't
	if (source->parameters->length == 1 && source->parameters->get(0)->storage->finalStorage == nullptr) {
		TempStorage* firstParameterStorage = source->parameters->get(0)->storage;
		firstParameterStorage->finalStorage = source->eligibleForRegisterParameters
			? (AssemblyStorage*)Register::aRegisterForBitSize(firstParameterStorage->bitSize)
			: (AssemblyStorage*)new MemoryPointer(cpuSPRegister, -cpuByteSize, firstParameterStorage->bitSize);
	}

	source->registersUsed = registersUsed;

	storagesUsed.deleteContents();
	forEach(
			AVLNode<AssemblyStorage* COMMA Array<AssemblyStorage*>*>*,
			storageWithPotentialSameStorages,
			storagesWithPotentialSameStorages,
			deletePotentialSameStoragesI)
		delete storageWithPotentialSameStorages->value;
	delete storagesWithPotentialSameStorages;
	Array<AVLNode<AssemblyStorage*, Array<AssemblyStorage*>*>*>* storagesWithConflicts =
		conflictsByStorage.entrySet();
	forEach(
			AVLNode<AssemblyStorage* COMMA Array<AssemblyStorage*>*>*,
			storageWithConflicts,
			storagesWithConflicts,
			deleteConflictsI)
		delete storageWithConflicts->value;
	delete storagesWithConflicts;
}
//go through the instructions and track the of storages, per instruction, that need to be alive before the instruction executes
//assumes storagesUsed has already been filled with one array per instruction
void BuildInitialAssembly::trackStoragesUsed(
	Array<AssemblyInstruction*>* instructions,
	Array<Array<AssemblyStorage*>*>* storagesUsed,
	AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* potentialSameStoragesByStorage,
	Array<CALL*>* calls)
{
	//the last instruction is always a RET, start at the index before it
	int lastIndex = instructions->length - 2;
	int latestBackwrdsJumpIndex = 0;
	int earliestBackwardsJumpIndex = instructions->length;
	for (int i = lastIndex; i >= 0; i--) {
		AssemblyInstruction* instruction = instructions->get(i);
		Array<AssemblyStorage*>* instructionStoragesUsed = storagesUsed->get(i);
		JumpInstruction* j;
		//jump instruction, pull in storages from the possible next instructions
		if (let(JumpInstruction*, j, instruction)) {
			int jumpIndex = j->jumpDestination->instructionArrayIndex;
			//start by pulling the storages of the label
			instructionStoragesUsed->add(storagesUsed->get(jumpIndex));
			//if we're on a conditional jump, union with the storages of the following instruction
			if (!istype(j, JMP*))
				//assumes that a JMP will never be the last instruction
				instructionStoragesUsed->addNonDuplicates(storagesUsed->get(i + 1));
			//we did a backwards jump, set the range of instructions that we need to modify
			if (jumpIndex < i) {
				if (latestBackwrdsJumpIndex == 0)
					latestBackwrdsJumpIndex = i;
				if (jumpIndex < earliestBackwardsJumpIndex)
					earliestBackwardsJumpIndex = jumpIndex;
			}
		//for any other instruction, find which storages are used before this instruction
		} else {
			instructionStoragesUsed->add(storagesUsed->get(i + 1));
			instruction->removeDestinations(instructionStoragesUsed);
			instruction->addSources(instructionStoragesUsed);
			CALL* c;
			MOV* m;
			//move, track the destination and source as potentially the same storage
			if (let(MOV*, m, instruction)) {
				storagesListFor(potentialSameStoragesByStorage, m->destination)->addNonDuplicate(m->source);
				storagesListFor(potentialSameStoragesByStorage, m->source)->addNonDuplicate(m->destination);
			//call, later we will find out which registers conflict with our needed storages
			} else if (let(CALL*, c, instruction))
				calls->add(c);
		}
	}

	//if we have backwards jumps, we need to iterate through the list again
	bool storagesUsedHasChanged = latestBackwrdsJumpIndex > 0;
	while (storagesUsedHasChanged) {
		storagesUsedHasChanged = false;
		for (int i = latestBackwrdsJumpIndex; i >= earliestBackwardsJumpIndex; i--) {
			AssemblyInstruction* instruction = instructions->get(i);
			Array<AssemblyStorage*>* instructionStoragesUsed = storagesUsed->get(i);
			int oldStoragesUsedCount = instructionStoragesUsed->length;
			JumpInstruction* j;
			//jump instruction, pull in storages from the possible next instructions like before
			if (let(JumpInstruction*, j, instruction)) {
				int jumpIndex = j->jumpDestination->instructionArrayIndex;
				instructionStoragesUsed->addNonDuplicates(storagesUsed->get(jumpIndex));
				if (!istype(j, JMP*))
					instructionStoragesUsed->addNonDuplicates(storagesUsed->get(i + 1));
			} else
				//we know that this is not the last instruction
				instructionStoragesUsed->addNonDuplicates(storagesUsed->get(i + 1));
			if (instructionStoragesUsed->length > oldStoragesUsedCount)
				storagesUsedHasChanged = true;
		}
	}
}
//add conflicts for all storages that will exist at the same time
void BuildInitialAssembly::markConflicts(
	Array<Array<AssemblyStorage*>*>* storagesUsed,
	AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* conflictsByStorage,
	Array<CALL*>* calls,
	FunctionDefinition* source)
{
	//go through each storagesUsed list and conflict each register with each other register
	forEach(Array<AssemblyStorage*>*, storagesUsedForInstruction, storagesUsed, si) {
		for (int i = storagesUsedForInstruction->length - 1; i >= 0; i--) {
			Array<AssemblyStorage*>* conflictsForStorage =
				storagesListFor(conflictsByStorage, storagesUsedForInstruction->get(i));
			for (int j = storagesUsedForInstruction->length - 1; j >= 0; j--) {
				if (i != j)
					conflictsForStorage->addNonDuplicate(storagesUsedForInstruction->get(j));
			}
		}
	}
	//go through all the CALLs and conflict the storages with registers
	forEach(CALL*, c, calls, ci) {
		FunctionStaticStorage* f;
		Array<SpecificRegister>* registersUsed;
		//if we know exactly what registers we're using, we can just use those as conflicts
		if (let(FunctionStaticStorage*, f, c->source) && let(Array<SpecificRegister>*, registersUsed, f->val->registersUsed)) {
			//add conflicts for each storage
			forEach(AssemblyStorage*, storageUsed, storagesUsed->get(c->instructionArrayIndex), si2) {
				Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, storageUsed);
				forEach(SpecificRegister, s, registersUsed, si3) {
					conflictsForStorage->addNonDuplicate(Register::registerFor(s));
				}
			}
		//otherwise, use all registers as conflicts
		} else {
			unsigned char specificRegisterStart = (unsigned char)SpecificRegister::Register32BitStart;
			unsigned char specificRegisterEnd = (unsigned char)SpecificRegister::Register32BitEnd;
			//add conflicts for each storage
			forEach(AssemblyStorage*, storageUsed, storagesUsed->get(c->instructionArrayIndex), si2) {
				Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, storageUsed);
				for (unsigned char s = specificRegisterStart; s < specificRegisterEnd; s++) {
					conflictsForStorage->addNonDuplicate(Register::registerFor((SpecificRegister)s));
				}
			}
		}
	}
	//and also conflict all the parameters with each other
	forEach(CVariableDefinition*, c, source->parameters, ci2) {
		Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, c->storage);
		forEach(CVariableDefinition*, c2, source->parameters, ci3) {
			if (c != c2)
				conflictsForStorage->addNonDuplicate(c2->storage);
		}
	}
}
//set any conflicts from the given conflict storages
//assumes any conflicting temps have not yet been given stack storage (so that we can consider them undecided
//	if they don't have a register)
//returns the number of undecided storages in the list
int BuildInitialAssembly::setConflictRegisters(bool* conflictRegisters, Array<AssemblyStorage*>* conflictStorages) {
	//reset the conflicts since we reuse the list
	for (unsigned char i = 0; i < (unsigned char)ConflictRegister::ConflictRegisterCount; i++)
		conflictRegisters[i] = false;
	//SP is always a conflict
	conflictRegisters[(unsigned char)ConflictRegister::SP] = true;

	int conflictsCount = conflictStorages == nullptr ? 0 : conflictStorages->length;
	int undecidedConflictsCount = 0;
	for (int j = 0; j < conflictsCount; j++) {
		AssemblyStorage* other = conflictStorages->get(j);
		Register* otherRegister;
		TempStorage* otherTemp;
		if (let(Register*, otherRegister, other)) {
			if (!otherRegister->markConflicts(conflictRegisters))
				undecidedConflictsCount++;
		//if it's a temp with a final register storage, replace the value in the array
		//we're assuming that stack storages haven't been assigned yet
		} else if (let(TempStorage*, otherTemp, other)) {
			if (let(Register*, otherRegister, otherTemp->finalStorage)) {
				if (!otherRegister->markConflicts(conflictRegisters))
					undecidedConflictsCount++;
				//we know what it is now so replace it
				conflictStorages->set(j, otherRegister);
			} else
				undecidedConflictsCount++;
		}
	}
	return undecidedConflictsCount;
}
//find the first available register that doesn't conflict with the given registers
//returns the first available register as well as the number of conflicting registers
Group2<ConflictRegister, int> BuildInitialAssembly::getFirstAvailableRegister(BitSize bitSize, bool* conflictRegisters) {
	ConflictRegister firstAvailableRegister = ConflictRegister::ConflictRegisterCount;
	int takenRegisterConflicts = 0;
	if (bitSize <= BitSize::B8) {
		for (unsigned char i = 0; i <= (unsigned char)ConflictRegister::BH; i++) {
			if (conflictRegisters[i])
				takenRegisterConflicts++;
			else if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount)
				firstAvailableRegister = (ConflictRegister)i;
		}
	} else {
		for (unsigned char i = 0; i < (unsigned char)ConflictRegister::SP; i += 2) {
			if (conflictRegisters[i] || conflictRegisters[i + 1])
				takenRegisterConflicts++;
			else if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount)
				firstAvailableRegister = (ConflictRegister)i;
		}
		for (unsigned char i = (unsigned char)ConflictRegister::SP; i <= (unsigned char)ConflictRegister::DI; i++) {
			if (conflictRegisters[i])
				takenRegisterConflicts++;
			else if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount)
				firstAvailableRegister = (ConflictRegister)i;
		}
	}
	return Group2<ConflictRegister, int>(firstAvailableRegister, takenRegisterConflicts);
}
//set the temp or register to a register with the specific register
void BuildInitialAssembly::setStorageToRegister(
	AssemblyStorage* storageToAssign, SpecificRegister specificRegister, FunctionDefinition* errorToken)
{
	TempStorage* tempToAssign;
	Register* registerToAssign;
	if (let(TempStorage*, tempToAssign, storageToAssign))
		tempToAssign->finalStorage = Register::registerFor(specificRegister);
	else if (let(Register*, registerToAssign, storageToAssign))
		registerToAssign->specificRegister = specificRegister;
	else
		Error::logError(ErrorType::CompilerIssue, "assigning a register value to a variable", errorToken);
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
//since smaller-bit-size parameters will be grouped together, get the indices that each parameter will appear in
//returns the stack offset in bytes of each parameter (-cpuBitSize for the parameter at the highest address)
Array<int>* BuildInitialAssembly::getParameterStackOffsets(Array<BitSize>* parameters) {
	Array<int>* parameterStackOffsets = new Array<int>();
	if (parameters->length == 0)
		return parameterStackOffsets;
	//start by finding the order that the parameters will be in memory
	Array<int> parameterStackOrder;
	parameterStackOrder.add(0);
	for (int i = 1; i < parameters->length; i++) {
		unsigned char currentBitSize = (unsigned char)parameters->get(i);
		int insertionIndex = i;
		while (insertionIndex >= 1
				&& (unsigned char)parameters->get(parameterStackOrder.get(insertionIndex - 1))
					< currentBitSize)
			insertionIndex--;
		parameterStackOrder.insert(i, insertionIndex);
	}
	//now go through that order and assign offsets
	for (int i = parameters->length; i > 0; i--)
		parameterStackOffsets->add(0);
	//don't forget to leave space for the return address
	int stackOffset = -(int)((unsigned char)cpuBitSize / 8);
	forEach(int, parameterIndex, &parameterStackOrder, pi) {
		parameterStackOffsets->set(parameterIndex, stackOffset);
		stackOffset -= (int)((unsigned char)parameters->get(parameterIndex)) / 8;
	}
	return parameterStackOffsets;
}
//add the entry to the associated list of the key
Array<AssemblyStorage*>* BuildInitialAssembly::storagesListFor(
	AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* storages, AssemblyStorage* key)
{
	Array<AssemblyStorage*>* associatedStorages = storages->get(key);
	if (associatedStorages == nullptr) {
		associatedStorages = new Array<AssemblyStorage*>();
		storages->set(key, associatedStorages);
	}
	return associatedStorages;
}
