#include "Project.h"

/*
MEMPTR mblankdataptr (-1, -1, -1, IMAGEBASE, false);
intptr_t blankdataptr = (intptr_t)(&mblankdataptr);

//turn a register into a pointer
int ptrfromreg(int reg) {
	if (reg == TREG32)
		return TDWORDPTR;
	else if (reg == TREG16)
		return TWORDPTR;
	return TBYTEPTR;
}
//turn a pointer into a register
int regfromptr(int ptr) {
	if (ptr == TDWORDPTR)
		return TREG32;
	else if (ptr == TWORDPTR)
		return TREG16;
	return TREG8;
}
//turn a pointer into a data pointer
int dataptrfromptr(int ptr) {
	if (ptr == TDWORDPTR)
		return TDATADWORDPTR;
	else if (ptr == TWORDPTR)
		return TDATAWORDPTR;
	return TDATABYTEPTR;
}
//turn a data pointer into a pointer
int ptrfromdataptr(int dataptr) {
	if (dataptr == TDATADWORDPTR)
		return TDWORDPTR;
	else if (dataptr == TDATAWORDPTR)
		return TWORDPTR;
	return TBYTEPTR;
}
//turn a data pointer into a tag data pointer
int tagfromdataptr(int dataptr) {
	if (dataptr == TDATADWORDPTR)
		return TAGDATADWORDPTR;
	else if (dataptr == TDATAWORDPTR)
		return TAGDATAWORDPTR;
	return TAGDATABYTEPTR;
}
//turn a tag data pointer into a data pointer
int dataptrfromtag(int tag) {
	if (tag == TAGDATADWORDPTR)
		return TDATADWORDPTR;
	else if (tag == TAGDATAWORDPTR)
		return TDATAWORDPTR;
	return TDATABYTEPTR;
}
//turn a data pointer into a tag data pointer constant
int tagconstantfromdataptr(int dataptr) {
	if (dataptr == TDATADWORDPTR)
		return TAGDATADWORDPTRCONSTANT;
	else if (dataptr == TDATAWORDPTR)
		return TAGDATAWORDPTRCONSTANT;
	return TAGDATABYTEPTRCONSTANT;
}
//turn a tag data pointer constant into a data pointer
int dataptrfromtagconstant(int tagconstant) {
	if (tagconstant == TAGDATADWORDPTRCONSTANT)
		return TDATADWORDPTR;
	else if (tagconstant == TAGDATAWORDPTRCONSTANT)
		return TDATAWORDPTR;
	return TDATABYTEPTR;
}
//check if the val is a register
bool isreg(int val) {
	return val == TREG32 || val == TREG16 || val == TREG8;
}
//check if the val is a pointer
bool isptr(int val) {
	return val == TDWORDPTR || val == TWORDPTR || val == TBYTEPTR;
}
//check if the val is a data pointer
bool isdataptr(int val) {
	return val == TDATADWORDPTR || val == TDATAWORDPTR || val == TDATABYTEPTR;
}
//check if the val is a tag data pointer
bool istagdataptr(int val) {
	return val == TAGDATADWORDPTR || val == TAGDATAWORDPTR || val == TAGDATABYTEPTR;
}
//check if the val is a tag data pointer constant
bool istagdataptrconstant(int val) {
	return val == TAGDATADWORDPTRCONSTANT || val == TAGDATAWORDPTRCONSTANT || val == TAGDATABYTEPTRCONSTANT;
}
//add the assembly instructions from one array to another
//increase any jump destinations to account for the new array location
Array<AssemblyInstruction*>* addAssembly(Array<AssemblyInstruction*>* to, Array<AssemblyInstruction*>* from, bool deletable) {
	ArrayIterator<AssemblyInstruction*> ai (from);
	int length = to->length;
	for (AssemblyInstruction* a = ai.getFirst(); ai.hasThis(); a = ai.getNext()) {
		if (a->tag == TAGJMPINSTRUCTIONSHORT)
			a->tag2 += length;
	}
	return to->add(from, deletable);
}
MEMPTR::MEMPTR(char thereg1, char thepow, char thereg2, int theconstant, bool thedeletable):
ObjCounter("MPTR"),
	reg1(thereg1),
	pow(thepow),
	reg2(thereg2),
	constant(theconstant),
	deletable(thedeletable) {
}
MEMPTR::~MEMPTR() {}
//get the bytes for a memory pointer
string MEMPTR::toptr(char offset) {
	return toptr(offset, 0);
}
//get the bytes for a memory pointer
string MEMPTR::toptr(char offset, char reg3) {
	string s;
	if (reg1 == -1)
		s.assign(1, offset + 5 + reg3 * 8).append(to4bytes(constant));
	else if (reg2 == -1) {
		if (pow > 0)
			s.assign(1, offset + 4 + reg3 * 8).append(1, 5 + 0x40 * pow + reg1 * 8).append(to4bytes(constant));
		else if (constant == 0 && reg1 != EBP) {
			if (reg1 == ESP)
				s.assign(1, offset + 4 + reg3 * 8).append(1, 0x24);
			else
				s.assign(1, offset + reg1 + reg3 * 8);
		} else if (constant >= -128 && constant <= 127) {
			if (reg1 == ESP)
				s.assign(1, offset + 0x44 + reg3 * 8).append(1, 0x24).append(1, (char)(constant));
			else
				s.assign(1, offset + 0x40 + reg1 + reg3 * 8).append(1, (char)(constant));
		} else {
			if (reg1 == ESP)
				s.assign(1, offset + 0x84 + reg3 * 8).append(1, 0x24).append(to4bytes(constant));
			else
				s.assign(1, offset + 0x80 + reg1 + reg3 * 8).append(to4bytes(constant));
		}
	} else {
		if (constant == 0 && reg2 != EBP)
			s.assign(1, offset + 4 + reg3 * 8).append(1, 0x40 * pow + reg1 * 8 + reg2);
		else if (constant >= -128 && constant <= 127)
			s.assign(1, offset + 0x44 + reg3 * 8).append(1, 0x40 * pow + reg1 * 8 + reg2).append(1, (char)(constant));
		else
			s.assign(1, offset + 0x84 + reg3 * 8).append(1, 0x40 * pow + reg1 * 8 + reg2).append(to4bytes(constant));
	}
	if (deletable)
		delete this;
	return s;
}
AssemblyInstruction::AssemblyInstruction():
ObjCounter("ASIN"),
	tag(TAGNONE) {
}
AssemblyInstruction::AssemblyInstruction(AssemblyInstruction* a):
ObjCounter("ASIN"),
	bytes(a->bytes),
	tag(a->tag),
	tag2(a->tag2),
	tag3(a->tag3),
	tag4(a->tag4),
	tagp(a->tagp) {
}
AssemblyInstruction::~AssemblyInstruction() {}
//set a 0-1 operand instruction
void AssemblyInstruction::set(int val) {}
//set a 1 operand instruction
void AssemblyInstruction::set(int type, intptr_t val) {}
//set a 2 operand instruction
void AssemblyInstruction::set(int type1, intptr_t val1, int type2, intptr_t val2) {}
//set a 1 operand instruction using the given bytes
void AssemblyInstruction::setregptr(int type, intptr_t val,
	string ocdword, string ocword, string ocbyte, int yy) {
	if (type == TREG32)
		bytes.assign(ocdword).append(1, (char)(0xC0 + yy + val));
	else if (type == TREG16)
		bytes.assign(ocword).append(1, (char)(0xC0 + yy + val));
	else if (type == TREG8)
		bytes.assign(ocbyte).append(1, (char)(0xC0 + yy + val));
	else if (type == TDWORDPTR)
		bytes.assign(ocdword).append(((MEMPTR*)(val))->toptr((char)(yy)));
	else if (type == TWORDPTR)
		bytes.assign(ocword).append(((MEMPTR*)(val))->toptr((char)(yy)));
	else if (type == TBYTEPTR)
		bytes.assign(ocbyte).append(((MEMPTR*)(val))->toptr((char)(yy)));
}
//set a 2 operand reg/??? instruction using the given bytes
void AssemblyInstruction::setreg(int rtype, intptr_t regn, int type2, intptr_t val2,
	string ocbyteconstant, string ocbigconstant, int yy, string ocspecialconstant,
	string ocvalat) {
	//reg/constant
	if (type2 == TCONSTANT) {
		//EAX = AX = AL, all share the special
		if (regn == EAX && ocspecialconstant.length() > 0) {
			if (rtype == TREG8)
				bytes.assign(ocspecialconstant).append(1, (char)(val2));
			else
				bytes.assign(ocspecialconstant).append(rtype == TREG16 ? to2bytes((int)(val2)) : to4bytes((int)(val2)));
		//byte constant with any reg
		} else if (rtype == TREG8 || (val2 >= -128 && val2 <= 127 && ocbyteconstant.compare("") != 0))
			bytes.assign(ocbyteconstant).append(1, (char)(yy + regn)).append(1, (char)(val2));
		//word or dword constant
		else
			bytes.assign(ocbigconstant).append(1, (char)(yy + regn)).append(rtype == TREG16 ? to2bytes((int)(val2)) : to4bytes((int)(val2)));
	//reg/reg
	} else if (isreg(type2))
		bytes.assign(ocvalat).append(1, (char)(0xC0 + regn * 8 + val2));
	//reg/ptr
	else if (isptr(type2))
		bytes.assign(ocvalat).append(((MEMPTR*)(val2))->toptr(0, (char)(regn)));
	//reg/data ptr
	else if (isdataptr(type2)) {
		set(rtype, regn, ptrfromreg(rtype), blankdataptr);
		tag = TAGREGDATAPTR;
		tag2 = val2;
		tag3 = rtype;
		tag4 = regn;
	}
}
//set a 2 operand memory pointer/??? instruction using the given bytes
void AssemblyInstruction::setptr(int ptype, intptr_t val1, int type2, intptr_t val2,
	string ocreg,
	string ocbyteconstant, string ocbigconstant, int yy) {
	//ptr/reg
	if (isreg(type2))
		bytes.assign(ocreg).append(((MEMPTR*)(val1))->toptr(0, (char)(val2)));
	//ptr/constant
	else if (type2 == TCONSTANT) {
		//byte constant
		if (ptype == TBYTEPTR || (val2 >= -128 && val2 <= 127 && ocbyteconstant.compare("") != 0))
			bytes.assign(ocbyteconstant).append(((MEMPTR*)(val1))->toptr((char)(yy))).append(1, (char)(val2));
		//word or dword constant
		else if (ptype == TWORDPTR)
			bytes.assign(ocbigconstant).append(((MEMPTR*)(val1))->toptr((char)(yy))).append(to2bytes((int)(val2)));
		else
			bytes.assign(ocbigconstant).append(((MEMPTR*)(val1))->toptr((char)(yy))).append(to4bytes((int)(val2)));
	}
}
//set a 1 operand data pointer instruction
void AssemblyInstruction::setdataptr(int type, int val) {
	set(ptrfromdataptr(type), blankdataptr);
	tag = tagfromdataptr(type);
	tag2 = val;
}
//set a 2 operand data pointer instruction
void AssemblyInstruction::setdataptr(int type1, int val1, int type2, int val2) {
	if (isreg(type2)) {
		set(ptrfromreg(type2), blankdataptr, type2, val2);
		tag = TAGDATAPTRREG;
		tag2 = val1;
		tag3 = type2;
		tag4 = val2;
	} else if (type2 == TCONSTANT) {
		set(ptrfromdataptr(type1), blankdataptr, type2, val2);
		tag = tagconstantfromdataptr(type1);
		tag2 = val1;
		tag3 = type2;
		tag4 = val2;
	}
}
//set a jump instruction using the given bytes
void AssemblyInstruction::setjump(int type, intptr_t val,
	string ocsmall, string oclarge) {
	if (type == TCONSTANT) {
		if (val >= -128 && val <= 127)
			bytes.assign(ocsmall).append(1, (char)(val));
		else
			bytes.assign(oclarge).append(to4bytes(val));
	} else if (type == TINSTRUCTION) {
		setjump(TCONSTANT, 0, ocsmall, oclarge);
		tag = TAGJMPINSTRUCTIONSHORT;
		tag2 = val;
	} else if (type == TINSTRUCTIONRELATIVE) {
		setjump(TCONSTANT, 0, ocsmall, oclarge);
		tag = TAGJMPINSTRUCTIONSHORTRELATIVE;
		tag2 = val;
	}
}
//set a shift reg/??? instruction using the given bytes
void AssemblyInstruction::setshiftreg(int type1, int val1, int type2, int val2,
	string oc1constant, string ocbyteconstant, string occl, int yy) {
	if (type2 == TCONSTANT) {
		if (val2 == 1)
			bytes.assign(oc1constant).append(1, (char)(yy + val1));
		else
			bytes.assign(ocbyteconstant).append(1, (char)(yy + val1)).append(1, (char)(val2));
	} else
		bytes.assign(occl).append(1, (char)(yy + val1));
}
//set a shift memory pointer/??? instruction using the given bytes
void AssemblyInstruction::setshiftptr(int type1, int val1, int type2, int val2,
	string oc1constant, string ocbyteconstant, string occl, int yy) {
	if (type2 == TCONSTANT) {
		if (val2 == 1)
			bytes.assign(oc1constant).append(((MEMPTR*)(val1))->toptr((char)(yy)));
		else
			bytes.assign(ocbyteconstant).append(((MEMPTR*)(val1))->toptr((char)(yy))).append(1, (char)(val2));
	} else
		bytes.assign(occl).append(((MEMPTR*)(val1))->toptr((char)(yy)));
}
//set an IMUL instruction using the given bytes
void AssemblyInstruction::setIMUL(int rtype, int regn, int type2, intptr_t val2, int constant) {
	bytes.assign(rtype == TREG16 ? "\x66" : "");
	if (constant == 0)
		bytes.append("\x0F\xAF");
	else if (constant >= -128 && constant <= 127)
		bytes.append("\x6B");
	else
		bytes.append("\x69");
	if (type2 == rtype)
		bytes.append(1, (char)(0xC0 + regn * 8 + val2));
	else
		bytes.append(((MEMPTR*)(val2))->toptr(0));
	if (constant != 0) {
		if (constant >= -128 && constant <= 127)
			bytes.append(1, (char)(constant));
		else
			bytes.append(to4bytes(constant));
	}
}
RET::RET() {set(0);}
RET::RET(int val) {set(val);}
RET::RET(AssemblyInstruction* a): AssemblyInstruction(a) {}
RET::~RET() {}
//clone this RET
RET* RET::clone() {return new RET(this);}
//set the RET
void RET::set(int val) {
	if (val == 0)
		bytes.assign("\xC3");
	else
		bytes.assign("\xC2").append(to2bytes(val));
}
REPMOVSB::REPMOVSB() {bytes.assign("\xF3\xA4");}
REPMOVSB::~REPMOVSB() {}
//clone this REPMOVSB
REPMOVSB* REPMOVSB::clone() {return new REPMOVSB();}
LOOP::LOOP(int val) {set(val);}
LOOP::LOOP(AssemblyInstruction* a): AssemblyInstruction(a) {}
LOOP::~LOOP() {}
//clone this LOOP
LOOP* LOOP::clone() {return new LOOP(this);}
//set the LOOP
void LOOP::set(int val) {
	bytes.assign("\xE2").append(1, (char)(val));
}
NOP::NOP() {bytes.assign("\x90");}
NOP::~NOP() {}
//clone this NOP
NOP* NOP::clone() {return new NOP();}
CBW::CBW() {bytes.assign("\x66\x98");}
CBW::~CBW() {}
//clone this CBW
CBW* CBW::clone() {return new CBW();}
CWD::CWD() {bytes.assign("\x66\x99");}
CWD::~CWD() {}
//clone this CWD
CWD* CWD::clone() {return new CWD();}
CWDE::CWDE() {bytes.assign("\x98");}
CWDE::~CWDE() {}
//clone this CWDE
CWDE* CWDE::clone() {return new CWDE();}
CDQ::CDQ() {bytes.assign("\x99");}
CDQ::~CDQ() {}
//clone this CDQ
CDQ* CDQ::clone() {return new CDQ();}
CLD::CLD() {bytes.assign("\xFC");}
CLD::~CLD() {}
//clone this CLD
CLD* CLD::clone() {return new CLD();}
INC::INC(int type, intptr_t val) {set(type, val);}
INC::INC(AssemblyInstruction* a): AssemblyInstruction(a) {}
INC::~INC() {}
//clone this INC
INC* INC::clone() {return new INC(this);}
//set the INC
void INC::set(int type, intptr_t val) {
	if (type == TREG32)
		bytes.assign(1, (char)(0x40 + val));
	else if (type == TREG16)
		bytes.assign("\x66").append(1, (char)(0x40 + val));
	else if (type == TREG8 || isptr(type))
		setregptr(type, val, "\xFF", "\x66\xFF", "\xFE", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
DEC::DEC(int type, intptr_t val) {set(type, val);}
DEC::DEC(AssemblyInstruction* a): AssemblyInstruction(a) {}
DEC::~DEC() {}
//clone this DEC
DEC* DEC::clone() {return new DEC(this);}
//set the DEC
void DEC::set(int type, intptr_t val) {
	if (type == TREG32)
		bytes.assign(1, (char)(0x48 + val));
	else if (type == TREG16)
		bytes.assign("\x66").append(1, (char)(0x48 + val));
	else if (type == TREG8 || isptr(type))
		setregptr(type, val, "\xFF", "\x66\xFF", "\xFE", 8);
	else if (isdataptr(type))
		setdataptr(type, val);
}
JMP::JMP(int type, intptr_t val) {set(type, val);}
JMP::JMP(AssemblyInstruction* a): AssemblyInstruction(a) {}
JMP::~JMP() {}
//clone this JMP
JMP* JMP::clone() {return new JMP(this);}
//set the JMP
void JMP::set(int type, intptr_t val) {
	if (type == TCONSTANT || type == TINSTRUCTION || type == TINSTRUCTIONRELATIVE)
		setjump(type, val, "\xEB", "\xE9");
	else if (type == TREG32 || type == TDWORDPTR)
		setregptr(type, val, "\xFF", "", "", 0x20);
	else if (isdataptr(type))
		setdataptr(type, val);
}
CALL::CALL(int type, intptr_t val) {set(type, val);}
CALL::CALL(Thunk* thunk) {set(thunk);}
CALL::CALL(Function* function) {set(function);}
CALL::CALL(AssemblyInstruction* a): AssemblyInstruction(a) {}
CALL::~CALL() {}
//clone this CALL
CALL* CALL::clone() {return new CALL(this);}
//set the CALL
void CALL::set(int type, intptr_t val) {
	if (type == TCONSTANT)
		bytes.assign("\xE8").append(to4bytes(val));
	else if (type == TREG32 || type == TDWORDPTR)
		setregptr(type, val, "\xFF", "", "", 0x10);
	else if (isdataptr(type))
		setdataptr(type, val);
}
//set the CALL to call a thunk
void CALL::set(Thunk* thunk) {
	set(TDWORDPTR, blankdataptr);
	tagp = (intptr_t)(thunk);
	tag = TAGCALLTHUNK;
}
//set the CALL to call a function
void CALL::set(Function* function) {
	set(TCONSTANT, 0);
	tagp = (intptr_t)(function);
	tag = TAGCALLFUNCTION;
}
PUSH::PUSH(int type, intptr_t val) {set(type, val);}
PUSH::PUSH(AssemblyInstruction* a): AssemblyInstruction(a) {}
PUSH::~PUSH() {}
//clone this PUSH
PUSH* PUSH::clone() {return new PUSH(this);}
//set the PUSH
void PUSH::set(int type, intptr_t val) {
	if (type == TCONSTANT) {
		if (val >= -128 && val <= 127)
			bytes.assign("\x6A").append(1, (char)(val));
		else
			bytes.assign("\x68").append(to4bytes(val));
	} else if (type == TREG32)
		bytes.assign(1, (char)(0x50 + val));
	else if (type == TREG16)
		bytes.assign("\x66").append(1, (char)(0x50 + val));
	else if (type == TDWORDPTR || type == TWORDPTR)
		setregptr(type, val, "\xFF", "\x66\xFF", "", 0x30);
	else if (type == TDATAADDRESS) {
		set(TCONSTANT, IMAGEBASE);
		tag = TAGDATAADDRESS;
		tag2 = val;
	} else if (isdataptr(type))
		setdataptr(type, val);
}
POP::POP(int type, intptr_t val) {set(type, val);}
POP::POP(AssemblyInstruction* a): AssemblyInstruction(a) {}
POP::~POP() {}
//clone this POP
POP* POP::clone() {return new POP(this);}
//set the POP
void POP::set(int type, intptr_t val) {
	if (type == TREG32)
		bytes.assign(1, (char)(0x58 + val));
	else if (type == TREG16)
		bytes.assign("\x66").append(1, (char)(0x58 + val));
	else if (type == TDWORDPTR || type == TWORDPTR)
		setregptr(type, val, "\x8F", "\x66\x8F", "", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
NOT::NOT(int type, intptr_t val) {set(type, val);}
NOT::NOT(AssemblyInstruction* a): AssemblyInstruction(a) {}
NOT::~NOT() {}
//clone this NOT
NOT* NOT::clone() {return new NOT(this);}
//set the NOT
void NOT::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "\xF7", "\x66\xF7", "\xF6", 0x10);
	else if (isdataptr(type))
		setdataptr(type, val);
}
NEG::NEG(int type, intptr_t val) {set(type, val);}
NEG::NEG(AssemblyInstruction* a): AssemblyInstruction(a) {}
NEG::~NEG() {}
//clone this NEG
NEG* NEG::clone() {return new NEG(this);}
//set the NEG
void NEG::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "\xF7", "\x66\xF7", "\xF6", 0x18);
	else if (isdataptr(type))
		setdataptr(type, val);
}
IDIV::IDIV(int type, intptr_t val) {set(type, val);}
IDIV::IDIV(AssemblyInstruction* a): AssemblyInstruction(a) {}
IDIV::~IDIV() {}
//clone this IDIV
IDIV* IDIV::clone() {return new IDIV(this);}
//set the IDIV
void IDIV::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "\xF7", "\x66\xF7", "\xF6", 0x38);
	else if (isdataptr(type))
		setdataptr(type, val);
}
IMUL::IMUL(int type, intptr_t val) {set(type, val);}
IMUL::IMUL(int rtype, int regn, int type2, intptr_t val2, int constant) {setIMUL(rtype, regn, type2, val2, constant);}
IMUL::IMUL(AssemblyInstruction* a): AssemblyInstruction(a) {}
IMUL::~IMUL() {}
//clone this IMUL
IMUL* IMUL::clone() {return new IMUL(this);}
//set the IMUL
void IMUL::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "\xF7", "\x66\xF7", "\xF6", 0x28);
	else if (isdataptr(type))
		setdataptr(type, val);
}
JE::JE(int type, intptr_t val) {set(type, val);}
JE::JE(AssemblyInstruction* a): AssemblyInstruction(a) {}
JE::~JE() {}
//clone this JE
JE* JE::clone() {return new JE(this);}
//set the JE
void JE::set(int type, intptr_t val) {
	setjump(type, val, "\x74", "\x0F\x84");
}
JNE::JNE(int type, intptr_t val) {set(type, val);}
JNE::JNE(AssemblyInstruction* a): AssemblyInstruction(a) {}
JNE::~JNE() {}
//clone this JNE
JNE* JNE::clone() {return new JNE(this);}
//set the JNE
void JNE::set(int type, intptr_t val) {
	setjump(type, val, "\x75", "\x0F\x85");
}
JL::JL(int type, intptr_t val) {set(type, val);}
JL::JL(AssemblyInstruction* a): AssemblyInstruction(a) {}
JL::~JL() {}
//clone this JL
JL* JL::clone() {return new JL(this);}
//set the JL
void JL::set(int type, intptr_t val) {
	setjump(type, val, "\x7C", "\x0F\x8C");
}
JLE::JLE(int type, intptr_t val) {set(type, val);}
JLE::JLE(AssemblyInstruction* a): AssemblyInstruction(a) {}
JLE::~JLE() {}
//clone this JLE
JLE* JLE::clone() {return new JLE(this);}
//set the JLE
void JLE::set(int type, intptr_t val) {
	setjump(type, val, "\x7E", "\x0F\x8E");
}
JG::JG(int type, intptr_t val) {set(type, val);}
JG::JG(AssemblyInstruction* a): AssemblyInstruction(a) {}
JG::~JG() {}
//clone this JG
JG* JG::clone() {return new JG(this);}
//set the JG
void JG::set(int type, intptr_t val) {
	setjump(type, val, "\x7F", "\x0F\x8F");
}
JGE::JGE(int type, intptr_t val) {set(type, val);}
JGE::JGE(AssemblyInstruction* a): AssemblyInstruction(a) {}
JGE::~JGE() {}
//clone this JGE
JGE* JGE::clone() {return new JGE(this);}
//set the JGE
void JGE::set(int type, intptr_t val) {
	setjump(type, val, "\x7D", "\x0F\x8D");
}
SETE::SETE(int type, intptr_t val) {set(type, val);}
SETE::SETE(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETE::~SETE() {}
//clone this SETE
SETE* SETE::clone() {return new SETE(this);}
//set the SETE
void SETE::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x94", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
SETNE::SETNE(int type, intptr_t val) {set(type, val);}
SETNE::SETNE(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETNE::~SETNE() {}
//clone this SETNE
SETNE* SETNE::clone() {return new SETNE(this);}
//set the SETNE
void SETNE::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x95", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
SETLE::SETLE(int type, intptr_t val) {set(type, val);}
SETLE::SETLE(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETLE::~SETLE() {}
//clone this SETLE
SETLE* SETLE::clone() {return new SETLE(this);}
//set the SETLE
void SETLE::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x9E", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
SETGE::SETGE(int type, intptr_t val) {set(type, val);}
SETGE::SETGE(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETGE::~SETGE() {}
//clone this SETGE
SETGE* SETGE::clone() {return new SETGE(this);}
//set the SETGE
void SETGE::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x9D", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
SETL::SETL(int type, intptr_t val) {set(type, val);}
SETL::SETL(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETL::~SETL() {}
//clone this SETL
SETL* SETL::clone() {return new SETL(this);}
//set the SETL
void SETL::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x9C", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
SETG::SETG(int type, intptr_t val) {set(type, val);}
SETG::SETG(AssemblyInstruction* a): AssemblyInstruction(a) {}
SETG::~SETG() {}
//clone this SETG
SETG* SETG::clone() {return new SETG(this);}
//set the SETG
void SETG::set(int type, intptr_t val) {
	if (isreg(type) || isptr(type))
		setregptr(type, val, "", "", "\x0F\x9F", 0);
	else if (isdataptr(type))
		setdataptr(type, val);
}
ADD::ADD(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
ADD::ADD(AssemblyInstruction* a): AssemblyInstruction(a) {}
ADD::~ADD() {}
//clone this ADD
ADD* ADD::clone() {return new ADD(this);}
//set the ADD
void ADD::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xC0, "\x05", "\x03");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xC0, "\x66\x05", "\x66\x03");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xC0, "\x04", "\x02");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x01", "\x83", "\x81", 0);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x01", "\x66\x83", "\x66\x81", 0);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, string("\0", 1), "\x80", "", 0);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
SUB::SUB(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
SUB::SUB(AssemblyInstruction* a): AssemblyInstruction(a) {}
SUB::~SUB() {}
//clone this SUB
SUB* SUB::clone() {return new SUB(this);}
//set the SUB
void SUB::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xE8, "\x2D", "\x2B");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xE8, "\x66\x2D", "\x66\x2B");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xE8, "\x2C", "\x2A");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x29", "\x83", "\x81", 0x28);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x29", "\x66\x83", "\x66\x81", 0x28);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x28", "\x80", "", 0x28);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
MOV::MOV(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
MOV::MOV(AssemblyInstruction* a): AssemblyInstruction(a) {}
MOV::~MOV() {}
//clone this MOV
MOV* MOV::clone() {return new MOV(this);}
//set the MOV
void MOV::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	//MOV-exclusive specials
	if (val1 == EAX && isreg(type1) && type2 == ptrfromreg(type1)) {
		MEMPTR* memptr = (MEMPTR*)(val2);
		if (memptr->reg1 == -1) {
			if (type1 == TREG32)
				bytes.assign("\xA1").append(to4bytes(memptr->constant));
			else if (type1 == TREG16)
				bytes.assign("\x66\xA1").append(to4bytes(memptr->constant));
			else if (type1 == TREG8)
				bytes.assign("\xA0").append(to4bytes(memptr->constant));
			if (memptr->deletable)
				delete memptr;
			return;
		}
	} else if (val2 == EAX && isreg(type2) && type1 == ptrfromreg(type2)) {
		MEMPTR* memptr = (MEMPTR*)(val1);
		if (memptr->reg1 == -1) {
			if (type2 == TREG32)
				bytes.assign("\xA3").append(to4bytes(memptr->constant));
			else if (type2 == TREG16)
				bytes.assign("\x66\xA3").append(to4bytes(memptr->constant));
			else if (type2 == TREG8)
				bytes.assign("\xA2").append(to4bytes(memptr->constant));
			if (memptr->deletable)
				delete memptr;
			return;
		}
	}
	//move data addresses
	if (type2 == TDATAADDRESS) {
		set(type1, val1, TCONSTANT, IMAGEBASE);
		tag = TAGOPXDATAADDRESS;
		tag2 = val2;
		tag3 = type1;
		tag4 = val1;
	//move code addresses
	} else if (type2 == TCODEADDRESS) {
		set(type1, val1, TCONSTANT, IMAGEBASE);
		tag = TAGOPXCODEADDRESS;
		tagp = val2;
		tag3 = type1;
		tag4 = val1;
	//regular setting
	} else if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "", "", 0xB8, "", "\x8B");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "", "\x66", 0xB8, "", "\x66\x8B");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "", "", 0xB0, "", "\x8A");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x89", "", "\xC7", 0);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x89", "", "\x66\xC7", 0);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x88", "\xC6", "", 0);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
AND::AND(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
AND::AND(AssemblyInstruction* a): AssemblyInstruction(a) {}
AND::~AND() {}
//clone this AND
AND* AND::clone() {return new AND(this);}
//set the AND
void AND::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xE0, "\x25", "\x23");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xE0, "\x66\x25", "\x66\x23");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xE0, "\x24", "\x22");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x21", "\x83", "\x81", 0x20);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x21", "\x66\x83", "\x66\x81", 0x20);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x20", "\x80", "", 0x20);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
OR::OR(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
OR::OR(AssemblyInstruction* a): AssemblyInstruction(a) {}
OR::~OR() {}
//clone this OR
OR* OR::clone() {return new OR(this);}
//set the OR
void OR::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xC8, "\x0D", "\x0B");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xC8, "\x66\x0D", "\x66\x0B");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xC8, "\x0C", "\x0A");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x09", "\x83", "\x81", 8);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x09", "\x66\x83", "\x66\x81", 8);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x08", "\x80", "", 8);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
XOR::XOR(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
XOR::XOR(AssemblyInstruction* a): AssemblyInstruction(a) {}
XOR::~XOR() {}
//clone this XOR
XOR* XOR::clone() {return new XOR(this);}
//set the XOR
void XOR::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xF0, "\x35", "\x33");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xF0, "\x66\x35", "\x66\x33");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xF0, "\x34", "\x32");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x31", "\x83", "\x81", 0x30);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x31", "\x66\x83", "\x66\x81", 0x30);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x30", "\x80", "", 0x30);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
CMP::CMP(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
CMP::CMP(AssemblyInstruction* a): AssemblyInstruction(a) {}
CMP::~CMP() {}
//clone this CMP
CMP* CMP::clone() {return new CMP(this);}
//set the CMP
void CMP::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setreg(type1, val1, type2, val2, "\x83", "\x81", 0xF8, "\x3D", "\x3B");
	else if (type1 == TREG16)
		setreg(type1, val1, type2, val2, "\x66\x83", "\x66\x81", 0xF8, "\x66\x3D", "\x66\x3B");
	else if (type1 == TREG8)
		setreg(type1, val1, type2, val2, "\x80", "", 0xF8, "\x3C", "\x3A");
	else if (type1 == TDWORDPTR)
		setptr(type1, val1, type2, val2, "\x39", "\x83", "\x81", 0x38);
	else if (type1 == TWORDPTR)
		setptr(type1, val1, type2, val2, "\x66\x39", "\x66\x83", "\x66\x81", 0x38);
	else if (type1 == TBYTEPTR)
		setptr(type1, val1, type2, val2, "\x38", "\x80", "", 0x38);
	else if (isdataptr(type1))
		setdataptr(type1, val1, type2, val2);
}
LEA::LEA(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
LEA::LEA(AssemblyInstruction* a): AssemblyInstruction(a) {}
LEA::~LEA() {}
//clone this LEA
LEA* LEA::clone() {return new LEA(this);}
//set the LEA
void LEA::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	setreg(type1, val1, type2, val2, "", "", 0, "", "\x8D");
}
SHL::SHL(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
SHL::SHL(AssemblyInstruction* a): AssemblyInstruction(a) {}
SHL::~SHL() {}
//clone this SHL
SHL* SHL::clone() {return new SHL(this);}
//set the SHL
void SHL::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setshiftreg(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0xE0);
	else if (type1 == TREG16)
		setshiftreg(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0xE0);
	else if (type1 == TREG8)
		setshiftreg(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0xE0);
	else if (type1 == TDWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0x20);
	else if (type1 == TWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0x20);
	else if (type1 == TBYTE)
		setshiftptr(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0x20);
}
SHR::SHR(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
SHR::SHR(AssemblyInstruction* a): AssemblyInstruction(a) {}
SHR::~SHR() {}
//clone this SHR
SHR* SHR::clone() {return new SHR(this);}
//set the SHR
void SHR::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setshiftreg(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0xE8);
	else if (type1 == TREG16)
		setshiftreg(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0xE8);
	else if (type1 == TREG8)
		setshiftreg(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0xE8);
	else if (type1 == TDWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0x28);
	else if (type1 == TWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0x28);
	else if (type1 == TBYTE)
		setshiftptr(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0x28);
}
ROL::ROL(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
ROL::ROL(AssemblyInstruction* a): AssemblyInstruction(a) {}
ROL::~ROL() {}
//clone this ROL
ROL* ROL::clone() {return new ROL(this);}
//set the ROL
void ROL::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setshiftreg(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0xC0);
	else if (type1 == TREG16)
		setshiftreg(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0xC0);
	else if (type1 == TREG8)
		setshiftreg(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0xC0);
	else if (type1 == TDWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0);
	else if (type1 == TWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0);
	else if (type1 == TBYTE)
		setshiftptr(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0);
}
ROR::ROR(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
ROR::ROR(AssemblyInstruction* a): AssemblyInstruction(a) {}
ROR::~ROR() {}
//clone this ROR
ROR* ROR::clone() {return new ROR(this);}
//set the ROR
void ROR::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setshiftreg(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0xC8);
	else if (type1 == TREG16)
		setshiftreg(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0xC8);
	else if (type1 == TREG8)
		setshiftreg(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0xC8);
	else if (type1 == TDWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 8);
	else if (type1 == TWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 8);
	else if (type1 == TBYTE)
		setshiftptr(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 8);
}
SAR::SAR(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
SAR::SAR(AssemblyInstruction* a): AssemblyInstruction(a) {}
SAR::~SAR() {}
//clone this SAR
SAR* SAR::clone() {return new SAR(this);}
//set the SAR
void SAR::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32)
		setshiftreg(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0xF8);
	else if (type1 == TREG16)
		setshiftreg(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0xF8);
	else if (type1 == TREG8)
		setshiftreg(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0xF8);
	else if (type1 == TDWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\xD1", "\xC1", "\xD3", 0x38);
	else if (type1 == TWORDPTR)
		setshiftptr(type1, val1, type2, val2, "\x66\xD1", "\x66\xC1", "\x66\xD3", 0x38);
	else if (type1 == TBYTE)
		setshiftptr(type1, val1, type2, val2, "\xD0", "\xC0", "\xD2", 0x38);
}
MOVSX::MOVSX(int type1, intptr_t val1, int type2, intptr_t val2) {set(type1, val1, type2, val2);}
MOVSX::MOVSX(AssemblyInstruction* a): AssemblyInstruction(a) {}
MOVSX::~MOVSX() {}
//clone this MOVSX
MOVSX* MOVSX::clone() {return new MOVSX(this);}
//set the MOVSX
void MOVSX::set(int type1, intptr_t val1, int type2, intptr_t val2) {
	if (type1 == TREG32) {
		if (type2 == TREG16 || type2 == TWORDPTR)
			setreg(type1, val1, type2, val2, "", "", 0, "", "\x0F\xBF");
		else if (type2 == TREG8 || type2 == TBYTEPTR)
			setreg(type1, val1, type2, val2, "", "", 0, "", "\x0F\xBE");
	} else if (type1 == TREG16) {
		if (type2 == TREG8 || type2 == TBYTEPTR)
			setreg(type1, val1, type2, val2, "", "", 0, "", "\x66\x0F\xBE");
	}
}
*/
