#include "Project.h"

AssemblyStorage::AssemblyStorage(onlyWhenTrackingIDs(char* pObjType COMMA) unsigned char pBitSize)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
bitSize(pBitSize) {
}
AssemblyStorage::~AssemblyStorage() {}
StaticStorage::StaticStorage(onlyWhenTrackingIDs(char* pObjType COMMA) unsigned char pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs(pObjType COMMA) pBitSize) {
}
StaticStorage::~StaticStorage() {}
AssemblyConstant::AssemblyConstant(int pVal, unsigned char pBitSize)
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
unsigned char Register::bitSizeForSpecificRegister(SpecificRegister specificRegister) {
	if (((unsigned char)specificRegister) <= ((unsigned char)SpecificRegister::Register32BitEnd))
		return 32;
	else if (((unsigned char)specificRegister) <= ((unsigned char)SpecificRegister::Register16BitEnd))
		return 16;
	else
		return 8;
}
//get an *A* register for the provided bit size
Register* Register::aRegisterForBitSize(unsigned char pBitSize) {
	return registerFor(
		pBitSize == 32 ? SpecificRegister::EAX :
		pBitSize == 16 ? SpecificRegister::AX :
		SpecificRegister::AL);
}
//get an *SP register for the provided bit size
Register* Register::spRegisterForBitSize(unsigned char pBitSize) {
	return registerFor(
		pBitSize == 32 ? SpecificRegister::ESP :
		SpecificRegister::SP);
}
//get a new non-cached undecided register for the provided bit size
Register* Register::newUndecidedRegisterForBitSize(unsigned char pBitSize) {
	return new Register(
		pBitSize == 32 ? SpecificRegister::Undecided32BitRegister :
		pBitSize == 16 ? SpecificRegister::Undecided16BitRegister :
		SpecificRegister::Undecided8BitRegister);
}
MemoryPointer::MemoryPointer(
	AssemblyStorage* pPrimaryRegister,
	unsigned char pPrimaryRegisterMultiplierPower,
	AssemblyStorage* pSecondaryRegister,
	int pConstant,
	unsigned char pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("MEMPTR" COMMA) pBitSize)
, primaryRegister(pPrimaryRegister)
, primaryRegisterMultiplierPower(pPrimaryRegisterMultiplierPower)
, secondaryRegister(pSecondaryRegister)
, constant(pConstant) {
}
MemoryPointer::~MemoryPointer() {}
OffsetMemoryPointer::OffsetMemoryPointer(AssemblyStorage* pStorage, int pOffset)
: AssemblyStorage(onlyWhenTrackingIDs("OFMMPTR" COMMA) pStorage->bitSize)
, storage(pStorage)
, offset(pOffset) {
}
OffsetMemoryPointer::~OffsetMemoryPointer() {
	//don't delete the storage, something else owns it
}
ValueStaticStorage::ValueStaticStorage(unsigned char pBitSize)
: StaticStorage(onlyWhenTrackingIDs("VALSSTG" COMMA) pBitSize) {
}
ValueStaticStorage::~ValueStaticStorage() {}
StringStaticStorage::StringStaticStorage(StringLiteral* pVal, unsigned char pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("STRSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
StringStaticStorage::~StringStaticStorage() {
	//don't delete the string, something else owns it
}
FunctionStaticStorage::FunctionStaticStorage(FunctionDefinition* pVal, unsigned char pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("FNCSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
FunctionStaticStorage::~FunctionStaticStorage() {
	//don't delete the function, something else owns it
}
ThunkStaticStorage::ThunkStaticStorage(Thunk* pVal, unsigned char pointerBitSize)
: StaticStorage(onlyWhenTrackingIDs("TNKSSTG" COMMA) pointerBitSize)
, val(pVal) {
}
ThunkStaticStorage::~ThunkStaticStorage() {
	//don't delete the thunk, something else owns it
}
StaticAddress::StaticAddress(StaticStorage* pStorage, unsigned char pointerBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("STCADR" COMMA) pointerBitSize)
, storage(pStorage) {
}
StaticAddress::~StaticAddress() {
	//don't delete the storage, something else owns it
}
TempStorage::TempStorage(unsigned char pBitSize)
: AssemblyStorage(onlyWhenTrackingIDs("TMPSTRG" COMMA) pBitSize)
, finalStorage(nullptr) {
}
TempStorage::~TempStorage() {
	delete finalStorage;
}
