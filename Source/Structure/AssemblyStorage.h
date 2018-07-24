#include "../General/globals.h"

class StringLiteral;
class FunctionDefinition;
class Thunk;
enum class BitSize: unsigned char;

enum class SpecificRegister: unsigned char {
	AL,
	CL,
	DL,
	BL,
	AH,
	CH,
	DH,
	BH,
	Undecided8BitRegister,
	Register8BitStart = AL,
	Register8BitEnd = Undecided8BitRegister,
	AX,
	CX,
	DX,
	BX,
	SP,
	BP,
	SI,
	DI,
	Undecided16BitRegister,
	Register16BitStart = AX,
	Register16BitEnd = Undecided16BitRegister,
	EAX,
	ECX,
	EDX,
	EBX,
	ESP,
	EBP,
	ESI,
	EDI,
	Undecided32BitRegister,
	Register32BitStart = EAX,
	Register32BitEnd = Undecided32BitRegister,
	SpecificRegisterCount
};

//assembly storage
class AssemblyStorage onlyInDebug(: public ObjCounter) {
public:
	BitSize bitSize; //copper: readonly

protected:
	AssemblyStorage(onlyWhenTrackingIDs(char* pObjType COMMA) BitSize pBitSize);
public:
	virtual ~AssemblyStorage();
};
class StaticStorage: public AssemblyStorage {
protected:
	StaticStorage(onlyWhenTrackingIDs(char* pObjType COMMA) BitSize pBitSize);
public:
	virtual ~StaticStorage();
};
class AssemblyConstant: public AssemblyStorage {
public:
	int val; //copper: readonly

	AssemblyConstant(int pVal, BitSize pBitSize);
	virtual ~AssemblyConstant();
};
class Register: public AssemblyStorage {
private:
	static Register** allRegisters;

public:
	SpecificRegister specificRegister; //copper: readonly

	Register(SpecificRegister pSpecificRegister);
	virtual ~Register();

	static Register* registerFor(SpecificRegister pSpecificRegister);
	static BitSize bitSizeForSpecificRegister(SpecificRegister pSpecificRegister);
	static Register* aRegisterForBitSize(BitSize pBitSize);
	static Register* cRegisterForBitSize(BitSize pBitSize);
	static Register* dRegisterForBitSize(BitSize pBitSize);
	static Register* bRegisterForBitSize(BitSize pBitSize);
	static Register* spRegisterForBitSize(BitSize pBitSize);
	static Register* siRegisterForBitSize(BitSize pBitSize);
	static Register* diRegisterForBitSize(BitSize pBitSize);
	static Register* newUndecidedRegisterForBitSize(BitSize pBitSize);
};
class MemoryPointer: public AssemblyStorage {
private:
	Register* primaryRegister;
	unsigned char primaryRegisterMultiplierPower;
	Register* secondaryRegister;
	int constant;
	bool expectsShiftedStack;

public:
	MemoryPointer(
		Register* pPrimaryRegister,
		unsigned char pPrimaryRegisterMultiplierPower,
		Register* pSecondaryRegister,
		int pConstant,
		bool pExpectsShiftedStack,
		BitSize pBitSize);
	MemoryPointer(Register* pPrimaryRegister, int pConstant, BitSize pBitSize)
		: MemoryPointer(pPrimaryRegister, 0, nullptr, pConstant, false, pBitSize) {}
	virtual ~MemoryPointer();
};
class ValueStaticStorage: public StaticStorage {
public:
	ValueStaticStorage(BitSize pBitSize);
	virtual ~ValueStaticStorage();
};
class StringStaticStorage: public StaticStorage {
private:
	StringLiteral* val;

public:
	StringStaticStorage(StringLiteral* pVal, BitSize pointerBitSize);
	virtual ~StringStaticStorage();
};
class FunctionStaticStorage: public StaticStorage {
public:
	FunctionDefinition* val; //copper: readonly

	FunctionStaticStorage(FunctionDefinition* pVal, BitSize pointerBitSize);
	virtual ~FunctionStaticStorage();
};
class ThunkStaticStorage: public StaticStorage {
private:
	Thunk* val;

public:
	ThunkStaticStorage(Thunk* pVal, BitSize pointerBitSize);
	virtual ~ThunkStaticStorage();
};
class StaticAddress: public AssemblyStorage {
private:
	StaticStorage* storage;

public:
	StaticAddress(StaticStorage* pStorage, BitSize pointerBitSize);
	virtual ~StaticAddress();
};
class TempStorage: public AssemblyStorage {
private:
	AssemblyStorage* finalStorage;

public:
	TempStorage(BitSize pBitSize);
	virtual ~TempStorage();
};

//miscellaneous assembly utility classes
class Thunk onlyInDebug(: public ObjCounter) {
private:
	string name;
	unsigned short thunkID;

public:
	Thunk(string pName, unsigned short pThunkID);
	virtual ~Thunk();
};
class ConditionLabelPair onlyInDebug(: public ObjCounter) {
public:
	AssemblyLabel* trueJumpDest; //copper: readonly
	AssemblyLabel* falseJumpDest; //copper: readonly

	ConditionLabelPair(AssemblyLabel* pTrueJumpDest, AssemblyLabel* pFalseJumpDest);
	virtual ~ConditionLabelPair();
};
