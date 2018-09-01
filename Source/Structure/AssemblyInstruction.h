#include "../General/globals.h"

class FunctionDefinition;
class FunctionCall;
class AssemblyStorage;
class MemoryPointer;
class StaticStorage;
class Register;
enum class BitSize: unsigned char;
template <class Type> class Array;

#define basicInstructionDeclarationsWithBaseClassAndConstructorParameters(Type, BaseClass, parameters) \
	class Type: public BaseClass {\
	public:\
		Type(parameters);\
		virtual ~Type();\
		void removeDestinations(Array<AssemblyStorage*>* storages);\
		void addSources(Array<AssemblyStorage*>* storages);\
	};
#define basic0OperandInstructionDeclarations(Type) \
	basicInstructionDeclarationsWithBaseClassAndConstructorParameters(Type, AssemblyInstruction, )
#define basic1OperandInstructionDeclarations(Type) \
	basicInstructionDeclarationsWithBaseClassAndConstructorParameters(Type, AssemblyInstruction, AssemblyStorage* pDestination)
#define basic2OperandInstructionDeclarations(Type) \
	basicInstructionDeclarationsWithBaseClassAndConstructorParameters(\
		Type, AssemblyInstruction, AssemblyStorage* pDestination COMMA AssemblyStorage* pSource)
#define basicJumpDeclarations(Type) \
	basicInstructionDeclarationsWithBaseClassAndConstructorParameters(Type, JumpInstruction, AssemblyLabel* pJumpDestination)

class AssemblyInstruction onlyInDebug(: public ObjCounter) {
public:
	AssemblyStorage* destination; //copper: readonly
	AssemblyStorage* source; //copper: readonly
	int instructionArrayIndex; //copper: private<Build>

protected:
	AssemblyInstruction(onlyWhenTrackingIDs(char* pObjType COMMA) AssemblyStorage* pDestination, AssemblyStorage* pSource);
public:
	virtual ~AssemblyInstruction();

	virtual void removeDestinations(Array<AssemblyStorage*>* storages) = 0;
	virtual void addSources(Array<AssemblyStorage*>* storages) = 0;
	void removeDestination(AssemblyStorage* pDestination, Array<AssemblyStorage*>* storages);
	void addSource(AssemblyStorage* pSource, Array<AssemblyStorage*>* storages);
};
basic0OperandInstructionDeclarations(AssemblyLabel)
class ConditionLabelPair onlyInDebug(: public ObjCounter) {
public:
	AssemblyLabel* trueJumpDest; //copper: readonly
	AssemblyLabel* falseJumpDest; //copper: readonly

