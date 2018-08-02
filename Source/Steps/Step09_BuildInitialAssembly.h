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
class CVariableDefinition;
class ValueStaticStorage;
class MemoryPointer;
class Statement;
enum class BitSize: unsigned char;
template <class KeyElement, class Value> class PrefixTrie;
template <class Type> class Array;

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

	static thread_local Array<AssemblyInstruction*>* currentAssembly;
	static thread_local Array<FunctionDefinition*>* currentTempAssignmentDependencies;
	static thread_local Array<AssemblyInstruction*>* globalInitAssembly;
	static thread_local Array<StringStaticStorage*>* stringDefinitions;
	static thread_local Array<FunctionStaticStorage*>* functionDefinitions;
	static thread_local Array<AssemblyStorage*>* assemblyStorageToDelete;
	static thread_local BitSize cpuBitSize;
	static thread_local Register* cpuARegister;
	static thread_local Register* cpuCRegister;
	static thread_local Register* cpuDRegister;
	static thread_local Register* cpuBRegister;
	static thread_local Register* cpuSPRegister;
	static thread_local Register* cpuSIRegister;
	static thread_local Register* cpuDIRegister;
	static thread_local FunctionStaticStorage* Main_exit;
	static thread_local FunctionStaticStorage* Main_print;
	static thread_local FunctionStaticStorage* Main_str;
	static thread_local ValueStaticStorage* generalPurposeVariable1;
	static thread_local ValueStaticStorage* generalPurposeVariable2;
	static thread_local ValueStaticStorage* generalPurposeVariable3;
	static thread_local ValueStaticStorage* generalPurposeVariable4;
	static thread_local ValueStaticStorage* processHeapPointer;
	static thread_local ValueStaticStorage* copperHeapPointer;
	static thread_local ValueStaticStorage* copperHeapNextFreeAddressPointer;
	static thread_local ValueStaticStorage* copperHeapSizePointer;

public:
	static void buildInitialAssembly(Pliers* pliers);
private:
	static void buildInitialAssemblyForBitSize(Pliers* pliers, BitSize pCPUBitSize);
	static void setupAssemblyObjects();
	static void cleanupAssemblyObjects();
	static void build32BitMainFunctions();
	static AssemblyStorage* addTokenAssembly(Token* t, CDataType* expectedType, ConditionLabelPair* jumpDests);
	static AssemblyStorage* getIdentifierStorage(Identifier* i);
	static AssemblyStorage* addCastAssembly(Cast* c);
	static AssemblyStorage* getStaticOperatorStorage(StaticOperator* s);
	static Register* getOperatorAssembly(Operator* o, ConditionLabelPair* jumpDests);
	static void getConditionAssembly(Token* t, Register* resultStorage, CDataType* expectedType, ConditionLabelPair* jumpDests);
	static AssemblyStorage* getFunctionCallAssembly(FunctionCall* f);
	static FunctionStaticStorage* getFunctionDefinitionStorage(FunctionDefinition* f);
	static AssemblyConstant* getIntConstantStorage(IntConstant* i, CDataType* expectedType);
	static AssemblyConstant* getFloatConstantStorage(FloatConstant* f, CDataType* expectedType);
	static void addStatementListAssembly(Array<Statement*>* statements, FunctionDefinition* sourceFunction);
	static bool assignTemps(FunctionDefinition* f, bool ignoreDependencies);
	static BitSize typeBitSize(CDataType* dt);
	template <class AssemblyStorageType> static AssemblyStorageType* globalTrackedStorage(AssemblyStorageType* a);
	static Register* addMemoryToMemoryMove(
		Array<AssemblyInstruction*>* assembly, AssemblyStorage* destination, AssemblyStorage* source);
	static Array<int>* getParameterStackOrder(Array<Token*>* parameters);
};
