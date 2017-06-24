#include "Project.h"

/*
template class Array<AssemblyInstruction*>;
template class Array<Thunk*>;
template class Array<Function*>;
template class Array<Expression*>;
template class Array<VariableData*>;
template class Array<MainFunction*>;
template class Array<AssemblySequence*>;
template class Array<ControlFlow*>;
template class ArrayIterator<AssemblyInstruction*>;
template class ArrayIterator<Thunk*>;
template class ArrayIterator<Function*>;
template class ArrayIterator<Expression*>;
template class ArrayIterator<VariableData*>;
template class ArrayIterator<MainFunction*>;
template class ArrayIterator<AssemblySequence*>;
template class ArrayIterator<ControlFlow*>;
*/
template class Deleter<Separator2>;
template class Deleter<DirectiveTitle>;
template class Deleter<Identifier>;
template class Deleter<Array<string>>;
template class Deleter<LexToken>;

/*
string to4bytes(int i) {
	string s;
	s.reserve(4);
	s += (char)(i);
	s += (char)(i >> 8);
	s += (char)(i >> 16);
	s += (char)(i >> 24);
	return s;
}
string to2bytes(int i) {
	string s;
	s.reserve(2);
	s += (char)(i);
	s += (char)(i >> 8);
	return s;
}
int roundup(int num, int divisor) {
	return num + divisor - ((num - 1) % divisor + 1);
}
int intval(bool val) {
	if (val)
		return -1;
	return 0;
}
*/
int min(int a, int b) {
	return a < b ? a : b;
}
int max(int a, int b) {
	return a > b ? a : b;
}

