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

//TODO: function definitions
//-build assembly
//-stack modifications before + after function calls will only make space for parameters
//-nested function calls will not track stack offets from outer function calls
//-figure out how much stack space we need
//-**modify** stack modifications to be the right value (turn just-parameters constants into the whole stack)
//-arguments (MOV memory pointer destinations) don't get modified since they are already correctly relative to the stack pointer

//TODO: if a static function is only called once, insert its instructions into where it's called

//TODO: check for local uninitialized variables
//TODO: check for uninitialized variables within function definitions

//TODO: stack shifts need to handle temps

//TODO: make sure all parameter memory pointers have the appropriate offset

//TODO: stack shifts only care about cpuSPRegister, not any other memory pointer

//TODO: track which registers hold which temps, and use the register in place of the temp if possible

//TODO: use different memory pointers for ParameterStorages for this function vs the called function


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
thread_local FunctionStaticStorage* BuildInitialAssembly::currentFunction = nullptr;
thread_local BuildResult* BuildInitialAssembly::buildResult = nullptr;
thread_local AVLTree<FunctionDefinition*, FunctionStaticStorage*>* BuildInitialAssembly::functionDefinitionToStorageMap =
	nullptr;
thread_local AVLTree<CVariableDefinition*, AssemblyStorage*>* BuildInitialAssembly::variableToStorageMap = nullptr;
thread_local BitSize BuildInitialAssembly::cpuBitSize = BitSize::BInfinite;
thread_local int BuildInitialAssembly::cpuByteSize = 0;
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
//build the final executable file for the specified bit size specified
BuildResult* BuildInitialAssembly::buildInitialAssembly(Pliers* pliers, BitSize pCPUBitSize) {
	cpuBitSize = pCPUBitSize;
	cpuByteSize = (int)((unsigned char)cpuBitSize / 8);
	if (pliers->printProgress)
		puts("Building executable...");

	setupAssemblyObjects();
	if (cpuBitSize != BitSize::B32) {
		EmptyToken errorToken (0, pliers->allFiles->get(0));
		Error::logError(ErrorType::General, "only 32 bit compilation is supported", &errorToken);
		return buildResult;
	}

	//get the initial assembly for all tokens
	Identifier globalInitName ("", 0, 0, pliers->allFiles->get(0));
	currentFunction =
		(buildResult->globalInit =
			createAndStoreFunctionStaticStorage(
				new FunctionDefinition(
					CDataType::voidType, new Array<CVariableDefinition*>(), new Array<Statement*>(), &globalInitName)));
	addGlobalVariableInitializations(pliers);
	currentFunction->instructions->add(new CALL(currentFunction, Register::aRegisterForBitSize(cpuBitSize)));
	currentFunction->instructions->add(new RET(currentFunction));

	//now assign temps
	//start with the functions that have no other function dependencies, then slowly go up, that way we can
	//	possibly preserve register values across function calls
	//if we get to a point where dependencies are cyclic, just pick one and assume dependencies use all registers
	Array<FunctionStaticStorage*>* functionsToAssignTempsTo = new Array<FunctionStaticStorage*>();
	forEach(FunctionStaticStorage*, fss, buildResult->functionDefinitions, fssi) {
		//Main.* functions already have registers
		if (fss->registersUsed == nullptr)
			functionsToAssignTempsTo->add(fss);
	}
	while (functionsToAssignTempsTo->length > 0) {
		bool foundFunctionToAssignTempsTo = false;
		for (int i = functionsToAssignTempsTo->length - 1; i >= 0; i--) {
			FunctionStaticStorage* f = functionsToAssignTempsTo->get(i);
			Array<FunctionStaticStorage*>* dependencies = f->tempAssignmentDependencies;
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

	//finally go through and shift any memory pointers to match stack shifts
	shiftStackPointers(currentFunction);
	forEach(FunctionStaticStorage*, fss, buildResult->functionDefinitions, stackShiftsI) {
		shiftStackPointers(fss);
	}

	//TODO: get the actual assembly bytes
	//TODO: optimize the assembly
	//	-remove unnecessary double jumps from boolean conditions
	//	-assign temps and remove MOVs where possible (same register or same memory address)
	//TODO: write the file

	cleanupAssemblyObjects();
	return buildResult;
}
//initialize everything used for building, all of which will be tracked by the build result
void BuildInitialAssembly::setupAssemblyObjects() {
	buildResult = new BuildResult();
	functionDefinitionToStorageMap = new AVLTree<FunctionDefinition*, FunctionStaticStorage*>();
	variableToStorageMap = new AVLTree<CVariableDefinition*, AssemblyStorage*>();
	cpuARegister = Register::aRegisterForBitSize(cpuBitSize);
	cpuCRegister = Register::cRegisterForBitSize(cpuBitSize);
	cpuDRegister = Register::dRegisterForBitSize(cpuBitSize);
	cpuBRegister = Register::bRegisterForBitSize(cpuBitSize);
	cpuSPRegister = Register::spRegisterForBitSize(cpuBitSize);
	cpuSIRegister = Register::siRegisterForBitSize(cpuBitSize);
	cpuDIRegister = Register::diRegisterForBitSize(cpuBitSize);
	buildResult->globalValues->add(generalPurposeVariable1 = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(generalPurposeVariable2 = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(generalPurposeVariable3 = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(generalPurposeVariable4 = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(processHeapPointer = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(copperHeapPointer = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(copperHeapNextFreeAddressPointer = new ValueStaticStorage(cpuBitSize));
	buildResult->globalValues->add(copperHeapSizePointer = new ValueStaticStorage(cpuBitSize));
	if (cpuBitSize == BitSize::B32)
		build32BitMainFunctions();
}
//delete anything used for building that we don't need any more
void BuildInitialAssembly::cleanupAssemblyObjects() {
	delete functionDefinitionToStorageMap;
	delete variableToStorageMap;
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
	Register* blRegister = Register::registerFor(SpecificRegister::BL);
	Register* dlRegister = Register::registerFor(SpecificRegister::DL);

	CVariableDefinition* iParameter_Main_exit = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	buildResult->function_Main_exit =
		createAndStoreFunctionStaticStorage(
			new FunctionDefinition(
				CDataType::voidType,
				Array<CVariableDefinition*>::newArrayWith(iParameter_Main_exit),
				new Array<Statement*>(),
				&functionDefinitionSource));
	ParameterStorage* iParameterStorage_Main_exit;
	if (!let(ParameterStorage*, iParameterStorage_Main_exit, variableToStorageMap->get(iParameter_Main_exit))) {
		Error::logError(
			ErrorType::CompilerIssue,
			"retrieving the parameter assembly storage for Main.exit",
			buildResult->globalInit->sourceFunction);
		return;
	}
	iParameterStorage_Main_exit->finalStorage = cpuARegister;
	Array<AssemblyInstruction*>* assembly_Main_exit = buildResult->function_Main_exit->instructions;
	assembly_Main_exit->add(new SUB(cpuSPRegister, constant32Bit4));
	assembly_Main_exit->add(new MOV(argumentStorage1, cpuARegister));
	assembly_Main_exit->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TExitProcess, BitSize::B32)), nullptr));
	assembly_Main_exit->add(new RET(buildResult->function_Main_exit));
	//since this exits the program, we don't need to worry about what registers were used
	buildResult->function_Main_exit->registersUsed = new Array<SpecificRegister>();

	CVariableDefinition* sParameter_Main_print =
		new CVariableDefinition(CDataType::stringType, new Identifier("", 0, 0, nullptr));
	buildResult->function_Main_print =
		createAndStoreFunctionStaticStorage(
			new FunctionDefinition(
				CDataType::voidType,
				Array<CVariableDefinition*>::newArrayWith(sParameter_Main_print),
				new Array<Statement*>(),
				&functionDefinitionSource));
	ParameterStorage* sParameterStorage_Main_print;
	if (!let(ParameterStorage*, sParameterStorage_Main_print, variableToStorageMap->get(sParameter_Main_print))) {
		Error::logError(
			ErrorType::CompilerIssue,
			"retrieving the parameter assembly storage for Main.print",
			buildResult->globalInit->sourceFunction);
		return;
	}
	sParameterStorage_Main_print->finalStorage = new MemoryPointer(cpuSPRegister, 4, BitSize::B32);
	Array<AssemblyInstruction*>* assembly_Main_print = buildResult->function_Main_print->instructions;
	//make space for WriteFile parameters
	assembly_Main_print->add(new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(20, BitSize::B32))));
	//get the StdOutputHandle
	assembly_Main_print->add(new SUB(cpuSPRegister, constant32Bit4));
	assembly_Main_print->add(
		new MOV(argumentStorage1, globalTrackedStorage(new AssemblyConstant(STD_OUTPUT_HANDLE, BitSize::B32))));
	assembly_Main_print->add(
		new CALL(globalTrackedStorage(new ThunkStaticStorage(&TGetStdHandle, BitSize::B32)), cpuARegister));
	//load all the parameters for WriteFile
	//argument 1: STD_OUTPUT_HANDLE in cpuARegister
	assembly_Main_print->add(new MOV(argumentStorage1, cpuARegister));
	//put the (first parameter) string address in cpuARegister
	assembly_Main_print->add(
		new MOV(cpuARegister, globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, 24, true, BitSize::B32))));
	//put the char array address in cpuARegister
	assembly_Main_print->add(new MOV(cpuARegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32))));
	//put the address of the first char in cpuCRegister
	assembly_Main_print->add(new LEA(cpuCRegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 8, BitSize::B32))));
	//argument 2: address of string to print
	assembly_Main_print->add(new MOV(argumentStorage2, cpuCRegister));
	//put the length of the string in cpuCRegister
	assembly_Main_print->add(new MOV(cpuCRegister, globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32))));
	//argument 3: length of string
	assembly_Main_print->add(new MOV(argumentStorage3, cpuCRegister));
	//argument 4: int address to write number of bytes written
	assembly_Main_print->add(
		new MOV(argumentStorage4, globalTrackedStorage(new StaticAddress(generalPurposeVariable1, BitSize::B32))));
	//argument 5: null OVERLAPPED pointer
	assembly_Main_print->add(new MOV(argumentStorage5, globalTrackedStorage(new AssemblyConstant(0, BitSize::B32))));
	assembly_Main_print->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TWriteFile, BitSize::B32)), cpuARegister));
	assembly_Main_print->add(new RET(buildResult->function_Main_print));
	buildResult->function_Main_print->registersUsed = new Array<SpecificRegister>();
	//since we call an outside function, assume all registers were used
	for (unsigned char i = (unsigned char)SpecificRegister::Register32BitStart;
			i < (unsigned char)SpecificRegister::Register32BitEnd;
			i++)
		buildResult->function_Main_print->registersUsed->add((SpecificRegister)i);

	CVariableDefinition* iParameter_Main_str = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	buildResult->function_Main_str =
		createAndStoreFunctionStaticStorage(
			new FunctionDefinition(
				CDataType::stringType,
				Array<CVariableDefinition*>::newArrayWith(iParameter_Main_str),
				new Array<Statement*>(),
				&functionDefinitionSource));
	ParameterStorage* iParameterStorage_Main_str;
	if (!let(ParameterStorage*, iParameterStorage_Main_str, variableToStorageMap->get(iParameter_Main_str))) {
		Error::logError(
			ErrorType::CompilerIssue,
			"retrieving the parameter assembly storage for Main.str",
			buildResult->globalInit->sourceFunction);
		return;
	}
	iParameterStorage_Main_str->finalStorage = cpuARegister;
	Array<AssemblyInstruction*>* assembly_Main_str = buildResult->function_Main_str->instructions;
	AssemblyLabel* Main_strWriteDigitLabel = new AssemblyLabel();
	AssemblyLabel* Main_addMinusSignLabel = new AssemblyLabel();
	AssemblyLabel* Main_allocateStringLabel = new AssemblyLabel();
	//setup initial values- dividend (already set in EAX), buffer pointer (ESI), divisor (ECX), and negative flag (BL)
	assembly_Main_str->add(new MOV(cpuSIRegister, cpuSPRegister));
	assembly_Main_str->add(new CDQ());
	assembly_Main_str->add(new MOV(cpuCRegister, globalTrackedStorage(new AssemblyConstant(10, BitSize::B32))));
	assembly_Main_str->add(new MOV(blRegister, dlRegister));
	//divide once and jump if we have a regular non-negative number
	assembly_Main_str->add(new IDIV(cpuCRegister));
	assembly_Main_str->add(new CMP(blRegister, constant8Bit0));
	assembly_Main_str->add(new JGE(Main_strWriteDigitLabel));
	//negative number- negate EAX and EDX then continue normally
	assembly_Main_str->add(new NEG(cpuARegister));
	assembly_Main_str->add(new NEG(cpuDRegister));
	//write our digit
	assembly_Main_str->add(Main_strWriteDigitLabel);
	assembly_Main_str->add(new DEC(cpuSIRegister));
	assembly_Main_str->add(new ADD(dlRegister, globalTrackedStorage(new AssemblyConstant('0', BitSize::B8))));
	assembly_Main_str->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuSIRegister, 0, BitSize::B8)), dlRegister));
	//if we reached 0, stop and go add a minus sign if it was negative
	assembly_Main_str->add(new CMP(cpuARegister, globalTrackedStorage(new AssemblyConstant(0, BitSize::B32))));
	assembly_Main_str->add(new JLE(Main_addMinusSignLabel));
	//divide again and write the digit
	assembly_Main_str->add(new CDQ());
	assembly_Main_str->add(new IDIV(cpuCRegister));
	assembly_Main_str->add(new JMP(Main_strWriteDigitLabel));
	assembly_Main_str->add(Main_addMinusSignLabel);
	//if we don't need to add the minus sign, go allocate the string
	assembly_Main_str->add(new CMP(blRegister, constant8Bit0));
	assembly_Main_str->add(new JGE(Main_allocateStringLabel));
	//write a minus sign
	assembly_Main_str->add(new DEC(cpuSIRegister));
	assembly_Main_str->add(
		new MOV(
			globalTrackedStorage(new MemoryPointer(cpuSIRegister, 0, BitSize::B8)),
			globalTrackedStorage(new AssemblyConstant('-', BitSize::B8))));
	//allocate space for the string
	assembly_Main_str->add(Main_allocateStringLabel);
	assembly_Main_str->add(new MOV(cpuARegister, copperHeapNextFreeAddressPointer));
	assembly_Main_str->add(new ADD(copperHeapNextFreeAddressPointer, constant32Bit12));
	//get the length of the string
	assembly_Main_str->add(new MOV(cpuCRegister, cpuSPRegister));
	assembly_Main_str->add(new SUB(cpuCRegister, cpuSIRegister));
	//allocate space for the char array
	assembly_Main_str->add(new MOV(cpuDRegister, copperHeapNextFreeAddressPointer));
	assembly_Main_str->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuDRegister, 4, BitSize::B32)), cpuCRegister));
	assembly_Main_str->add(new LEA(cpuBRegister, globalTrackedStorage(new MemoryPointer(cpuCRegister, 12, BitSize::B32))));
	assembly_Main_str->add(new AND(cpuBRegister, globalTrackedStorage(new AssemblyConstant(0xFFFFFFFC, BitSize::B32))));
	assembly_Main_str->add(new ADD(copperHeapNextFreeAddressPointer, cpuBRegister));
	//write to the char array
	assembly_Main_str->add(new CLD());
	assembly_Main_str->add(new LEA(cpuDIRegister, globalTrackedStorage(new MemoryPointer(cpuDRegister, 8, BitSize::B32))));
	assembly_Main_str->add(new REPMOVSB(BitSize::B32));
	//save the char array in the string
	assembly_Main_str->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32)), cpuDRegister));
	assembly_Main_str->add(new RET(buildResult->function_Main_str));
	buildResult->function_Main_str->registersUsed = new Array<SpecificRegister>();
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::EAX);
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::ECX);
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::EDX);
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::EBX);
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::ESI);
	buildResult->function_Main_str->registersUsed->add(SpecificRegister::EDI);
}
//go through all the global variables and add their assembly in the order that they will be initialized in
void BuildInitialAssembly::addGlobalVariableInitializations(Pliers* pliers) {
	//go through all the variable initializations, add assembly to initialize them if possible and save them for later if not
	PrefixTrie<char, CVariableData*> globalVariableData;
	Array<Operator*> initializationsNotReady;
	forEach(SourceFile*, s, pliers->allFiles, si) {
		forEach(Token*, t, s->globalVariables, ti) {
			VariableDeclarationList* v;
			Operator* o = nullptr;
			if (let(VariableDeclarationList*, v, t) || (let(Operator*, o, t) && let(VariableDeclarationList*, v, o->left))) {
				forEach(CVariableDefinition*, c, v->variables, ci) {
					ValueStaticStorage* globalVariableStorage = new ValueStaticStorage(typeBitSize(c->type));
					variableToStorageMap->set(c, globalVariableStorage);
					buildResult->globalValues->add(globalVariableStorage);
				}
				if (o != nullptr) {
					FindUninitializedVariablesVisitor visitor (&globalVariableData, false);
					visitor.handleExpression(o);
					if (visitor.allVariablesAreInitialized)
						addTokenAssembly(o, nullptr, nullptr);
					else
						initializationsNotReady.add(o);
				}
			} else
				Error::logError(ErrorType::CompilerIssue, "a non-variable-initialization global token", t);
		}
	}

	//then keep trying to add assembly for the remaining initializations
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
				addTokenAssembly(o, nullptr, nullptr);
				foundInitializedVariable = true;
			}
		}
	}

	//error if there are any uninitialized global variables
	forEach(Operator*, o, &initializationsNotReady, initializationsNotReadyI) {
		FindUninitializedVariablesVisitor(&globalVariableData, true).handleExpression(o);
	}
	globalVariableData.deleteValues();
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
			return addOperatorAssembly(o, expectedType, jumpDests);
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
		buildResult->stringDefinitions->add(sss);
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
		//an initial value function definition is not eligible for register parameters
		//	if we're not calling this variable or if it was written to
		FunctionStaticStorage* storage = getFunctionDefinitionStorage(f, isBeingFunctionCalled && !i->variable->writtenTo);
		//add this function as a temp assignment dependency if we haven't already
		currentFunction->tempAssignmentDependencies->addNonDuplicate(storage);
		//if we never write to a function, we can directly get the function definition's storage
		if (!i->variable->writtenTo)
			return storage;
	}
	return getVariableStorage(i->variable);
}
//get the assembly for the inner token and add any assembly needed to cast it to the expected type
AssemblyStorage* BuildInitialAssembly::addCastAssembly(Cast* c) {
	AssemblyStorage* result = addTokenAssembly(c->right, c->dataType, nullptr);
	BitSize targetBitSize = typeBitSize(c->dataType);
	BitSize valueBitSize = result->bitSize;
	if ((unsigned char)valueBitSize < (unsigned char)targetBitSize) {
		Register* newResult = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(targetBitSize));
		currentFunction->instructions->add(new MOVSX(newResult, result));
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
		return buildResult->function_Main_exit;
	else if (i->name == "print")
		return buildResult->function_Main_print;
	else if (i->name == "str")
		return buildResult->function_Main_str;
	else {
		Error::logError(ErrorType::CompilerIssue, "finding the function for this variable", i);
		return cpuARegister;
	}
}
//get the assembly for the inner tokens and the assembly to perform this operator on them
//returns nullptr if jump destinations were passed and they were used instead
Register* BuildInitialAssembly::addOperatorAssembly(Operator* o, CDataType* expectedType, ConditionLabelPair* jumpDests) {
	//operators were rewritten to be postfix so the left token will always be present
	CDataType* leftExpectedType = o->left->dataType;
	CDataType* rightExpectedType = o->right != nullptr ? o->right->dataType : nullptr;
	//implicit casts were also already done, based on best compatible types
	//if the operand types still aren't final, give them the expected type or no type
	if (typeBitSize(leftExpectedType) == BitSize::BInfinite) {
		if (o->precedence == OperatorTypePrecedence::Comparison) {
			leftExpectedType = nullptr;
			rightExpectedType = nullptr;
		} else {
			leftExpectedType = expectedType;
			rightExpectedType = o->semanticsType != OperatorSemanticsType::IntegerIntegerBitShift ? expectedType : nullptr;
		}
	}
	AssemblyStorage* leftStorage = nullptr;
	AssemblyStorage* rightStorage = nullptr;
	Register* resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(typeBitSize(leftExpectedType)));
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
				currentFunction->instructions->add(new MOV(resultStorage, leftStorage));
				addBooleanJump(resultStorage, booleanJumpDests);
			//we didn't get a value, set the result and jump if we don't need to check the rest of the boolean
			} else {
				currentFunction->instructions->add(resultFinishedLabel);
				resultFinishedLabel = new AssemblyLabel();
				currentFunction->instructions->add(
					new MOV(
						resultStorage,
						globalTrackedStorage(new AssemblyConstant(andOperator ? 0 : 1, resultStorage->bitSize))));
				currentFunction->instructions->add(new JMP(resultFinishedLabel));
			}
		}
		delete booleanJumpDests;
		//add the right side normally
		currentFunction->instructions->add(continueBooleanLabel);
		addConditionAssembly(o->right, resultStorage, rightExpectedType, jumpDests);
		if (jumpDests != nullptr)
			return nullptr;
		else {
			currentFunction->instructions->add(resultFinishedLabel);
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
		currentFunction->instructions->add(ternaryTrueLabel);
		addConditionAssembly(ternaryResult->left, resultStorage, rightExpectedType, jumpDests);
		if (jumpDests == nullptr)
			currentFunction->instructions->add(new JMP(ternaryAfterLabel));
		currentFunction->instructions->add(ternaryFalseLabel);
		addConditionAssembly(ternaryResult->right, resultStorage, rightExpectedType, jumpDests);
		currentFunction->instructions->add(ternaryAfterLabel);
		return jumpDests != nullptr ? nullptr : resultStorage;
	} else if (o->operatorType == OperatorType::LogicalNot && jumpDests != nullptr) {
		ConditionLabelPair reversedJumpDests (jumpDests->falseJumpDest, jumpDests->trueJumpDest);
		addConditionAssembly(o->left, nullptr, leftExpectedType, &reversedJumpDests);
		return nullptr;
	}
	bool leftStorageIsAddress = false;
	VariableDeclarationList* v = nullptr;
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
				currentFunction->instructions->add(new LEA(transferStorage, leftMemoryPointer));
				currentFunction->instructions->add(new MOV(leftStorage, transferStorage));
				leftStorageIsAddress = true;
			//anything else (necessarily a temp) can stay how it is
			} else
				leftStorage = originalLeftStorage;
		//it doesn't modify a variable so it's just a value
		//if it's a register or memory pointer, put it in a temp to ensure it could be a stack value if it needs to be
		} else {
			if (istype(originalLeftStorage, Register*)) {
				leftStorage = globalTrackedStorage(new TempStorage(originalLeftStorage->bitSize));
				currentFunction->instructions->add(new MOV(leftStorage, originalLeftStorage));
			} else if (istype(originalLeftStorage, MemoryPointer*)) {
				leftStorage = globalTrackedStorage(new TempStorage(originalLeftStorage->bitSize));
				addMemoryToMemoryMove(leftStorage, originalLeftStorage);
			//if it's another temp or some constant storage, we don't need to do anything with it
			} else
				leftStorage = originalLeftStorage;
		}
	}
	FunctionDefinition* f;
	rightStorage = o->right == nullptr
		? nullptr
		//if we're initializing functions, make sure they could be eligible for register parameters
		: v != nullptr && let(FunctionDefinition*, f, o->right)
			? getFunctionDefinitionStorage(f, true)
			: addTokenAssembly(o->right, rightExpectedType, nullptr);
	//if the left storage is a value, load it into the result
	if (o->operatorType != OperatorType::Assign) {
		//if we stored an address, we need to that and then get the value
		if (leftStorageIsAddress) {
			Register* transferStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
			currentFunction->instructions->add(new MOV(transferStorage, leftStorage));
			currentFunction->instructions->add(
				new MOV(resultStorage, globalTrackedStorage(new MemoryPointer(transferStorage, 0, resultStorage->bitSize))));
		} else
			currentFunction->instructions->add(new MOV(resultStorage, leftStorage));
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
		case OperatorType::VariableLogicalNot:
		case OperatorType::LogicalNot:
			currentFunction->instructions->add(
				new CMP(resultStorage, globalTrackedStorage(new AssemblyConstant(0, resultStorage->bitSize))));
			currentFunction->instructions->add(new SETE(resultStorage));
			break;
		case OperatorType::Increment: currentFunction->instructions->add(new INC(resultStorage)); break;
		case OperatorType::Decrement: currentFunction->instructions->add(new DEC(resultStorage)); break;
		case OperatorType::VariableBitwiseNot:
		case OperatorType::BitwiseNot:
			currentFunction->instructions->add(new NOT(resultStorage));
			break;
		case OperatorType::VariableNegate:
		case OperatorType::Negate:
			currentFunction->instructions->add(new NEG(resultStorage));
			break;
		case OperatorType::Multiply:
		case OperatorType::AssignMultiply: {
			//use as many operands as possible (as recommended by performance testing)
			AssemblyConstant* rightConstant;
			if (let(AssemblyConstant*, rightConstant, rightStorage))
				currentFunction->instructions->add(new IMUL(resultStorage, resultStorage, rightConstant));
			else
				currentFunction->instructions->add(new IMUL(resultStorage, rightStorage, nullptr));
			break;
		}
		case OperatorType::Divide:
		case OperatorType::Modulus:
		case OperatorType::AssignDivide:
		case OperatorType::AssignModulus:
			//we needed to use the *A* register to hold the dividend
			resultStorage->specificRegister = Register::aRegisterForBitSize(resultStorage->bitSize)->specificRegister;
			if (resultStorage->bitSize == BitSize::B32)
				currentFunction->instructions->add(new CDQ());
			else if (resultStorage->bitSize == BitSize::B16)
				currentFunction->instructions->add(new CWD());
			else
				currentFunction->instructions->add(new CBW());
			currentFunction->instructions->add(new IDIV(resultStorage));
			if (o->operatorType == OperatorType::Modulus || o->operatorType == OperatorType::AssignModulus)
				resultStorage = Register::dRegisterForBitSize(resultStorage->bitSize);
			break;
		case OperatorType::Add:
		case OperatorType::AssignAdd:
//TODO: handle string concatenations
			currentFunction->instructions->add(new ADD(resultStorage, rightStorage));
			break;
		case OperatorType::Subtract:
		case OperatorType::AssignSubtract:
			currentFunction->instructions->add(new SUB(resultStorage, rightStorage));
			break;
		case OperatorType::ShiftLeft:
		case OperatorType::AssignShiftLeft:
			if (istype(rightStorage, AssemblyConstant*))
				currentFunction->instructions->add(new SHL(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentFunction->instructions->add(new MOV(clRegister, rightStorage));
				currentFunction->instructions->add(new SHL(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftRight:
		case OperatorType::AssignShiftRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentFunction->instructions->add(new SHR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentFunction->instructions->add(new MOV(clRegister, rightStorage));
				currentFunction->instructions->add(new SHR(resultStorage, clRegister));
			}
			break;
		case OperatorType::ShiftArithmeticRight:
		case OperatorType::AssignShiftArithmeticRight:
			if (istype(rightStorage, AssemblyConstant*))
				currentFunction->instructions->add(new SAR(resultStorage, rightStorage));
			else {
				Register* clRegister = Register::cRegisterForBitSize(BitSize::B8);
				currentFunction->instructions->add(new MOV(clRegister, rightStorage));
				currentFunction->instructions->add(new SAR(resultStorage, clRegister));
			}
			break;
//		case OperatorType::RotateLeft:
//			break;
//		case OperatorType::RotateRight:
//			break;
		case OperatorType::BitwiseAnd:
		case OperatorType::AssignBitwiseAnd:
			currentFunction->instructions->add(new AND(resultStorage, rightStorage));
			break;
		case OperatorType::BitwiseXor:
		case OperatorType::AssignBitwiseXor:
			currentFunction->instructions->add(new XOR(resultStorage, rightStorage));
			break;
		case OperatorType::BitwiseOr:
		case OperatorType::AssignBitwiseOr:
			currentFunction->instructions->add(new OR(resultStorage, rightStorage));
			break;
		case OperatorType::Equal:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JE(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETE(resultStorage));
			}
			break;
		case OperatorType::NotEqual:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JNE(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETNE(resultStorage));
			}
			break;
		case OperatorType::LessOrEqual:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JLE(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETLE(resultStorage));
			}
			break;
		case OperatorType::GreaterOrEqual:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JGE(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETGE(resultStorage));
			}
			break;
		case OperatorType::LessThan:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JL(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETL(resultStorage));
			}
			break;
		case OperatorType::GreaterThan:
