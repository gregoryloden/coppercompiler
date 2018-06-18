#include "Project.h"
#ifdef WIN32
	#include <Windows.h>
#endif

//build the final executable file
//TODO: finalize byte sizes for primitive types

//variable data: global variable data stored in tree
//each scope has a PrefixTrieUnion with the data
//when adding variable data:
//use the PrefixTrie check to see if it's got one already
//if not: make a new one, and if the parent has one (getFromParent), port the data from that one to the new one

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
thread_local unsigned char Build::cpuBitSize = 0;
thread_local Register* Build::cpuARegister = nullptr;
thread_local Register* Build::cpuSPRegister = nullptr;
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
	buildForBitSize(pliers, 32);
}
//build the final executable file for the specified bit size specified
void Build::buildForBitSize(Pliers* pliers, unsigned char pCPUBitSize) {
	cpuBitSize = pCPUBitSize;
	if (cpuBitSize != 32) {
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

	//TODO: build

	cleanupAssemblyObjects();
}
//initialize everything used for building
void Build::setupAssemblyObjects() {
	globalAssembly = new Array<AssemblyInstruction*>();
	stringDefinitions = new Array<StringStaticStorage*>();
	functionDefinitions = new Array<FunctionStaticStorage*>();
	assemblyStorageToDelete = new Array<AssemblyStorage*>();
	cpuARegister = Register::aRegisterForBitSize(cpuBitSize);
	cpuSPRegister = Register::spRegisterForBitSize(cpuBitSize);
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
	AssemblyStorage* argumentStorage1 = getArgumentStorage(0);
	AssemblyStorage* argumentStorage2 = getArgumentStorage(1);
	AssemblyStorage* argumentStorage3 = getArgumentStorage(2);
	AssemblyStorage* argumentStorage4 = getArgumentStorage(3);
	AssemblyStorage* argumentStorage5 = getArgumentStorage(4);
	AssemblyConstant* constant4Storage = globalTrackedStorage(new AssemblyConstant(4, 32));

	CVariableDefinition* Main_exitIParameter = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_exitAssembly = new Array<AssemblyInstruction*>();
	Main_exitAssembly->add(new SUB(cpuSPRegister, constant4Storage));
	Main_exitAssembly->add(
		new MOV(argumentStorage1, globalTrackedStorage(new OffsetMemoryPointer(Main_exitIParameter->storage, 4))));
	Main_exitAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TExitProcess, 32))));
	Main_exitAssembly->add(new RET(4));
	Main_exit = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::voidType,
			Array<CVariableDefinition*>::newArrayWith(Main_exitIParameter),
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_exitAssembly)),
			&functionDefinitionSource),
		32);
	functionDefinitions->add(Main_exit);

	CVariableDefinition* Main_printSParameter =
		new CVariableDefinition(CDataType::stringType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_printAssembly = new Array<AssemblyInstruction*>();
	Register* Main_printStringCharArrayRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(32));
	Register* Main_printStringCharArrayDataRegister = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(32));
	Main_printAssembly->add(new SUB(cpuSPRegister, globalTrackedStorage(new AssemblyConstant(20, 32))));
	Main_printAssembly->add(new SUB(cpuSPRegister, constant4Storage));
	Main_printAssembly->add(new MOV(argumentStorage1, globalTrackedStorage(new AssemblyConstant(STD_OUTPUT_HANDLE, 32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TGetStdHandle, 32))));
	Main_printAssembly->add(new MOV(argumentStorage1, cpuARegister));
	Main_printAssembly->add(
		new MOV(
			Main_printStringCharArrayRegister,
			globalTrackedStorage(new OffsetMemoryPointer(Main_printSParameter->storage, 20))));
	Main_printAssembly->add(
		new MOV(
			Main_printStringCharArrayRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 4, 32))));
	Main_printAssembly->add(
		new LEA(
			Main_printStringCharArrayDataRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 8, 32))));
	Main_printAssembly->add(new MOV(argumentStorage2, Main_printStringCharArrayDataRegister));
	Main_printAssembly->add(
		new MOV(
			Main_printStringCharArrayDataRegister,
			globalTrackedStorage(new MemoryPointer(Main_printStringCharArrayRegister, 4, 32))));
	Main_printAssembly->add(new MOV(argumentStorage3, Main_printStringCharArrayDataRegister));
	Main_printAssembly->add(new MOV(argumentStorage4, globalTrackedStorage(new StaticAddress(generalPurposeVariable1, 32))));
	Main_printAssembly->add(new MOV(argumentStorage5, globalTrackedStorage(new AssemblyConstant(0, 32))));
	Main_printAssembly->add(new CALL(globalTrackedStorage(new ThunkStaticStorage(&TWriteFile, 32))));
	Main_printAssembly->add(new RET(4));
	Main_print = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::voidType,
			Array<CVariableDefinition*>::newArrayWith(Main_printSParameter),
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_printAssembly)),
			&functionDefinitionSource),
		32);
	functionDefinitions->add(Main_print);

	CVariableDefinition* Main_strIParameter = new CVariableDefinition(CDataType::intType, new Identifier("", 0, 0, nullptr));
	Array<AssemblyInstruction*>* Main_strAssembly = new Array<AssemblyInstruction*>();
	Main_str = new FunctionStaticStorage(
		new FunctionDefinition(
			CDataType::stringType,
			Array<CVariableDefinition*>::newArrayWith(Main_strIParameter),
			Array<Statement*>::newArrayWith(new AssemblyStatement(Main_strAssembly)),
			&functionDefinitionSource),
		32);
