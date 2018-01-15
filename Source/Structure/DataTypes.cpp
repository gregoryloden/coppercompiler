#include "Project.h"

//classes defined within other classes have owning class

PrefixTrie<char, CDataType*>* CDataType::globalDataTypes = nullptr;
Array<CDataType*>* CDataType::typesToDelete = nullptr;
CDataType::CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
name(pName) {
	typesToDelete->add(this);
}
CDataType::~CDataType() {}
//build the trie of data types here so that we can track deleting them properly
void CDataType::initializeGlobalDataTypes() {
	typesToDelete = new Array<CDataType*>();
	CDataType* typesArray[] = {
		new CVoid(),
		new CIntegerPrimitive("bool", 1),
		new CIntegerPrimitive("byte", 8),
		new CIntegerPrimitive("short", 16),
		new CIntegerPrimitive("int", 32),
		new CFloatingPointPrimitive("float", 32),
		new CGenericFunction(),
		new CClass("String")
	};
	const int typesArrayCount = sizeof(typesArray) / sizeof(typesArray[0]);
	globalDataTypes = new PrefixTrie<char, CDataType*>();
	for (int i = 0; i < typesArrayCount; i++) {
		CDataType* cdt = typesArray[i];
		globalDataTypes->set(cdt->name.c_str(), cdt->name.length(), cdt);
	}
}
//delete all the data types
void CDataType::deleteGlobalDataTypes() {
	typesToDelete->deleteContents();
	delete typesToDelete;
	delete globalDataTypes;
	globalDataTypes = nullptr;
}
CVoid::CVoid()
: CDataType(onlyWhenTrackingIDs("VOIDTYP" COMMA) "void") {
}
CVoid::~CVoid() {}
CPrimitive::CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, short pBitSize)
: CDataType(onlyWhenTrackingIDs(pObjType COMMA) pName)
, bitSize(pBitSize) {
}
CPrimitive::~CPrimitive() {}
CIntegerPrimitive::CIntegerPrimitive(string pName, int pBitSize)
: CPrimitive(onlyWhenTrackingIDs("INPMTYP" COMMA) pName, pBitSize) {
}
CIntegerPrimitive::~CIntegerPrimitive() {}
CFloatingPointPrimitive::CFloatingPointPrimitive(string pName, int pBitSize)
: CPrimitive(onlyWhenTrackingIDs("FLPMTYP" COMMA) pName, pBitSize) {
}
CFloatingPointPrimitive::~CFloatingPointPrimitive() {}
CGenericFunction::CGenericFunction()
: CDataType(onlyWhenTrackingIDs("GNFNTYP" COMMA) "Function") {
}
CGenericFunction::~CGenericFunction() {}
CSpecificFunction::CSpecificFunction(string pName, CDataType* pReturnType, Array<CDataType*>* pParameterTypes)
: CDataType(onlyWhenTrackingIDs("SPFNTYP" COMMA) pName)
, returnType(pReturnType)
, parameterTypes(pParameterTypes) {
}
CSpecificFunction::~CSpecificFunction() {
	//don't delete returnType or the parameter types since they're owned by the global types trie
	delete parameterTypes;
}
CClass::CClass(string pName)
: CDataType(onlyWhenTrackingIDs("CLSSTYP" COMMA) pName) {
}
CClass::~CClass() {}