//TODO: handle floats
			currentFunction->instructions->add(new CMP(resultStorage, rightStorage));
			if (jumpDests != nullptr) {
				currentFunction->instructions->add(new JG(jumpDests->trueJumpDest));
				currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
				return nullptr;
			} else {
				resultStorage = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(BitSize::B8));
				currentFunction->instructions->add(new SETG(resultStorage));
			}
			break;
		case OperatorType::Assign: currentFunction->instructions->add(new MOV(resultStorage, rightStorage)); break;
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
				currentFunction->instructions->add(new MOV(getVariableStorage(vd), resultStorage));
			}
		//any other type of variable modifier
		} else {
			//if it was an address, store it there
			if (leftStorageIsAddress) {
				Register* transferRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(cpuBitSize));
				currentFunction->instructions->add(new MOV(transferRegister, leftStorage));
				currentFunction->instructions->add(
					new MOV(
						globalTrackedStorage(new MemoryPointer(transferRegister, 0, resultStorage->bitSize)),
						resultStorage));
			} else
				currentFunction->instructions->add(new MOV(leftStorage, resultStorage));
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
			fd = functionStaticStorage->sourceFunction;
		//if we don't and it's not a temp either, store it in a temp while we compute our arguments
		else if (!istype(functionStorage, TempStorage*)) {
			TempStorage* functionTempStorage = globalTrackedStorage(new TempStorage(cpuBitSize));
			addMemoryToMemoryMove(functionTempStorage, functionStorage);
			functionStorage = functionTempStorage;
		}
	}
	//then go through and add all the arguments
	//if we have a function definition, we can use its temps to save arguments, and return its result storage
	//sometime later we will adjust or remove the SUB for the stack if we were able to use register parameters
	if (functionStaticStorage != nullptr) {
		currentFunction->instructions->add(new StackShift(currentFunction, functionStaticStorage, 0, true));
		for (int i = 0; i < f->arguments->length; i++) {
			addMemoryToMemoryMove(
				functionStaticStorage->parameterStorages->get(i),
				addTokenAssembly(f->arguments->get(i), fd->parameters->get(i)->type, nullptr));
		}
		currentFunction->instructions->add(new CALL(functionStaticStorage, functionStaticStorage->resultStorage));
		currentFunction->instructions->add(new StackShift(currentFunction, functionStaticStorage, 0, false));
		return functionStaticStorage->resultStorage;
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
		int shiftTotalBytes = Math::roundUpToMultipleOf(shiftTotalBits / 8, cpuByteSize);
		currentFunction->instructions->add(new StackShift(currentFunction, nullptr, shiftTotalBytes, true));
		Array<int>* argumentStackOffsets = getBitSizeOrderedStackOffsets(&argumentBitSizes);
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
		currentFunction->instructions->add(new MOV(functionStorageRegister, functionStorage));
		currentFunction->instructions->add(
			new CALL(
				//the function address is right after the type pointer in memory
				globalTrackedStorage(new MemoryPointer(functionStorageRegister, cpuByteSize, cpuBitSize)),
				resultRegister));
		currentFunction->instructions->add(new StackShift(currentFunction, nullptr, shiftTotalBytes, false));
		return resultRegister;
	}
}
//get the storage of the function definition
FunctionStaticStorage* BuildInitialAssembly::getFunctionDefinitionStorage(
	FunctionDefinition* f, bool couldBeEligibleForRegisterParameters)
{
	FunctionStaticStorage* storage = functionDefinitionToStorageMap->get(f);
	if (storage == nullptr) {
		FunctionStaticStorage* oldFunction = currentFunction;
		currentFunction = (storage = createAndStoreFunctionStaticStorage(f));
		addStatementListAssembly(f->body);
		if (f->dataType == CDataType::voidType)
			storage->instructions->add(new RET(storage));
		currentFunction = oldFunction;
	}
	//if it's part of an expression (like a ternary), it's not eligible for register parameters
	if (!couldBeEligibleForRegisterParameters) {
		storage->eligibleForRegisterParameters = false;
		if (storage->resultStorage != nullptr)
			storage->resultStorage->specificRegister =
				Register::aRegisterForBitSize(storage->resultStorage->bitSize)->specificRegister;
	}
	return storage;
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
	return globalTrackedStorage(new AssemblyConstant(val, bitSize));
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
				AssemblyStorage* resultStorage =
					addTokenAssembly(r->expression, currentFunction->sourceFunction->returnType, nullptr);
				currentFunction->instructions->add(new MOV(currentFunction->resultStorage, resultStorage));
			}
			currentFunction->instructions->add(new RET(currentFunction));
		} else if (let(IfStatement*, i, s)) {
			AssemblyLabel* thenLabel = new AssemblyLabel();
			AssemblyLabel* elseLabel = new AssemblyLabel();
			ConditionLabelPair ifJumpDests (thenLabel, elseLabel);
			addConditionAssembly(i->condition, nullptr, nullptr, &ifJumpDests);
			currentFunction->instructions->add(thenLabel);
			addStatementListAssembly(i->thenBody);
			if (i->elseBody != nullptr) {
				AssemblyLabel* ifAfterLabel = new AssemblyLabel();
				currentFunction->instructions->add(new JMP(ifAfterLabel));
				currentFunction->instructions->add(elseLabel);
				addStatementListAssembly(i->elseBody);
				currentFunction->instructions->add(ifAfterLabel);
			} else
				currentFunction->instructions->add(elseLabel);
		} else if (let(LoopStatement*, l, s)) {
			if (l->initialization != nullptr)
				addTokenAssembly(l->initialization, nullptr, nullptr);
			AssemblyLabel* loopStartLabel = new AssemblyLabel();
			AssemblyLabel* loopContinueLabel = new AssemblyLabel();
			AssemblyLabel* loopBreakLabel = new AssemblyLabel();
			ConditionLabelPair loopJumpDests (loopStartLabel, loopBreakLabel);
			if (l->initialConditionCheck)
				addConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentFunction->instructions->add(loopStartLabel);
			//TODO: pass this loop for breaks and continues
			addStatementListAssembly(l->body);
			currentFunction->instructions->add(loopContinueLabel);
			if (l->increment != nullptr)
				addTokenAssembly(l->increment, nullptr, nullptr);
			addConditionAssembly(l->condition, nullptr, nullptr, &loopJumpDests);
			currentFunction->instructions->add(loopBreakLabel);
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
		currentFunction->instructions->add(new MOV(transferRegister, source));
	}
	currentFunction->instructions->add(new MOV(destination, transferRegister));
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
		currentFunction->instructions->add(new MOV(resultStorage, valueStorage));
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
		currentFunction->instructions->add(new JMP(a->val != 0 ? jumpDests->trueJumpDest : jumpDests->falseJumpDest));
	else {
		currentFunction->instructions->add(new CMP(source, zeroConstant));
		currentFunction->instructions->add(new JNE(jumpDests->trueJumpDest));
		currentFunction->instructions->add(new JMP(jumpDests->falseJumpDest));
	}
}
//assign registers or memory pointers to all temps of the provided instructions
void BuildInitialAssembly::assignTemps(FunctionStaticStorage* source) {
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
	Array<SpecificRegister>* registersUsed = new Array<SpecificRegister>();
	markConflicts(&storagesUsed, &conflictsByStorage, &calls, registersUsed, source);

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
		//an unused, uninitialized variable has no conflicts
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
		forEach(ParameterStorage*, p, source->parameterStorages, ci) {
			argumentBitSizes.add(p->bitSize);
		}
		Array<int>* parameterStackOffsets = getBitSizeOrderedStackOffsets(&argumentBitSizes);
		for (int i = 0; i < parameterStackOffsets->length; i++)
			source->parameterStorages->get(i)->finalStorage =
				new MemoryPointer(cpuSPRegister, parameterStackOffsets->get(i) + cpuByteSize, argumentBitSizes.get(i));
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
					Error::logError(
						ErrorType::CompilerIssue, "finding assembly stack storage for variables", source->sourceFunction);
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
				Register::specificRegisterFor(nextFirstAvailableRegister, nextStorageToAssign->bitSize),
				source);
			storagesToAssign.remove(nextStorageToAssignIndex);
		}
	}

	//we finished assigning all the storages we weren't sure about
	//give registers to all the storages that can definitely have them
	forEach(AssemblyStorage*, storageToAssign, &storagesToAssignWithGuaranteedRegister, guaranteedRegisterI) {
		setConflictRegisters(conflictRegisters, conflictsByStorage.get(storageToAssign));
		ConflictRegister firstAvailableRegister = getFirstAvailableRegister(storageToAssign->bitSize, conflictRegisters).val1;
		if (firstAvailableRegister == ConflictRegister::ConflictRegisterCount) {
			Error::logError(ErrorType::CompilerIssue, "finding final register storage for variables", source->sourceFunction);
			return;
		}
		setStorageToRegister(
			storageToAssign,
			Register::specificRegisterFor(firstAvailableRegister, storageToAssign->bitSize),
			source);
	}