template <class Type> Deleter<Type>::Deleter(Type* pToDelete)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("DELETER")))
toDelete(pToDelete) {
}
template <class Type> Deleter<Type>::~Deleter() {
	delete toDelete;
}
//return the held object without releasing it
template <class Type> Type* Deleter<Type>::retrieve() {
	return toDelete;
}
//return the held object and release it so it won't be deleted
template <class Type> Type* Deleter<Type>::release() {
	Type* val = toDelete;
	toDelete = nullptr;
	return val;
}
/*
BigInt::BigInt(int b)
: ObjCounter("BGNT")
 {
	base = b;
	newarray();
}
BigInt::BigInt(BigInt* b)
: ObjCounter("BGNT")
 {
	base = b->base;
	inner = b->inner;
	innerlength = b->innerlength;
	highbyte = b->highbyte;
	//make sure it doesn't use this array any more
	b->newarray();
}
BigInt::~BigInt() {
	delete[] inner;
}
//gives the BigInt a new, empty array
void BigInt::newarray() {
	inner = new int[4];
	for (int i = 0; i < 4; i += 1)
		inner[i] = 0;
	innerlength = 4;
	highbyte = -1;
}
//add a digit of this BigInt's base
void BigInt::digit(int c) {
	for (int i = 0; c != 0 || i <= highbyte; i += 1) {
		if (i > highbyte && (highbyte = i) >= innerlength)
			expand(2);
		if ((inner[i] = inner[i] * base + c) >= 256) {
			c = inner[i] >> 8;
			inner[i] &= 255;
		} else
			c = 0;
	}
}
//double the length of the inner array
void BigInt::expand(int scale) {
	int* newinner = new int[innerlength * scale];
	int j = 0;
	for (; j < innerlength; j += 1)
		newinner[j] = inner[j];
	innerlength *= scale;
	for (; j < innerlength; j += 1)
		newinner[j] = 0;
	delete[] inner;
	inner = newinner;
}
//get this BigInt as an int
int BigInt::getInt() {
	return inner[0] & 0xFF | (inner[1] & 0xFF) << 8 | (inner[2] & 0xFF) << 16 | (inner[3] & 0xFF) << 24;
}
//get the number of bits that the int occupies
int BigInt::bitCount() {
	if (highbyte == -1)
		return 0;
	int num = highbyte * 8;
	for (int val = inner[highbyte]; val > 0; num += 1)
		val >>= 1;
	return num;
}
//left shift the BigInt by the given amount
void BigInt::lShift(int bitshift) {
	if (highbyte == -1)
		return;
	int byteshift = bitshift >> 3;
	//shift bytes
	if (byteshift > 0) {
		int scale = 2;
		while (byteshift + highbyte >= innerlength * scale)
			scale *= 2;
		expand(scale);
		for (int i = highbyte; i >= 0; i -= 1)
			inner[i + byteshift] = inner[i];
		highbyte += byteshift;
		for (int i = 0; i < byteshift; i += 1)
			inner[i] = 0;
	}
	//shift bits
	if ((bitshift &= 7) > 0) {
		int c = 0;
		for (int i = byteshift; c != 0 || i <= highbyte; i += 1) {
			if (i > highbyte && (highbyte = i) >= innerlength)
				expand(2);
			if ((inner[i] = inner[i] << bitshift | c) >= 256) {
				c = inner[i] >> 8;
				inner[i] &= 255;
			} else
				c = 0;
		}
	}
}
//right shift the BigInt by the given amount
void BigInt::rShift(int bitshift) {
	if (highbyte == -1)
		return;
	int byteshift = bitshift >> 3;
	//shift bytes
	if (byteshift > highbyte) {
		for (int i = 0; i <= highbyte; i += 1)
			inner[i] = 0;
		highbyte = -1;
		return;
	} else if (byteshift > 0) {
		for (int i = byteshift; i <= highbyte; i += 1)
			inner[i - byteshift] = inner[i];
		for (int i = highbyte - byteshift + 1; i <= highbyte; i += 1)
			inner[i] = 0;
		highbyte -= byteshift;
	}
	//shift bits
	if ((bitshift &= 7) > 0) {
		for (int i = 0; i < highbyte; i += 1)
			inner[i] = ((inner[i] >> bitshift) | (inner[i + 1] << (8 - bitshift))) & 255;
		if ((inner[highbyte] >>= bitshift) == 0)
			highbyte -= 1;
	}
}
//long division by the given BigInt
//destroys the value of the given BigInt
//assumes the given BigInt has a smaller (but positive) bitcount than this BigInt
void BigInt::longDiv(BigInt* b) {
	BigInt d (this);
	int dbcount = d.bitCount();
	int bbcount = b->bitCount();
	int shifted = dbcount - bbcount;
	b->lShift(shifted);
	bbcount = dbcount;
	while (shifted >= 0) {
		if (d.compare(b) < 0) {
			b->rShift(1);
			bbcount -= 1;
			if ((shifted -= 1) < 0)
				break;
		}
		d.subtract(b);
		dbcount = d.bitCount();
		setBit(shifted);
		//shift to the next long division test
		if (bbcount > dbcount) {
			int toshift = bbcount - dbcount;
			b->rShift(toshift);
			bbcount = dbcount;
			shifted -= toshift;
		}
	}
}
//compare this BigInt to the given BigInt
//positive if greater, negative if less, 0 if equal
int BigInt::compare(BigInt* b) {
	if (highbyte != b->highbyte)
		return highbyte - b->highbyte;
	for (int i = highbyte; i >= 0; i -= 1) {
		if (inner[i] != b->inner[i])
			return inner[i] - b->inner[i];
	}
	return 0;
}
//subtract the given BigInt from this BigInt
//assumes this bigInt is greater than the given BigInt
//assumes the given BigInt has an array as big as this one's
void BigInt::subtract(BigInt* b) {
	int carry = 0;
	for (int i = 0; i < highbyte; i += 1) {
		if ((inner[i] -= b->inner[i]) < 0) {
			inner[i + 1] -= 1;
			inner[i] += 256;
		}
	}
	inner[highbyte] -= b->inner[highbyte];
	while (highbyte >= 0 && inner[highbyte] == 0)
		highbyte -= 1;
}
//set the given bit
void BigInt::setBit(int bit) {
	int byte = bit >> 3;
	if (byte > highbyte) {
		highbyte = byte;
		int scale = 2;
		while (highbyte >= innerlength * scale)
			scale *= 2;
		expand(scale);
	}
	inner[byte] |= 1 << (bit & 7);
}
//get the given bit
int BigInt::getBit(int bit) {
	int byte = bit >> 3;
	if (byte > highbyte)
		return 0;
	return inner[byte] >> (bit & 7) & 1;
}
*/
//------------------------------------------------------------------------------------------------------------------
