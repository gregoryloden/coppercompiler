#include "../Structure/Tokens.h"

class Pliers;
class CVariableData;
class AssemblyInstruction;
class AssemblyStorage;
class AssemblyConstant;
class ConditionLabelPair;
class TempStorage;
class CDataType;
class Register;
class StringStaticStorage;
class FunctionStaticStorage;
class FunctionDefinition;
class BuildResult;
class ValueStaticStorage;
class MemoryPointer;
class Statement;
class CALL;
enum class BitSize: unsigned char;
enum class SpecificRegister: unsigned char;
enum class ConflictRegister: unsigned char;
template <class Key, class Value> class AVLTree;
template <class Key, class Value> class AVLNode;
template <class Key, class Value> class InsertionOrderedAVLTree;
template <class KeyElement, class Value> class PrefixTrie;
template <class Type> class Array;
template <class Type1, class Type2> class Group2;

class BuildInitialAssembly {
private:
	class FindUninitializedVariablesVisitor: public TokenVisitor {
	public:
		bool allVariablesAreInitialized; //copper: readonly
	private:
		PrefixTrie<char, CVariableData*>* variableData;
		bool errorForUninitializedVariables;

	public:
		FindUninitializedVariablesVisitor(
			PrefixTrie<char, CVariableData*>* pVariableData, bool pErrorForUninitializedVariables);
		virtual ~FindUninitializedVariablesVisitor();

		void handleExpression(Token* t);
	};

	static thread_local FunctionStaticStorage* currentFunction;
	static thread_local BuildResult* buildResult;
	static thread_local AVLTree<FunctionDefinition*, FunctionStaticStorage*>* functionDefinitionToStorageMap;
	static thread_local AVLTree<CVariableDefinition*, AssemblyStorage*>* variableToStorageMap;
	static thread_local BitSize cpuBitSize;
	static thread_local int cpuByteSize;
	static thread_local Register* cpuARegister;
	static thread_local Register* cpuCRegister;
	static thread_local Register* cpuDRegister;
	static thread_local Register* cpuBRegister;
	static thread_local Register* cpuSPRegister;
	static thread_local Register* cpuSIRegister;
	static thread_local Register* cpuDIRegister;
	static thread_local ValueStaticStorage* generalPurposeVariable1;
	static thread_local ValueStaticStorage* generalPurposeVariable2;
	static thread_local ValueStaticStorage* generalPurposeVariable3;
	static thread_local ValueStaticStorage* generalPurposeVariable4;
	static thread_local ValueStaticStorage* processHeapPointer;
	static thread_local ValueStaticStorage* copperHeapPointer;
	static thread_local ValueStaticStorage* copperHeapNextFreeAddressPointer;
	static thread_local ValueStaticStorage* copperHeapSizePointer;

public:
	static BuildResult* buildInitialAssembly(Pliers* pliers, BitSize pCPUBitSize);
private:
	static void setupAssemblyObjects();
	static void cleanupAssemblyObjects();
	static void build32BitMainFunctions();
	static void addGlobalVariableInitializations(Pliers* pliers);
	static AssemblyStorage* addTokenAssembly(Token* t, CDataType* expectedType, ConditionLabelPair* jumpDests);
	static AssemblyStorage* getIdentifierStorage(Identifier* i, bool isBeingFunctionCalled);
	static AssemblyStorage* addCastAssembly(Cast* c);
	static AssemblyStorage* getStaticOperatorStorage(StaticOperator* s);
	static Register* addOperatorAssembly(Operator* o, CDataType* expectedType, ConditionLabelPair* jumpDests);
	static AssemblyStorage* addFunctionCallAssembly(FunctionCall* f);
	static FunctionStaticStorage* getFunctionDefinitionStorage(
		FunctionDefinition* f, bool couldBeEligibleForRegisterParameters);
	static AssemblyConstant* getIntConstantStorage(IntConstant* i, CDataType* expectedType);
	static AssemblyConstant* getFloatConstantStorage(FloatConstant* f, CDataType* expectedType);
	static void addStatementListAssembly(Array<Statement*>* statements);
	static void addConditionAssembly(Token* t, Register* resultStorage, CDataType* expectedType, ConditionLabelPair* jumpDests);
	static Register* addMemoryToMemoryMove(AssemblyStorage* destination, AssemblyStorage* source);
	static void addBooleanJump(AssemblyStorage* source, ConditionLabelPair* jumpDests);
	static void assignTemps(FunctionStaticStorage* source);
	static void trackStoragesUsed(
		Array<AssemblyInstruction*>* instructions,
		Array<Array<AssemblyStorage*>*>* storagesUsed,
		AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* potentialSameStoragesByStorage,
		Array<CALL*>* calls);
	static void markConflicts(
		Array<Array<AssemblyStorage*>*>* storagesUsed,
		AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* conflictsByStorage,
		Array<CALL*>* calls,
		Array<AVLNode<AssemblyStorage*, Array<AssemblyStorage*>*>*>* storagesWithPotentialSameStorages,
		Array<SpecificRegister>* registersUsedForSource,
		FunctionStaticStorage* source);
	static void assignRegisterFinalStorages(
		InsertionOrderedAVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* conflictsByStorage,
		Array<AVLNode<AssemblyStorage*, Array<AssemblyStorage*>*>*>* storagesWithPotentialSameStorages,
		Array<TempStorage*>* tempsToAssignOnStack,
		FunctionStaticStorage* source);
	static int setConflictRegisters(bool* conflictRegisters, Array<AssemblyStorage*>* conflictStorages);
	static Group2<ConflictRegister, int> getFirstAvailableRegister(BitSize bitSize, bool* conflictRegisters);
	static void setStorageToRegister(
		AssemblyStorage* storageToAssign, SpecificRegister specificRegister, FunctionStaticStorage* errorTokenHolder);
	static void assignStackFinalStorages(
		Array<TempStorage*>* tempsToAssignOnStack,
		AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* conflictsByStorage,
		FunctionStaticStorage* source);
	static void shiftStackPointers(FunctionStaticStorage* source);
	static BitSize typeBitSize(CDataType* dt);
	template <class AssemblyStorageType> static AssemblyStorageType* globalTrackedStorage(AssemblyStorageType* a);
	static AssemblyStorage* getVariableStorage(CVariableDefinition* variable);
	static FunctionStaticStorage* createAndStoreFunctionStaticStorage(FunctionDefinition* f);
	static Array<int>* getBitSizeOrderedStackOffsets(Array<BitSize>* valueBitSizes);
	static Array<AssemblyStorage*>* storagesListFor(
		AVLTree<AssemblyStorage*, Array<AssemblyStorage*>*>* storages, AssemblyStorage* key);
};