//TODO: give stack memory pointers to parameter temps separately
	//give stack memory pointers to all the temps that need to be on the stack
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

	//figure out how big our stack parameters are
	int parameterBytesUsed = 0;
	forEach(ParameterStorage*, parameter, source->parameterStorages, parameterBytesUsedI) {
		MemoryPointer* stackParameter;
		if (let(MemoryPointer*, stackParameter, parameter->finalStorage))
			parameterBytesUsed += (int)(((unsigned char)stackParameter->bitSize + 7) / 8);
	}
	source->parameterBytesUsed = Math::roundUpToMultipleOf(parameterBytesUsed, cpuByteSize);

	//and go through all the storages and mark registers as used
	forEach(AssemblyStorage*, storage, conflictsByStorage.insertionOrder, usedRegistersI) {
		Register* registerStorage;
		TempStorage* tempStorage;
		if (let(Register*, registerStorage, storage)
				|| (let(TempStorage*, tempStorage, storage) && let(Register*, registerStorage, tempStorage->finalStorage)))
			registersUsed->addNonDuplicate(registerStorage->specificRegister);
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
	Array<SpecificRegister>* registersUsedForSource,
	FunctionStaticStorage* source)
{
	//go through each storagesUsed list and conflict each register with each other register
	forEach(Array<AssemblyStorage*>*, storagesUsedForInstruction, storagesUsed, baseConflictsI) {
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
	forEach(CALL*, c, calls, callsI) {
		FunctionStaticStorage* f;
		Array<SpecificRegister>* registersUsed;
		//if we know exactly what registers we're using, we can just use those as conflicts
		if (let(FunctionStaticStorage*, f, c->source) && let(Array<SpecificRegister>*, registersUsed, f->registersUsed)) {
			//add conflicts for each storage
			forEach(AssemblyStorage*, storageUsed, storagesUsed->get(c->instructionArrayIndex), storagesUsedI) {
				Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, storageUsed);
				forEach(SpecificRegister, s, registersUsed, conflictingRegisterI) {
					conflictsForStorage->addNonDuplicate(Register::registerFor(s));
				}
			}
			//and also add these registers as being used in this function
			forEach(SpecificRegister, s, registersUsed, conflictingRegisterI) {
				registersUsedForSource->addNonDuplicate(s);
			}
		//otherwise, use all registers as conflicts
		} else {
			unsigned char specificRegisterStart = (unsigned char)SpecificRegister::Register32BitStart;
			unsigned char specificRegisterEnd = (unsigned char)SpecificRegister::Register32BitEnd;
			//add conflicts for each storage
			forEach(AssemblyStorage*, storageUsed, storagesUsed->get(c->instructionArrayIndex), storagesUsedI) {
				Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, storageUsed);
				for (unsigned char s = specificRegisterStart; s < specificRegisterEnd; s++)
					conflictsForStorage->addNonDuplicate(Register::registerFor((SpecificRegister)s));
			}
			//and also add these registers as being used in this function
			for (unsigned char s = specificRegisterStart; s < specificRegisterEnd; s++)
				registersUsedForSource->addNonDuplicate((SpecificRegister)s);
		}
	}
	//and also conflict all the parameters with each other
	forEach(ParameterStorage*, p, source->parameterStorages, firstParametersI) {
		Array<AssemblyStorage*>* conflictsForStorage = storagesListFor(conflictsByStorage, p);
		forEach(ParameterStorage*, p2, source->parameterStorages, secondParametersI) {
			if (p != p2)
				conflictsForStorage->addNonDuplicate(p2);
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
	AssemblyStorage* storageToAssign, SpecificRegister specificRegister, FunctionStaticStorage* errorTokenHolder)
{
	TempStorage* tempToAssign;
	Register* registerToAssign;
	if (let(TempStorage*, tempToAssign, storageToAssign))
		tempToAssign->finalStorage = Register::registerFor(specificRegister);
	else if (let(Register*, registerToAssign, storageToAssign))
		registerToAssign->specificRegister = specificRegister;
	else
		Error::logError(ErrorType::CompilerIssue, "assigning a register value to a variable", errorTokenHolder->sourceFunction);
}
//go through all the instructions and modify any memory pointers on the stack based on which stack shifts we have
void BuildInitialAssembly::shiftStackPointers(FunctionStaticStorage* source) {
	//this is an offset; after we shift the stack we need to add this to our memory pointers to get the same address
	int currentStackShiftAmount = 0;
	//jump instructions will never go past a stack shift so we can just iterate the instructions in order
	forEach(AssemblyInstruction*, instruction, source->instructions, ii) {
		StackShift* stackShift;
		MemoryPointer* memoryPointer;
		if (let(StackShift*, stackShift, instruction)) {
			int nextStackShiftAmount = stackShift->calledFunction != nullptr
				? stackShift->calledFunction->parameterBytesUsed
				: stackShift->functionCallArgumentBytes;
			if (stackShift->beforeFunctionCall) {
				if (currentStackShiftAmount == 0)
					nextStackShiftAmount += stackShift->owningFunction->stackBytesUsed;
				ii.replaceThis(
					new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(nextStackShiftAmount, cpuBitSize))));
				currentStackShiftAmount += nextStackShiftAmount;
			} else {
				if (currentStackShiftAmount == nextStackShiftAmount + stackShift->owningFunction->stackBytesUsed)
					nextStackShiftAmount = currentStackShiftAmount;
				ii.replaceThis(
					new ADD(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(nextStackShiftAmount, cpuBitSize))));
				currentStackShiftAmount -= nextStackShiftAmount;
			}
			delete stackShift;
		} else if (let(MemoryPointer*, memoryPointer, instruction->destination) && !memoryPointer->expectsShiftedStack) {
			instruction->destination =
				globalTrackedStorage(
					new MemoryPointer(
						memoryPointer->primaryRegister,
						memoryPointer->primaryRegisterMultiplierPower,
						memoryPointer->secondaryRegister,
						memoryPointer->constant + currentStackShiftAmount,
						true,
						memoryPointer->bitSize));
		} else if (let(MemoryPointer*, memoryPointer, instruction->source) && !memoryPointer->expectsShiftedStack) {
			instruction->source =
				globalTrackedStorage(
					new MemoryPointer(
						memoryPointer->primaryRegister,
						memoryPointer->primaryRegisterMultiplierPower,
						memoryPointer->secondaryRegister,
						memoryPointer->constant + currentStackShiftAmount,
						true,
						memoryPointer->bitSize));
		}
	}