	ConditionLabelPair(AssemblyLabel* pTrueJumpDest, AssemblyLabel* pFalseJumpDest);
	virtual ~ConditionLabelPair();
};
class JumpInstruction: public AssemblyInstruction {
public:
	AssemblyLabel* jumpDestination; //copper: private<readonly Build>

protected:
	JumpInstruction(onlyWhenTrackingIDs(char* pObjType COMMA) AssemblyLabel* pJumpDestination);
public:
	virtual ~JumpInstruction();
};
//0 operand instructions
basic0OperandInstructionDeclarations(NOP)
basic0OperandInstructionDeclarations(CBW)
basic0OperandInstructionDeclarations(CWDE)
basic0OperandInstructionDeclarations(CWD)
basic0OperandInstructionDeclarations(CDQ)
basic0OperandInstructionDeclarations(CLD)
class REPMOVSB: public AssemblyInstruction {
private:
	BitSize cpuBitSize;

public:
	REPMOVSB(BitSize pCPUBitSize);
	virtual ~REPMOVSB();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
//0-1 operand instructions
class RET: public AssemblyInstruction {
private:
	FunctionDefinition* owningFunction;

public:
	RET(FunctionDefinition* pOwningFunction);
	virtual ~RET();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
//class LOOP: public AssemblyInstruction {
//1 operand instructions
basicJumpDeclarations(JMP)
basicJumpDeclarations(JE)
basicJumpDeclarations(JNE)
//	class JB: public AssemblyInstruction {
//	class JBE: public AssemblyInstruction {
//	class JA: public AssemblyInstruction {
//	class JAE: public AssemblyInstruction {
basicJumpDeclarations(JL)
basicJumpDeclarations(JLE)
basicJumpDeclarations(JG)
basicJumpDeclarations(JGE)
class CALL: public AssemblyInstruction {
public:
	CALL(MemoryPointer* target, Register* functionResult);
	CALL(StaticStorage* target, Register* functionResult);
	virtual ~CALL();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
basic1OperandInstructionDeclarations(INC)
basic1OperandInstructionDeclarations(DEC)
//class PUSH: public AssemblyInstruction {
//class POP: public AssemblyInstruction {
basic1OperandInstructionDeclarations(NOT)
basic1OperandInstructionDeclarations(NEG)
//	class DIV: public AssemblyInstruction {
class IDIV: public AssemblyInstruction {
public:
	IDIV(AssemblyStorage* divisor);
	virtual ~IDIV();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
//	class MUL: public AssemblyInstruction {
class IMUL: public AssemblyInstruction {
private:
	AssemblyConstant* multiplier;

public:
	IMUL(Register* pDestination, AssemblyStorage* pSource, AssemblyConstant* pMultiplier);
	virtual ~IMUL();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
basic1OperandInstructionDeclarations(SETE)
basic1OperandInstructionDeclarations(SETNE)
basic1OperandInstructionDeclarations(SETLE)
basic1OperandInstructionDeclarations(SETGE)
basic1OperandInstructionDeclarations(SETL)
basic1OperandInstructionDeclarations(SETG)
//2 operand instructions
basic2OperandInstructionDeclarations(ADD)
basic2OperandInstructionDeclarations(SUB)
basic2OperandInstructionDeclarations(MOV)
//	class ADC: public AssemblyInstruction {
//	class SBB: public AssemblyInstruction {
basic2OperandInstructionDeclarations(AND)
basic2OperandInstructionDeclarations(OR)
basic2OperandInstructionDeclarations(XOR)
class CMP: public AssemblyInstruction {
public:
	CMP(AssemblyStorage* leftSource, AssemblyStorage* rightSource);
	virtual ~CMP();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
class LEA: public AssemblyInstruction {
public:
	LEA(Register* pDestination, MemoryPointer* pSource);
	virtual ~LEA();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
//	class TEST: public AssemblyInstruction {
basic2OperandInstructionDeclarations(SHL)
basic2OperandInstructionDeclarations(SHR)
//class ROL: public AssemblyInstruction {
//class ROR: public AssemblyInstruction {
basic2OperandInstructionDeclarations(SAR)
//	class RCL: public AssemblyInstruction {
//	class RCR: public AssemblyInstruction {
class MOVSX: public AssemblyInstruction {
public:
	MOVSX(Register* pDestination, AssemblyStorage* pSource);
	virtual ~MOVSX();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};
//this will become a SUB but we need to track it separately
class StackShift: public AssemblyInstruction {
private:
	FunctionDefinition* owningFunction;
	FunctionDefinition* calledFunction;
	int functionCallArgumentBytes;
	bool calling;

public:
	StackShift(
		FunctionDefinition* pOwningFunction,
		FunctionDefinition* pCalledFunction,
		int pFunctionCallArgumentBytes,
		bool pCalling);
	~StackShift();