Main_printAssembly->add(new MOV(cpuARegister, globalTrackedStorage(new AssemblyConstant(0, 32))));
	Main_printAssembly->add(new RET(4));
	//TODO: add the body of str
	functionDefinitions->add(Main_str);
}
//add the assembly that this token produces to the global assembly array
//returns the storage of the result of the token's assembly
AssemblyStorage* Build::addTokenAssembly(Token* t, CDataType* expectedType) {
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
		return i->variable->storage;
	else if (let(DirectiveTitle*, d, t))
		//TODO: handle directive titles
		;
	else if (let(ParenthesizedExpression*, p, t))
		return addTokenAssembly(p->expression, expectedType);
	else if (let(Cast*, c, t))
		return addCastAssembly(c);
	else if (let(StaticOperator*, s, t))
		return getStaticOperatorStorage(s);
	else if (let(Operator*, o, t))
return nullptr;
	else if (let(FunctionCall*, fc, t))
return nullptr;
	else if (let(FunctionDefinition*, fd, t))
return nullptr;
	else if (let(Group*, g, t))
		//TODO: handle groups
		;
	else if (istype(t, IntConstant*) || istype(t, FloatConstant*) || istype(t, BoolConstant*) || istype(t, StringLiteral*))
return nullptr;
	else if (let(VariableDeclarationList*, v, t))
return nullptr;
	Error::logError(ErrorType::CompilerIssue, "obtaining assembly for this token", t);
	return globalTrackedStorage(new TempStorage(8));
}
//get the assembly for the inner token and add any assembly needed to cast it to the expected type
AssemblyStorage* Build::addCastAssembly(Cast* c) {
	AssemblyStorage* result = addTokenAssembly(c->right, c->dataType);
	int targetBitSize = typeBitSize(c->dataType);
	int valueBitSize = result->bitSize;
	if (valueBitSize < targetBitSize) {
		Register* newResult = globalTrackedStorage(Register::newUndecidedRegisterForBitSize(valueBitSize));
		globalAssembly->add(new MOVSX(newResult, result));
		result = newResult;
	}
	//TODO: class type checking
	return result;
}
//get the assembly for the inner token and add any assembly needed to cast it to the expected type
AssemblyStorage* Build::getStaticOperatorStorage(StaticOperator* s) {
//TODO: handle static operators
return cpuARegister;
}
//get the bit size to use for the provided type
int Build::typeBitSize(CDataType* dt) {
	CPrimitive* p;
	return let(CPrimitive*, p, dt) ? p->bitSize : cpuBitSize;
}
//add the storage to the list and return it (to save callers from needing an extra local variable)
template <class AssemblyStorageType> AssemblyStorageType* Build::globalTrackedStorage(AssemblyStorageType* a) {
	assemblyStorageToDelete->add(a);
	return a;
}
//get a memory pointer to one of the arguments (assuming *SP was already subtracted)
MemoryPointer* Build::getArgumentStorage(unsigned char argumentIndex) {
	return globalTrackedStorage(new MemoryPointer(cpuSPRegister, cpuBitSize * argumentIndex, cpuBitSize));
}