//TODO: how do we distinguish between parameters for this function (which get the full shift) and arguments (which get a cpuBitSize shift)?
}
//get the bit size to use for the provided type
BitSize BuildInitialAssembly::typeBitSize(CDataType* dt) {
	CPrimitive* p;
	return let(CPrimitive*, p, dt) ? p->bitSize : cpuBitSize;
}
//add the storage to the list and return it (to save callers from needing an extra local variable)
template <class AssemblyStorageType> AssemblyStorageType* BuildInitialAssembly::globalTrackedStorage(AssemblyStorageType* a) {
	buildResult->assemblyStorageToDelete->add(a);
	return a;
}
//get the storage for the variable if we've already set it, or create one and set it before returning it
AssemblyStorage* BuildInitialAssembly::getVariableStorage(CVariableDefinition* variable) {
	AssemblyStorage* variableStorage = variableToStorageMap->get(variable);
	if (variableStorage == nullptr) {
		variableStorage = globalTrackedStorage(new TempStorage(typeBitSize(variable->type)));
		variableToStorageMap->set(variable, variableStorage);
	}
	return variableStorage;
}
//setup a FunctionStaticStorage for the function definition, store it in the build result, and return it
FunctionStaticStorage* BuildInitialAssembly::createAndStoreFunctionStaticStorage(FunctionDefinition* f) {
	FunctionStaticStorage* storage = new FunctionStaticStorage(f, cpuBitSize);
	if (f->returnType != CDataType::voidType)
		storage->resultStorage = Register::newUndecidedRegisterForBitSize(typeBitSize(f->returnType));
	forEach(CVariableDefinition*, parameter, f->parameters, pi) {
		ParameterStorage* parameterStorage = new ParameterStorage(storage, typeBitSize(parameter->type));
		variableToStorageMap->set(parameter, parameterStorage);
		storage->parameterStorages->add(parameterStorage);
	}
	functionDefinitionToStorageMap->set(f, storage);
	buildResult->functionDefinitions->add(storage);
	return storage;
}
//since larger-bit-size values are stored in memory before smaller-bit-size values when grouped together,
//	get the memory offsets that each value will be at
//returns the stack offset in bytes for each value (0 + bytes for preceding values)
Array<int>* BuildInitialAssembly::getBitSizeOrderedStackOffsets(Array<BitSize>* valueBitSizes) {
	Array<int>* orderedStackOffsets = new Array<int>();
	if (valueBitSizes->length == 0)
		return orderedStackOffsets;
	//track a tree of how many bytes are used per bit size
	AVLTree<BitSize, int> bytesPerBitSize;
	//the first time we iterate the values,
	//	we'll track how many bytes the value is offset compared to the "leftmost" value of its bit size
	for (int i = 0; i < valueBitSizes->length; i++) {
		BitSize valueBitSize = valueBitSizes->get(i);
		int stackOffset = bytesPerBitSize.get(valueBitSize);
		orderedStackOffsets->add(stackOffset);
		bytesPerBitSize.set(valueBitSize, stackOffset + (int)(((unsigned char)valueBitSize + 7) / 8));
	}
	//then, shift over the bytes so that each bit size has the total bytes for values of the next-larger bit size
	Array<AVLNode<BitSize, int>*>* bitSizesWithBytes = bytesPerBitSize.entrySet();
	for (int i = 1; i < bitSizesWithBytes->length; i++)
		bytesPerBitSize.set(bitSizesWithBytes->get(i - 1)->key, bitSizesWithBytes->get(i)->value);
	bytesPerBitSize.set(bitSizesWithBytes->get(bitSizesWithBytes->length - 1)->key, 0);
	//then, for each bit size from large to small, add the bytes from the next-bigger bit size so that they cascade so that
	//	each bit size has the total bytes of all values of bigger bit sizes
	for (int i = bitSizesWithBytes->length - 2; i >= 0; i--) {
		AVLNode<BitSize, int>* bitSizeWithBytes = bitSizesWithBytes->get(i);
		bytesPerBitSize.set(bitSizeWithBytes->key, bitSizeWithBytes->value + bitSizesWithBytes->get(i + 1)->value);
	}
	delete bitSizesWithBytes;
	//finally go through all stack offsets and add the bytes that preceded them
	for (int i = 0; i < valueBitSizes->length; i++) {
		int additionalOffset = bytesPerBitSize.get(valueBitSizes->get(i));
		if (additionalOffset > 0)
			orderedStackOffsets->set(i, orderedStackOffsets->get(i) + additionalOffset);
	}
	return orderedStackOffsets;
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
