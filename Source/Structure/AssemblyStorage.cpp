#include "Project.h"

AssemblyStorage::AssemblyStorage(onlyWhenTrackingIDs(char* pObjType COMMA) BitSize pBitSize)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
bitSize(pBitSize) {
}
AssemblyStorage::~AssemblyStorage() {}
StaticStorage::StaticStorage(onlyWhenTrackingIDs(char* pObjType COMMA) BitSize pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs(pObjType COMMA) pBitSize) {
}
StaticStorage::~StaticStorage() {}
AssemblyConstant::AssemblyConstant(int pVal, BitSize pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("ASMCNST" COMMA) pBitSize)
, val(pVal) {
}
AssemblyConstant::~AssemblyConstant() {}
Register** Register::allRegisters = []() -> Register** {
	Register** val = new Register*[(unsigned char)SpecificRegister::SpecificRegisterCount];
	for (unsigned char i = 0;
			i < (unsigned char)SpecificRegister::SpecificRegisterCount;
			i++)
		val[i] = new Register((SpecificRegister)i);
	return val;
}();
Register::Register(SpecificRegister pSpecificRegister)
: AssemblyStorage(onlyWhenTrackingIDs("REGISTR" COMMA) bitSizeForSpecificRegister(pSpecificRegister))
, specificRegister(pSpecificRegister) {
}
Register::~Register() {}
//return one of the cached registers for this specific register
Register* Register::registerFor(SpecificRegister pSpecificRegister) {
	return allRegisters[(unsigned char)pSpecificRegister];
}
//convert the enum value to a bit count
BitSize Register::bitSizeForSpecificRegister(SpecificRegister pSpecificRegister) {
	if (((unsigned char)pSpecificRegister) >= ((unsigned char)SpecificRegister::Register32BitStart))
		return BitSize::B32;
	else if (((unsigned char)pSpecificRegister) >= ((unsigned char)SpecificRegister::Register16BitStart))
		return BitSize::B16;
	else
		return BitSize::B8;
}
//get an *A* register for the provided bit size
Register* Register::aRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::EAX :
		pBitSize == BitSize::B16 ? SpecificRegister::AX :
		SpecificRegister::AL);
}
//get a *C* register for the provided bit size
Register* Register::cRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::ECX :
		pBitSize == BitSize::B16 ? SpecificRegister::CX :
		SpecificRegister::CL);
}
//get a *D* register for the provided bit size
Register* Register::dRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::EDX :
		pBitSize == BitSize::B16 ? SpecificRegister::DX :
		SpecificRegister::DL);
}
//get a *B* register for the provided bit size
Register* Register::bRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::EBX :
		pBitSize == BitSize::B16 ? SpecificRegister::BX :
		SpecificRegister::BL);
}
//get an *SP register for the provided bit size
Register* Register::spRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::ESP :
		SpecificRegister::SP);
}
//get an *SI register for the provided bit size
Register* Register::siRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::ESI :
		SpecificRegister::SI);
}
//get a *DI register for the provided bit size
Register* Register::diRegisterForBitSize(BitSize pBitSize) {
	return registerFor(
		pBitSize == BitSize::B32 ? SpecificRegister::EDI :
		SpecificRegister::DI);
}
//get a new non-cached undecided register for the provided bit size
Register* Register::newUndecidedRegisterForBitSize(BitSize pBitSize) {
	return new Register(
		pBitSize == BitSize::B32 ? SpecificRegister::Undecided32BitRegister :
		pBitSize == BitSize::B16 ? SpecificRegister::Undecided16BitRegister :
		SpecificRegister::Undecided8BitRegister);
}
//get the conflict register that corresponds to this register's specific register
//if it's more than 8 bits, use the lower one and the call site will handle looking at the other conflict
ConflictRegister Register::getConflictRegister() {
	if (bitSize == BitSize::B32) {
		if (specificRegister == SpecificRegister::Undecided32BitRegister)
			return ConflictRegister::ConflictRegisterCount;
		if ((unsigned char)specificRegister >= (unsigned char)SpecificRegister::ESP)
			return (ConflictRegister)(
				((unsigned char)specificRegister) - (unsigned char)SpecificRegister::ESP + (unsigned char)ConflictRegister::SP);
		else
			return (ConflictRegister)(
				((unsigned char)specificRegister - (unsigned char)SpecificRegister::Register32BitStart) * 2);
	} else if (bitSize == BitSize::B16) {
		if (specificRegister == SpecificRegister::Undecided16BitRegister)
			return ConflictRegister::ConflictRegisterCount;
		if ((unsigned char)specificRegister >= (unsigned char)SpecificRegister::SP)
			return (ConflictRegister)(
				((unsigned char)specificRegister) - (unsigned char)SpecificRegister::SP + (unsigned char)ConflictRegister::SP);
		else
			return (ConflictRegister)(
				((unsigned char)specificRegister - (unsigned char)SpecificRegister::Register32BitStart) * 2);
	} else {
		if (specificRegister == SpecificRegister::Undecided8BitRegister)
			return ConflictRegister::ConflictRegisterCount;
		if ((unsigned char)specificRegister >= (unsigned char)SpecificRegister::AH)
			return (ConflictRegister)(
				(((unsigned char)specificRegister) - (unsigned char)SpecificRegister::AH) * 2
					+ (unsigned char)ConflictRegister::AH);
		else
			return (ConflictRegister)((unsigned char)specificRegister * 2);
	}
	return ConflictRegister::ConflictRegisterCount;
}
//mark any conflicts in the array
//returns whether there were conflicts to mark (ie the register was not undecided)
bool Register::markConflicts(bool* conflicts) {
	unsigned char conflictRegister = (unsigned char)getConflictRegister();
	if (conflictRegister == (unsigned char)ConflictRegister::ConflictRegisterCount)
		return false;
	conflicts[conflictRegister] = true;
	if (conflictRegister < (unsigned char)ConflictRegister::AH && (unsigned char)bitSize > (unsigned char)BitSize::B8)
		conflicts[conflictRegister + 1] = true;
	return true;
}
//returns whether this register conflicts with any other registersf, or if it hasn't been decided yet
bool Register::isConflictOrUnknown(bool* conflicts) {
	unsigned char conflictRegister = (unsigned char)getConflictRegister();
	return conflictRegister == (unsigned char)ConflictRegister::ConflictRegisterCount
		|| conflicts[conflictRegister]
		|| ((unsigned char)bitSize > (unsigned char)BitSize::B8 && conflicts[conflictRegister + 1]);
}
//returns the specific register that corresponds to the conflict register using the given bit size
SpecificRegister Register::specificRegisterFor(ConflictRegister conflictRegister, BitSize pBitSize) {
	if (pBitSize == BitSize::B32) {
		if ((unsigned char)conflictRegister >= (unsigned char)ConflictRegister::SP)
			return (SpecificRegister)(
				(unsigned char)conflictRegister - (unsigned char)ConflictRegister::SP + (unsigned char)SpecificRegister::ESP);
		else
			return (SpecificRegister)((unsigned char)conflictRegister / 2 + (unsigned char)SpecificRegister::EAX);
	} else if (pBitSize == BitSize::B16) {
		if ((unsigned char)conflictRegister >= (unsigned char)ConflictRegister::SP)
			return (SpecificRegister)(
				(unsigned char)conflictRegister - (unsigned char)ConflictRegister::SP + (unsigned char)SpecificRegister::SP);
		else
			return (SpecificRegister)((unsigned char)conflictRegister / 2 + (unsigned char)SpecificRegister::AX);
	} else
		return (SpecificRegister)(
			(unsigned char)conflictRegister / 2
				+ ((unsigned char)conflictRegister & 1) * 4
				+ (unsigned char)SpecificRegister::AL);
}
MemoryPointer::MemoryPointer(
	Register* pPrimaryRegister,
	unsigned char pPrimaryRegisterMultiplierPower,
	Register* pSecondaryRegister,
	int pConstant,
	bool pExpectsShiftedStack,
	BitSize pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("MEMPTR" COMMA) pBitSize)
