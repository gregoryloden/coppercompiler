#include "../General/globals.h"

class StringLiteral;
class FunctionDefinition;
class Thunk;

enum class SpecificRegister: unsigned char {
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
	SpecificRegisterCount
};

//assembly storage
class AssemblyStorage onlyInDebug(: public ObjCounter) {
public:
	unsigned char bitSize; //copper: readonly

protected:
	AssemblyStorage(onlyWhenTrackingIDs(char* pObjType COMMA) unsigned char pBitSize);
public:
	virtual ~AssemblyStorage();
};
class StaticStorage: public AssemblyStorage {
protected:
	StaticStorage(onlyWhenTrackingIDs(char* pObjType COMMA) unsigned char pBitSize);
public:
	virtual ~StaticStorage();
};
class AssemblyConstant: public AssemblyStorage {
public:
	int val; //copper: readonly

	AssemblyConstant(int pVal, unsigned char pBitSize);
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
	static unsigned char bitSizeForSpecificRegister(SpecificRegister pSpecificRegister);
	static Register* aRegisterForBitSize(unsigned char pBitSize);
	static Register* spRegisterForBitSize(unsigned char pBitSize);
	static Register* newUndecidedRegisterForBitSize(unsigned char pBitSize);
};
class MemoryPointer: public AssemblyStorage {
private:
	AssemblyStorage* primaryRegister;
	unsigned char primaryRegisterMultiplierPower;
	AssemblyStorage* secondaryRegister;
	int constant;

public:
	MemoryPointer(
		AssemblyStorage* pPrimaryRegister,
		unsigned char pPrimaryRegisterMultiplierPower,
		AssemblyStorage* pSecondaryRegister,
		int pConstant,
		unsigned char pBitSize);
	MemoryPointer(AssemblyStorage* pPrimaryRegister, int pConstant, unsigned char pBitSize)
		: MemoryPointer(pPrimaryRegister, 0, nullptr, pConstant, pBitSize) {}
	virtual ~MemoryPointer();
};
class OffsetMemoryPointer: public AssemblyStorage {
private:
	AssemblyStorage* storage;
	int offset;

public:
	OffsetMemoryPointer(AssemblyStorage* pStorage, int pOffset);
	virtual ~OffsetMemoryPointer();
};
class ValueStaticStorage: public StaticStorage {
public:
	ValueStaticStorage(unsigned char pBitSize);
	virtual ~ValueStaticStorage();
};
class StringStaticStorage: public StaticStorage {
private:
	StringLiteral* val;

public:
	StringStaticStorage(StringLiteral* pVal, unsigned char pointerBitSize);
	virtual ~StringStaticStorage();
};
class FunctionStaticStorage: public StaticStorage {
public:
	FunctionDefinition* val; //copper: readonly

	FunctionStaticStorage(FunctionDefinition* pVal, unsigned char pointerBitSize);
	virtual ~FunctionStaticStorage();
};
class ThunkStaticStorage: public StaticStorage {
private:
	Thunk* val;

public:
	ThunkStaticStorage(Thunk* pVal, unsigned char pointerBitSize);
	virtual ~ThunkStaticStorage();
};
class StaticAddress: public AssemblyStorage {
private:
	StaticStorage* storage;

public:
	StaticAddress(StaticStorage* pStorage, unsigned char pointerBitSize);
	virtual ~StaticAddress();
};
class TempStorage: public AssemblyStorage {
private:
	AssemblyStorage* finalStorage;

public:
	TempStorage(unsigned char pBitSize);
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