	void removeDestinations(Array<AssemblyStorage*>* storages);
	void addSources(Array<AssemblyStorage*>* storages);
};

































/*
#include "string"
using namespace std;

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7
#define AX 0
#define CX 1
#define DX 2
#define BX 3
#define SP 4
#define BP 5
#define SI 6
#define DI 7
#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7
#define TCONSTANT 1
#define TREG32 2
#define TREG16 3
#define TREG8 4
#define TDWORDPTR 5
#define TWORDPTR 6
#define TBYTEPTR 7
#define TINSTRUCTION 8
#define TINSTRUCTIONRELATIVE 9
#define TDATAADDRESS 10
#define TDATADWORDPTR 11
#define TDATAWORDPTR 12
#define TDATABYTEPTR 13
#define TCODEADDRESS 14
#define TAGNONE 0
#define TAGJMPINSTRUCTIONSHORT 1
#define TAGJMPINSTRUCTIONSHORTRELATIVE 2
#define TAGJMPINSTRUCTIONLONG 3
#define TAGCALLFUNCTION 4
#define TAGCALLTHUNK 5
#define TAGDATAADDRESS 6
#define TAGDATAPTRREG 7
#define TAGDATADWORDPTRCONSTANT 8
#define TAGDATAWORDPTRCONSTANT 9
#define TAGDATABYTEPTRCONSTANT 10
#define TAGREGDATAPTR 11
#define TAGDATADWORDPTR 12
#define TAGDATAWORDPTR 13
#define TAGDATABYTEPTR 14
#define TAGOPXDATAADDRESS 15
#define TAGOPXCODEADDRESS 16

class Thunk;
class Function;
template <class Type> class Array;
class AssemblyInstruction;
class ObjCounter;

int ptrfromreg(int reg);
int regfromptr(int ptr);
int dataptrfromptr(int ptr);
int ptrfromdataptr(int dataptr);
int tagfromdataptr(int dataptr);
int dataptrfromtag(int tag);
int tagconstantfromdataptr(int dataptr);
int dataptrfromtagconstant(int tagconstant);
bool isreg(int val);
bool isptr(int val);
bool isdataptr(int val);
bool istagdataptr(int val);
bool istagdataptrconstant(int val);
Array<AssemblyInstruction*>* addAssembly(Array<AssemblyInstruction*>* to, Array<AssemblyInstruction*>* from, bool deletable);

class MEMPTR
: public ObjCounter
 {
public:
	MEMPTR(char thereg1, char thepow, char thereg2, int theconstant, bool thedeletable);
	~MEMPTR();

	string toptr(char offset);
	string toptr(char offset, char reg3);

	char reg1;
	char pow;
	char reg2;
	int constant;
	bool deletable;
};
class AssemblyInstruction
: public ObjCounter
 {
public:
	AssemblyInstruction();
	AssemblyInstruction(AssemblyInstruction* a);
	~AssemblyInstruction();

	virtual AssemblyInstruction* clone() = 0;
	virtual void set(int val);
	virtual void set(int type, intptr_t val);
	virtual void set(int type1, intptr_t val1, int type2, intptr_t val2);
	void setregptr(int type, intptr_t val,
		string ocdword, string ocword, string ocbyte, int yy);
	void setreg(int rtype, intptr_t regn, int type2, intptr_t val2,
		string ocbyteconstant, string ocbigconstant, int yy, string ocspecialconstant,
		string ocvalat);
	void setptr(int ptype, intptr_t val1, int type2, intptr_t val2,
		string ocreg,
		string ocbyteconstant, string ocbigconstant, int yy);
	void setdataptr(int ptype, intptr_t val);
	void setdataptr(int ptype, intptr_t val1, int type2, intptr_t val2);
	void setjump(int type1, intptr_t val1,
		string ocsmall, string oclarge);
	void setshiftreg(int type1, int val1, int type2, int val2,
		string oc1constant, string ocbyteconstant, string occl, int yy);
	void setshiftptr(int type1, int val1, int type2, int val2,
		string oc1constant, string ocbyteconstant, string occl, int yy);
	void setIMUL(int rtype, int regn, int type2, intptr_t val2, int constant);

	string bytes;
	int tag;
	int tag2;
	int tag3;
	int tag4;
	intptr_t tagp;
};
//0-1 operand
class RET: public AssemblyInstruction {
public:
	RET();
	RET(int val);
	RET(AssemblyInstruction* a);
	~RET();

	RET* clone();
	void set(int val);
};
class REPMOVSB: public AssemblyInstruction {
public:
	REPMOVSB();
	~REPMOVSB();

	REPMOVSB* clone();
};
class LOOP: public AssemblyInstruction {
public:
	LOOP(int val);
	LOOP(AssemblyInstruction* a);
	~LOOP();

	LOOP* clone();
	void set(int val);
};
class NOP: public AssemblyInstruction {
public:
	NOP();
	~NOP();

	NOP* clone();
};
class CBW: public AssemblyInstruction {
public:
	CBW();
	~CBW();

	CBW* clone();
};
class CWD: public AssemblyInstruction {
public:
	CWD();
	~CWD();

	CWD* clone();
};
class CWDE: public AssemblyInstruction {
public:
	CWDE();
	~CWDE();

	CWDE* clone();
};
class CDQ: public AssemblyInstruction {
public:
	CDQ();
	~CDQ();

	CDQ* clone();
};
class CLD: public AssemblyInstruction {
public:
	CLD();
	~CLD();

	CLD* clone();
};
//1 operand
class INC: public AssemblyInstruction {
public:
	INC(int type, intptr_t val);
	INC(AssemblyInstruction* a);
	~INC();

	INC* clone();
	void set(int type, intptr_t val);
};
class DEC: public AssemblyInstruction {
public:
	DEC(int type, intptr_t val);
	DEC(AssemblyInstruction* a);
	~DEC();

	DEC* clone();
	void set(int type, intptr_t val);
};
class JMP: public AssemblyInstruction {
public:
	JMP(int type, intptr_t val);
	JMP(AssemblyInstruction* a);
	~JMP();

	JMP* clone();
	void set(int type, intptr_t val);
};
class CALL: public AssemblyInstruction {
public:
	CALL(int type, intptr_t val);
	CALL(Thunk* thunk);
	CALL(Function* function);
	CALL(AssemblyInstruction* a);
	~CALL();

	CALL* clone();
	void set(int type, intptr_t val);
	void set(Thunk* thunk);
	void set(Function* function);
};
class PUSH: public AssemblyInstruction {
public:
	PUSH(int type, intptr_t val);
	PUSH(AssemblyInstruction* a);
	~PUSH();

	PUSH* clone();
	void set(int type, intptr_t val);
};
class POP: public AssemblyInstruction {
public:
	POP(int type, intptr_t val);
	POP(AssemblyInstruction* a);
	~POP();

	POP* clone();
	void set(int type, intptr_t val);
};
class NOT: public AssemblyInstruction {
public:
	NOT(int type, intptr_t val);
	NOT(AssemblyInstruction* a);
	~NOT();

	NOT* clone();
	void set(int type, intptr_t val);
};
class NEG: public AssemblyInstruction {
public:
	NEG(int type, intptr_t val);
	NEG(AssemblyInstruction* a);
	~NEG();

	NEG* clone();
	void set(int type, intptr_t val);
};
/*
class DIV: public AssemblyInstruction {
public:
	DIV(int type, intptr_t val);
	DIV(AssemblyInstruction* a);
	~DIV();

	DIV* clone();
	void set(int type, intptr_t val);
};
*/
/*
class IDIV: public AssemblyInstruction {
public:
	IDIV(int type, intptr_t val);
	IDIV(AssemblyInstruction* a);
	~IDIV();

	IDIV* clone();
	void set(int type, intptr_t val);
};
/*
class MUL: public AssemblyInstruction {
public:
	MUL(int type, intptr_t val);
	MUL(AssemblyInstruction* a);
	~MUL();

	MUL* clone();
	void set(int type, intptr_t val);
};
*/
/*
class IMUL: public AssemblyInstruction {
public:
	IMUL(int type, intptr_t val);
	IMUL(int rtype, int regn, int type2, intptr_t val2, int constant);
	IMUL(AssemblyInstruction* a);
	~IMUL();

	IMUL* clone();
	void set(int type, intptr_t val);
};
class JE: public AssemblyInstruction {
public:
	JE(int type, intptr_t val);
	JE(AssemblyInstruction* a);
	~JE();

	JE* clone();
	void set(int type, intptr_t val);
};
class JNE: public AssemblyInstruction {
public:
	JNE(int type, intptr_t val);
	JNE(AssemblyInstruction* a);
	~JNE();

	JNE* clone();
	void set(int type, intptr_t val);
};
/*
class JB: public AssemblyInstruction {
public:
	JB(int type, intptr_t val);
	JB(AssemblyInstruction* a);
	~JB();

	JB* clone();
	void set(int type, intptr_t val);
};
class JBE: public AssemblyInstruction {
public:
	JBE(int type, intptr_t val);
	JBE(AssemblyInstruction* a);
	~JBE();

	JBE* clone();
	void set(int type, intptr_t val);
};
class JA: public AssemblyInstruction {
public:
	JA(int type, intptr_t val);
	JA(AssemblyInstruction* a);
	~JA();

	JA* clone();
	void set(int type, intptr_t val);
};
class JAE: public AssemblyInstruction {
public:
	JAE(int type, intptr_t val);
	JAE(AssemblyInstruction* a);
	~JAE();

	JAE* clone();
	void set(int type, intptr_t val);
};
*/
/*
class JL: public AssemblyInstruction {
public:
	JL(int type, intptr_t val);
	JL(AssemblyInstruction* a);
	~JL();

	JL* clone();
	void set(int type, intptr_t val);
};
class JLE: public AssemblyInstruction {
public:
	JLE(int type, intptr_t val);
	JLE(AssemblyInstruction* a);
	~JLE();

	JLE* clone();
	void set(int type, intptr_t val);
};
class JG: public AssemblyInstruction {
public:
	JG(int type, intptr_t val);
	JG(AssemblyInstruction* a);
	~JG();

	JG* clone();
	void set(int type, intptr_t val);
};
class JGE: public AssemblyInstruction {
public:
	JGE(int type, intptr_t val);
	JGE(AssemblyInstruction* a);
	~JGE();

	JGE* clone();
	void set(int type, intptr_t val);
};
class SETE: public AssemblyInstruction {
public:
	SETE(int type, intptr_t val);
	SETE(AssemblyInstruction* a);
	~SETE();

	SETE* clone();
	void set(int type, intptr_t val);
};
class SETNE: public AssemblyInstruction {
public:
	SETNE(int type, intptr_t val);
	SETNE(AssemblyInstruction* a);
	~SETNE();

	SETNE* clone();
	void set(int type, intptr_t val);
};
class SETLE: public AssemblyInstruction {
public:
	SETLE(int type, intptr_t val);
	SETLE(AssemblyInstruction* a);
	~SETLE();

	SETLE* clone();
	void set(int type, intptr_t val);
};
class SETGE: public AssemblyInstruction {
public:
	SETGE(int type, intptr_t val);
	SETGE(AssemblyInstruction* a);
	~SETGE();

	SETGE* clone();
	void set(int type, intptr_t val);
};
class SETL: public AssemblyInstruction {
public:
	SETL(int type, intptr_t val);
	SETL(AssemblyInstruction* a);
	~SETL();

	SETL* clone();
	void set(int type, intptr_t val);
};
class SETG: public AssemblyInstruction {
public:
	SETG(int type, intptr_t val);
	SETG(AssemblyInstruction* a);
	~SETG();

	SETG* clone();
	void set(int type, intptr_t val);
};
//2 operand
class ADD: public AssemblyInstruction {
public:
	ADD(int type1, intptr_t val1, int type2, intptr_t val2);
	ADD(AssemblyInstruction* a);
	~ADD();

	ADD* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class SUB: public AssemblyInstruction {
public:
	SUB(int type1, intptr_t val1, int type2, intptr_t val2);
	SUB(AssemblyInstruction* a);
	~SUB();

	SUB* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class MOV: public AssemblyInstruction {
public:
	MOV(int type1, intptr_t val1, int type2, intptr_t val2);
	MOV(AssemblyInstruction* a);
	~MOV();

	MOV* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
/*
class ADC: public AssemblyInstruction {
public:
	ADC(int type1, intptr_t val1, int type2, intptr_t val2);
	ADC(AssemblyInstruction* a);
	~ADC();

	ADC* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class SBB: public AssemblyInstruction {
public:
	SBB(int type1, intptr_t val1, int type2, intptr_t val2);
	SBB(AssemblyInstruction* a);
	~SBB();

	SBB* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
*/
/*
class AND: public AssemblyInstruction {
public:
	AND(int type1, intptr_t val1, int type2, intptr_t val2);
	AND(AssemblyInstruction* a);
	~AND();

	AND* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class OR: public AssemblyInstruction {
public:
	OR(int type1, intptr_t val1, int type2, intptr_t val2);
	OR(AssemblyInstruction* a);
	~OR();

	OR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class XOR: public AssemblyInstruction {
public:
	XOR(int type1, intptr_t val1, int type2, intptr_t val2);
	XOR(AssemblyInstruction* a);
	~XOR();

	XOR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class CMP: public AssemblyInstruction {
public:
	CMP(int type1, intptr_t val1, int type2, intptr_t val2);
	CMP(AssemblyInstruction* a);
	~CMP();

	CMP* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class LEA: public AssemblyInstruction {
public:
	LEA(int type1, intptr_t val1, int type2, intptr_t val2);
	LEA(AssemblyInstruction* a);
	~LEA();

	LEA* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
/*
class TEST: public AssemblyInstruction {
public:
	TEST(int type1, intptr_t val1, int type2, intptr_t val2);
	TEST(AssemblyInstruction* a);
	~TEST();

	TEST* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
*/
/*
class SHL: public AssemblyInstruction {
public:
	SHL(int type1, intptr_t val1, int type2, intptr_t val2);
	SHL(AssemblyInstruction* a);
	~SHL();

	SHL* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class SHR: public AssemblyInstruction {
public:
	SHR(int type1, intptr_t val1, int type2, intptr_t val2);
	SHR(AssemblyInstruction* a);
	~SHR();

	SHR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class ROL: public AssemblyInstruction {
public:
	ROL(int type1, intptr_t val1, int type2, intptr_t val2);
	ROL(AssemblyInstruction* a);
	~ROL();

	ROL* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class ROR: public AssemblyInstruction {
public:
	ROR(int type1, intptr_t val1, int type2, intptr_t val2);
	ROR(AssemblyInstruction* a);
	~ROR();

	ROR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class SAR: public AssemblyInstruction {
public:
	SAR(int type1, intptr_t val1, int type2, intptr_t val2);
	SAR(AssemblyInstruction* a);
	~SAR();

	SAR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
/*
class RCL: public AssemblyInstruction {
public:
	RCL(int type1, intptr_t val1, int type2, intptr_t val2);
	RCL(AssemblyInstruction* a);
	~RCL();

	RCL* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
class RCR: public AssemblyInstruction {
public:
	RCR(int type1, intptr_t val1, int type2, intptr_t val2);
	RCR(AssemblyInstruction* a);
	~RCR();

	RCR* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
*/
/*
class MOVSX: public AssemblyInstruction {
public:
	MOVSX(int type1, intptr_t val1, int type2, intptr_t val2);
	MOVSX(AssemblyInstruction* a);
	~MOVSX();

	MOVSX* clone();
	void set(int type1, intptr_t val1, int type2, intptr_t val2);
};
*/