, primaryRegister(pPrimaryRegister)
, primaryRegisterMultiplierPower(pPrimaryRegisterMultiplierPower)
, secondaryRegister(pSecondaryRegister)
, constant(pConstant)
, expectsShiftedStack(pExpectsShiftedStack) {
}
MemoryPointer::~MemoryPointer() {}
ValueStaticStorage::ValueStaticStorage(BitSize pBitSize)
: StaticStorage(onlyWhenTrackingIDs("VALSSTG" COMMA) pBitSize) {
}
ValueStaticStorage::~ValueStaticStorage() {}
StringStaticStorage::StringStaticStorage(StringLiteral* pVal, BitSize pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("STRSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
StringStaticStorage::~StringStaticStorage() {
	//don't delete the string, something else owns it
}
FunctionStaticStorage::FunctionStaticStorage(FunctionDefinition* pVal, BitSize pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("FNCSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
FunctionStaticStorage::~FunctionStaticStorage() {
	//don't delete the function, something else owns it
}
ThunkStaticStorage::ThunkStaticStorage(Thunk* pVal, BitSize pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("TNKSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
ThunkStaticStorage::~ThunkStaticStorage() {
	//don't delete the thunk, something else owns it
}
StaticAddress::StaticAddress(StaticStorage* pStorage, BitSize pointerBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("STCADR" COMMA) pointerBitSize)
, storage(pStorage) {
}
StaticAddress::~StaticAddress() {
	//don't delete the storage, something else owns it
}
TempStorage::TempStorage(BitSize pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("TMPSTRG" COMMA) pBitSize)
, finalStorage(nullptr) {
}
TempStorage::~TempStorage() {
	//registers will be the cached versions, memory pointers are owned here
	if (istype(finalStorage, MemoryPointer*))
		delete finalStorage;
}
Thunk::Thunk(string pName, unsigned short pThunkID)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("THUNK")) COMMA)
name(pName)
, thunkID(pThunkID) {
}
Thunk::~Thunk() {}
