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
Build::FindUninitializedVariablesVisitor::FindUninitializedVariablesVisitor(
	PrefixTrie<char, CVariableData*>* pVariableData, bool pErrorForUninitializedVariables)
: TokenVisitor(onlyWhenTrackingIDs("FUVVTR"))
, allVariablesAreInitialized(true)
, errorForUninitializedVariables(pErrorForUninitializedVariables)
, variableData(pVariableData) {
}
Build::FindUninitializedVariablesVisitor::~FindUninitializedVariablesVisitor() {
	//don't delete the variable data, something else owns it
}
//go through the expression and make sure all of the variables it uses are initialized
void Build::FindUninitializedVariablesVisitor::handleExpression(Token* t) {
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
thread_local Array<AssemblyInstruction*>* Build::globalAssembly = nullptr;
thread_local Array<StringStaticStorage*>* Build::stringDefinitions = nullptr;
thread_local Array<FunctionStaticStorage*>* Build::functionDefinitions = nullptr;
thread_local Array<AssemblyStorage*>* Build::assemblyStorageToDelete = nullptr;
thread_local BitSize Build::cpuBitSize = BitSize::B32;
thread_local Register* Build::cpuARegister = nullptr;
thread_local Register* Build::cpuCRegister = nullptr;
thread_local Register* Build::cpuDRegister = nullptr;
thread_local Register* Build::cpuBRegister = nullptr;
thread_local Register* Build::cpuSPRegister = nullptr;
thread_local Register* Build::cpuSIRegister = nullptr;
thread_local Register* Build::cpuDIRegister = nullptr;
thread_local FunctionStaticStorage* Build::Main_exit = nullptr;
thread_local FunctionStaticStorage* Build::Main_print = nullptr;
thread_local FunctionStaticStorage* Build::Main_str = nullptr;
thread_local ValueStaticStorage* Build::generalPurposeVariable1 = nullptr;
thread_local ValueStaticStorage* Build::generalPurposeVariable2 = nullptr;
thread_local ValueStaticStorage* Build::generalPurposeVariable3 = nullptr;
thread_local ValueStaticStorage* Build::generalPurposeVariable4 = nullptr;
thread_local ValueStaticStorage* Build::processHeapPointer = nullptr;
thread_local ValueStaticStorage* Build::copperHeapPointer = nullptr;
thread_local ValueStaticStorage* Build::copperHeapNextFreeAddressPointer = nullptr;
thread_local ValueStaticStorage* Build::copperHeapSizePointer = nullptr;
//build the final executable file for each bit size specified
void Build::build(Pliers* pliers) {
	buildForBitSize(pliers, BitSize::B32);
}
//build the final executable file for the specified bit size specified
void Build::buildForBitSize(Pliers* pliers, BitSize pCPUBitSize) {
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

//Array<CVariableDefinition*>* allGlobalVariables = pliers->allFiles->get(0)->variablesVisibleToFile->getValues();
//delete allGlobalVariables;
	setupAssemblyObjects();

	forEach(Operator*, o, &initializationOrder, oi2) {
		addTokenAssembly(o, nullptr);
	}

	//TODO: build the functions
	//TODO: get the actual assembly bytes
	//TODO: write the file

	cleanupAssemblyObjects();
}
//initialize everything used for building
void Build::setupAssemblyObjects() {
	globalAssembly = new Array<AssemblyInstruction*>();
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
void Build::cleanupAssemblyObjects() {
	globalAssembly->deleteContents();
	delete globalAssembly;
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
void Build::build32BitMainFunctions() {
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
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_exitAssembly)),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_exit);
	Main_exitAssembly->add(new RET(Main_exit->val));

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
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_printAssembly)),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_print);
	Main_printAssembly->add(new RET(Main_print->val));

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
	//save the char array in the string and return the string
	Main_strAssembly->add(new MOV(globalTrackedStorage(new MemoryPointer(cpuARegister, 4, BitSize::B32)), cpuDRegister));
	Main_str = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::stringType,
			Array<CVariableDefinition*>::newArrayWith(Main_strIParameter),
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_strAssembly)),
			&functionDefinitionSource),
		BitSize::B32);
	functionDefinitions->add(Main_str);
	Main_strAssembly->add(new RET(Main_str->val));
}
//add the assembly that this token produces to the global assembly array
//returns the storage of the result of the token's assembly
AssemblyStorage* Build::addTokenAssembly(Token* t, CDataType* expectedType) {
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
			return getOperatorAssembly(o);
	} else if (let(FunctionCall*, fc, t))
		return getFunctionCallAssembly(fc);
	else if (let(FunctionDefinition*, fd, t))
		return getFunctionDefinitionStorage(fd, false);
	else if (let(Group*, g, t))
		//TODO: handle groups
		;
	else if (let(IntConstant*, ic, t))
		return getIntConstantStorage(ic, expectedType);
	else if (let(FloatConstant*, fcn, t))
		return getFloatConstantStorage(fcn, expectedType);
	else if (let(BoolConstant*, b, t))
		return globalTrackedStorage(new AssemblyConstant((int)(b->val), BitSize::B1));
	else if (let(StringLiteral*, sl, t))
		return globalTrackedStorage(new StringStaticStorage(sl, cpuBitSize));
	//compiler error
	if (istype(t, ParenthesizedExpression*))
		Error::logError(ErrorType::CompilerIssue, "resulting in an unflattened parenthesized expression", t);
	else
		Error::logError(ErrorType::CompilerIssue, "obtaining assembly for this token", t);
	return cpuARegister;
}
//get the storage of the identifier
AssemblyStorage* Build::getIdentifierStorage(Identifier* i, bool isBeingFunctionCalled) {
	FunctionDefinition* f;
	if (!isBeingFunctionCalled && let(FunctionDefinition*, f, i->variable->initialValue))
		f->eligibleForRegisterParameters = false;
	return i->variable->storage;
}
//get the assembly for the inner token and add any assembly needed to cast it to the expected type
AssemblyStorage* Build::addCastAssembly(Cast* c) {
	AssemblyStorage* result = addTokenAssembly(c->right, c->dataType);
	BitSize targetBitSize = typeBitSize(c->dataType);
	BitSize valueBitSize = result->bitSize;
	if ((unsigned char)valueBitSize < (unsigned char)targetBitSize) {
		Register* newResult = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(targetBitSize));
		globalAssembly->add(new MOVSX(newResult, result));
		result = newResult;
	}
	//TODO: class type checking
	return result;
}
//get the storage pointed to by the static operator
AssemblyStorage* Build::getStaticOperatorStorage(StaticOperator* s) {
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
AssemblyStorage* Build::getOperatorAssembly(Operator* o) {
	AssemblyStorage* resultStorage = cpuARegister;
	VariableDeclarationList* v = nullptr;
	CDataType* expectedType = o->precedence == OperatorTypePrecedence::Comparison
		? CDataType::bestCompatibleType(o->left->dataType, o->right->dataType)
		: o->dataType;
	//operators were rewritten to be postfix so the left token will always be present
	AssemblyStorage* leftStorage =
		o->precedence == OperatorTypePrecedence::Assignment && let(VariableDeclarationList*, v, o->left)
			? nullptr
			: addTokenAssembly(o->left, typeBitSize(o->left->dataType) == BitSize::BInfinite ? expectedType : nullptr);
	AssemblyStorage* rightStorage = o->right != nullptr && o->precedence != OperatorTypePrecedence::Ternary
		? addTokenAssembly(o->right, typeBitSize(o->right->dataType) == BitSize::BInfinite ? expectedType : nullptr)
		: nullptr;
//TODO: ensure we don't have memory-memory or constant-constant or other invalid operands
	switch (o->operatorType) {
		//TODO: classes
		case OperatorType::Dot:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
		//TODO: classes
		case OperatorType::ObjectMemberAccess:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
		case OperatorType::Increment:
//TODO: add assembly
			break;
		case OperatorType::Decrement:
//TODO: add assembly
			break;
		case OperatorType::VariableLogicalNot:
//TODO: add assembly
			break;
		case OperatorType::VariableBitwiseNot:
//TODO: add assembly
			break;
		case OperatorType::VariableNegate:
//TODO: add assembly
			break;
		case OperatorType::LogicalNot:
//TODO: add assembly
			break;
		case OperatorType::BitwiseNot:
//TODO: add assembly
			break;
		case OperatorType::Negate:
//TODO: add assembly
			break;
		case OperatorType::Multiply:
//TODO: add assembly
			break;
		case OperatorType::Divide:
//TODO: add assembly
			break;
		case OperatorType::Modulus:
//TODO: add assembly
			break;
		case OperatorType::Add:
//TODO: add assembly
			break;
		case OperatorType::Subtract:
//TODO: add assembly
			break;
		case OperatorType::ShiftLeft:
//TODO: add assembly
			break;
		case OperatorType::ShiftRight:
//TODO: add assembly
			break;
		case OperatorType::ShiftArithmeticRight:
//TODO: add assembly
			break;
//		case OperatorType::RotateLeft:
//			break;
//		case OperatorType::RotateRight:
//			break;
		case OperatorType::BitwiseAnd:
//TODO: add assembly
			break;
		case OperatorType::BitwiseXor:
//TODO: add assembly
			break;
		case OperatorType::BitwiseOr:
//TODO: add assembly
			break;
		case OperatorType::Equal:
//TODO: add assembly
			break;
		case OperatorType::NotEqual:
//TODO: add assembly
			break;
		case OperatorType::LessOrEqual:
//TODO: add assembly
			break;
		case OperatorType::GreaterOrEqual:
//TODO: add assembly
			break;
		case OperatorType::LessThan:
//TODO: add assembly
			break;
		case OperatorType::GreaterThan:
//TODO: add assembly
			break;
		case OperatorType::BooleanAnd:
//TODO: add assembly
			break;
		case OperatorType::BooleanOr:
//TODO: add assembly
			break;
		case OperatorType::QuestionMark:
//TODO: add assembly
			break;
		case OperatorType::Assign:
//TODO: add assembly
			break;
		case OperatorType::AssignAdd:
//TODO: add assembly
			break;
		case OperatorType::AssignSubtract:
//TODO: add assembly
			break;
		case OperatorType::AssignMultiply:
//TODO: add assembly
			break;
		case OperatorType::AssignDivide:
//TODO: add assembly
			break;
		case OperatorType::AssignModulus:
//TODO: add assembly
			break;
		case OperatorType::AssignShiftLeft:
//TODO: add assembly
			break;
		case OperatorType::AssignShiftRight:
//TODO: add assembly
			break;
		case OperatorType::AssignShiftArithmeticRight:
//TODO: add assembly
			break;
//		case OperatorType::AssignRotateLeft:
//			break;
//		case OperatorType::AssignRotateRight:
//			break;
		case OperatorType::AssignBitwiseAnd:
//TODO: add assembly
			break;
		case OperatorType::AssignBitwiseXor:
//TODO: add assembly
			break;
		case OperatorType::AssignBitwiseOr:
//TODO: add assembly
			break;
//		case OperatorType::AssignBooleanAnd:
//			break;
//		case OperatorType::AssignBooleanOr:
//			break;
		//these should have already been handled
		case OperatorType::None:
		case OperatorType::StaticDot:
		case OperatorType::StaticMemberAccess:
		case OperatorType::Cast:
		case OperatorType::Colon:
		default:
			Error::logError(ErrorType::CompilerIssue, "getting the assembly for this operator", o);
			break;
	}
	if (o->precedence == OperatorTypePrecedence::Assignment) {
		if (o->operatorType == OperatorType::Assign) {
			if (leftStorage != nullptr)
				return addMemoryToMemoryMove(globalAssembly, leftStorage, rightStorage);
			else
//TODO: assign the result to each variable in the variable declaration list
				;
		} else {
			globalAssembly->add(new MOV(leftStorage, resultStorage));
			return resultStorage;
		}
	}
	return resultStorage;
}
//get the assembly for the arguments and the function and call it, returning the value
AssemblyStorage* Build::getFunctionCallAssembly(FunctionCall* f) {
	//get the function
	//if it's a variable, get it here in case the function definition is eligible for register parameters
	Identifier* i;
	FunctionDefinition* fd;
	AssemblyStorage* functionSourceStorage;
	if (let(Identifier*, i, f->function)) {
		functionSourceStorage = getIdentifierStorage(i, true);
		fd = dynamic_cast<FunctionDefinition*>(i->variable->initialValue);
	} else if (let(FunctionDefinition*, fd, f->function)) {
		functionSourceStorage = getFunctionDefinitionStorage(fd, true);
	} else {
		functionSourceStorage = addTokenAssembly(f->function, nullptr);
	}
	AssemblyStorage* functionStorage = globalTrackedStorage(new TempStorage(cpuBitSize));
	globalAssembly->add(new MOV(functionStorage, functionSourceStorage));
	//find out how much we need to shift the stack and shift it
	int shiftTotalBits = 0;
	forEach(Token*, t, f->arguments, ti) {
		shiftTotalBits += Math::max(8, (int)((unsigned char)typeBitSize(t->dataType)));
	}
	globalAssembly->add(
		new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant((shiftTotalBits / 8 + 3) & 0xFFFFFFFC, cpuBitSize))));
	//then go through and add all the arguments
	//if we have a function definition, we can use its temps to save arguments, and return its result storage
	//sometime later we will adjust or remove the SUB for the stack if we were able to use register parameters
	if (fd != nullptr) {
		for (int i = 0; i < f->arguments->length; i++) {
			Token* argument = f->arguments->get(i);
			addMemoryToMemoryMove(
				globalAssembly, fd->parameters->get(i)->storage, addTokenAssembly(argument, argument->dataType));
		}
		return fd->resultStorage;
	//if not, then they will have to be on the stack and return in the *A* register
	//get the positions that they will be
	} else {
		Array<int>* parameterStackOrder = getParameterStackOrder(f->arguments);
		int stackOffset = 0;
		forEach(int, parameterIndex, parameterStackOrder, pi) {
			Token* argument = f->arguments->get(parameterIndex);
			AssemblyStorage* argumentStorage = addTokenAssembly(argument, argument->dataType);
			addMemoryToMemoryMove(
				globalAssembly,
				globalTrackedStorage(new MemoryPointer(cpuSPRegister, 0, nullptr, stackOffset, true, argumentStorage->bitSize)),
				argumentStorage);
			stackOffset += (int)((unsigned char)argumentStorage->bitSize) / 8;
		}
		delete parameterStackOrder;
		return cpuARegister;
	}
}
//get the storage of the function definition
AssemblyStorage* Build::getFunctionDefinitionStorage(FunctionDefinition* f, bool couldBeEligibleForRegisterParameters) {
	if (!couldBeEligibleForRegisterParameters)
		f->eligibleForRegisterParameters = false;
	return globalTrackedStorage(new FunctionStaticStorage(f, cpuBitSize));
}
//get the int constant using the bit size of the expected type
AssemblyStorage* Build::getIntConstantStorage(IntConstant* i, CDataType* expectedType) {
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
AssemblyStorage* Build::getFloatConstantStorage(FloatConstant* f, CDataType* expectedType) {
//TODO: get the storage
return globalTrackedStorage(new AssemblyConstant(0xAAAAAAAA, cpuBitSize));
}
//get the bit size to use for the provided type
BitSize Build::typeBitSize(CDataType* dt) {
	CPrimitive* p;
	return let(CPrimitive*, p, dt) ? p->bitSize : cpuBitSize;
}
//add the storage to the list and return it (to save callers from needing an extra local variable)
template <class AssemblyStorageType> AssemblyStorageType* Build::globalTrackedStorage(AssemblyStorageType* a) {
	assemblyStorageToDelete->add(a);
	return a;
}
//add two MOVs to transfer from (possible) memory pointer to (possible) memory pointer via a transfer register
//returns the register used in the transfer
Register* Build::addMemoryToMemoryMove(
	Array<AssemblyInstruction*>* assembly, AssemblyStorage* destination, AssemblyStorage* source)
{
	Register* transferRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(destination->bitSize));
	assembly->add(new MOV(transferRegister, source));
	assembly->add(new MOV(destination, transferRegister));
	return transferRegister;
}
//since smaller-bit-size parameters will be grouped together, get the indices that each parameter will appear in
Array<int>* Build::getParameterStackOrder(Array<Token*>* parameters) {
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
