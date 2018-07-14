#include "Project.h"

//classes defined within other classes have owning class

PrefixTrie<char, CDataType*>* CDataType::globalDataTypes = nullptr;
CErrorType* CDataType::errorType = nullptr;
CVoid* CDataType::voidType = nullptr;
CBool* CDataType::boolType = nullptr;
CIntegerPrimitive* CDataType::infiniteByteSizeIntType = nullptr;
CIntegerPrimitive* CDataType::byteType = nullptr;
CIntegerPrimitive* CDataType::shortType = nullptr;
CIntegerPrimitive* CDataType::intType = nullptr;
CFloatingPointPrimitive* CDataType::infinitePrecisionFloatType = nullptr;
CFloatingPointPrimitive* CDataType::floatType = nullptr;
CGenericFunction* CDataType::functionType = nullptr;
CDataType* CDataType::emptyGroupType = nullptr;
CClass* CDataType::stringType = nullptr;
CClass* CDataType::mainType = nullptr;
CDataType::CDataType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs(pObjType)) COMMA)
name(pName) {
}
CDataType::~CDataType() {}
//build the trie of data types here so that we can track deleting them properly
void CDataType::initializeGlobalDataTypes() {
	CDataType* typesArray[] = {
		(errorType = new CErrorType()),
		(voidType = new CVoid()),
		(boolType = new CBool()),
		(infiniteByteSizeIntType = new CIntegerPrimitive("(int)", BitSize::BInfinite)),
		(byteType = new CIntegerPrimitive("byte", BitSize::B8)),
		(shortType = new CIntegerPrimitive("short", BitSize::B16)),
		(intType = new CIntegerPrimitive("int", BitSize::B32)),
		(infinitePrecisionFloatType = new CFloatingPointPrimitive("(float)", BitSize::BInfinite)),
		(floatType = new CFloatingPointPrimitive("float", BitSize::B32)),
		(functionType = new CGenericFunction()),
		(stringType = new CClass("String")),
		(mainType = new CClass("Main"))
	};
	const int typesArrayCount = sizeof(typesArray) / sizeof(typesArray[0]);
	globalDataTypes = new PrefixTrie<char, CDataType*>();
	for (int i = 0; i < typesArrayCount; i++) {
		CDataType* cdt = typesArray[i];
		globalDataTypes->set(cdt->name.c_str(), cdt->name.length(), cdt);
	}
	//since Group() is the same type as void, use the void type as the value for that key
	emptyGroupType = CGenericGroup::typeFor(new Array<CVariableDefinition*>());
	globalDataTypes->set(emptyGroupType->name.c_str(), emptyGroupType->name.length(), voidType);
}
//delete all the data types
void CDataType::deleteGlobalDataTypes() {
	errorType = nullptr;
	voidType = nullptr;
	boolType = nullptr;
	infiniteByteSizeIntType = nullptr;
	byteType = nullptr;
	shortType = nullptr;
	intType = nullptr;
	infinitePrecisionFloatType = nullptr;
	floatType = nullptr;
	functionType = nullptr;
	globalDataTypes->set(emptyGroupType->name.c_str(), emptyGroupType->name.length(), emptyGroupType);
	emptyGroupType = nullptr;
	stringType = nullptr;
	mainType = nullptr;
	globalDataTypes->deleteValues();
	delete globalDataTypes;
	globalDataTypes = nullptr;
}
//find the most specific type that both types can be implicitly casted to
//for two numeric types: return the bigger of the two types, the type that can represent the values of both types
//for two classes: return the most-specific superclass that both of them inherit (for multiple inheritance,
//	if there are two or more of those classes, recursively find the best class for them and use that)
//since raw casting doesn't apply here, any other type combinations are invalid
CDataType* CDataType::bestCompatibleType(CDataType* type1, CDataType* type2) {
	if (type1 == type2)
		return type1;
	CNumericPrimitive* npType1;
	CNumericPrimitive* npType2;
	if (let(CNumericPrimitive*, npType1, type1) && let(CNumericPrimitive*, npType2, type2)) {
		bool firstIsFloat;
		if ((firstIsFloat = (istype(npType1, CFloatingPointPrimitive*))) != (istype(npType2, CFloatingPointPrimitive*)))
			return firstIsFloat ? npType1 : npType2;
		else if (npType1 == infiniteByteSizeIntType || npType1 == infinitePrecisionFloatType)
			return npType2;
		else if (npType2 == infiniteByteSizeIntType || npType2 == infinitePrecisionFloatType)
			return npType1;
		return npType1->bitSize > npType2->bitSize ? npType1 : npType2;
	} else if ((type1 == CDataType::functionType && istype(type2, CSpecificFunction*))
			|| (type2 == CDataType::functionType && istype(type1, CSpecificFunction*)))
		return CDataType::functionType;
	//TODO: classes
	return nullptr;
}
CPrimitive::CPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, BitSize pBitSize)
: CDataType(onlyWhenTrackingIDs(pObjType COMMA) pName)
, bitSize(pBitSize) {
}
CPrimitive::~CPrimitive() {}
CNumericPrimitive::CNumericPrimitive(onlyWhenTrackingIDs(char* pObjType COMMA) string pName, BitSize pBitSize)
: CPrimitive(onlyWhenTrackingIDs(pObjType COMMA) pName, pBitSize) {
}
CNumericPrimitive::~CNumericPrimitive() {}
CGenericPointerType::CGenericPointerType(onlyWhenTrackingIDs(char* pObjType COMMA) string pName)
: CDataType(onlyWhenTrackingIDs(pObjType COMMA) pName) {
}
CGenericPointerType::~CGenericPointerType() {}
CErrorType::CErrorType()
: CDataType(onlyWhenTrackingIDs("ERORTYP" COMMA) "(error type)") {
}
CErrorType::~CErrorType() {}
CVoid::CVoid()
: CDataType(onlyWhenTrackingIDs("VOIDTYP" COMMA) "void") {
}
CVoid::~CVoid() {}
CBool::CBool()
: CPrimitive(onlyWhenTrackingIDs("BOOLTYP" COMMA) "bool", BitSize::B1) {
}
CBool::~CBool() {}
CIntegerPrimitive::CIntegerPrimitive(string pName, BitSize pBitSize)
: CNumericPrimitive(onlyWhenTrackingIDs("INPMTYP" COMMA) pName, pBitSize) {
}
CIntegerPrimitive::~CIntegerPrimitive() {}
CFloatingPointPrimitive::CFloatingPointPrimitive(string pName, BitSize pBitSize)
: CNumericPrimitive(onlyWhenTrackingIDs("FLPMTYP" COMMA) pName, pBitSize) {
}
CFloatingPointPrimitive::~CFloatingPointPrimitive() {}
CGenericFunction::CGenericFunction()
: CGenericPointerType(onlyWhenTrackingIDs("GNFNTYP" COMMA) "Function") {
}
CGenericFunction::~CGenericFunction() {}
//return the function type that matches the given return type and parameter types, creating it if it doesn't exist yet
//deletes the parameterTypes array if it isn't used
CDataType* CGenericFunction::typeFor(CDataType* returnType, Array<CDataType*>* parameterTypes) {
	//build the type name
	string typeName = "Function<";
	typeName += returnType->name;
	typeName += '(';
	bool addComma = false;
	forEach(CDataType*, c, parameterTypes, ci) {
		if (addComma)
			typeName += ',';
		else
			addComma = true;
		typeName += c->name;
	}
	typeName += ")>";

	//create the type if we don't already have it
	CDataType* cdt = CDataType::globalDataTypes->get(typeName.c_str(), typeName.length());
	if (cdt == nullptr) {
		cdt = new CSpecificFunction(typeName, returnType, parameterTypes);
		CDataType::globalDataTypes->set(typeName.c_str(), typeName.length(), cdt);
	} else
		delete parameterTypes;
	return cdt;
}
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
CGenericGroup::CGenericGroup()
: CGenericPointerType(onlyWhenTrackingIDs("GNGPTYP" COMMA) "Group") {
}
CGenericGroup::~CGenericGroup() {}
//return the group type that matches the given types, creating it if it doesn't exist yet
//deletes the types array and its contents if it isn't used
CDataType* CGenericGroup::typeFor(Array<CVariableDefinition*>* types) {
	//build the type name
	string typeName = "Group<";
	bool addComma = false;
	forEach(CVariableDefinition*, c, types, ci) {
		if (addComma)
			typeName += ',';
		else
			addComma = true;
		typeName += c->type->name;
		if (c->name != nullptr) {
			typeName += ' ';
			typeName += c->name->name;
		}
	}
	typeName += ">";

	//create the type if we don't already have it
	CDataType* cdt = CDataType::globalDataTypes->get(typeName.c_str(), typeName.length());
	if (cdt == nullptr) {
		cdt = new CSpecificGroup(typeName, types);
		CDataType::globalDataTypes->set(typeName.c_str(), typeName.length(), cdt);
	} else {
		types->deleteContents();
		delete types;
	}
	return cdt;
}
CSpecificGroup::CSpecificGroup(string pName, Array<CVariableDefinition*>* pTypes)
: CDataType(onlyWhenTrackingIDs("SPGPTYP" COMMA) pName)
, types(pTypes)
, allSameType(false) {
	if (pTypes->length == 0)
		return;
	CDataType* t = pTypes->first()->type;
	for (int i = 1; i < pTypes->length; i++) {
		if (pTypes->get(i)->type != t)
			return;
	}
	allSameType = true;
}
CSpecificGroup::~CSpecificGroup() {
	types->deleteContents();
	delete types;
}
