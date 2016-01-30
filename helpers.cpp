#include "general.h"
#include "AssemblyInstruction.h"
#include "Representation.h"
int ObjCounter::objcount = 0;
int ObjCounter::objid = -1;
int ObjCounter::lowobjid;
bool ObjCounter::objids [ObjCounter::OBJIDSCOUNT];
bool ObjCounter::trackobjids = false;
ObjCounter::ObjCounter(char* thectype) {
	ctype = thectype;
	objcount += 1;
	objcountid = (objid += 1);
	if (trackobjids) {
		printf("  Added %s\t%d,\tobj count: %d\n", ctype, objcountid, objcount);
		objids[objcountid] = true;
	}
}
ObjCounter::~ObjCounter() {
	objcount -= 1;
	if (trackobjids) {
		printf("Deleted %s\t%d,\tobj count: %d\n", ctype, objcountid, objcount);
		objids[objcountid] = false;
	}
}
//initialize the objids array
void ObjCounter::start() {
	lowobjid = objid + 1;
	if (trackobjids) {
		for (int i = lowobjid; i < OBJIDSCOUNT; i += 1) {
			objids[i] = false;
		}
	}
}
//final check for the objids array
void ObjCounter::end() {
	if (trackobjids) {
		bool printed = false;
		for (int i = lowobjid; i < OBJIDSCOUNT; i += 1) {
			if (objids[i]) {
				printf("Remaining object %d\n", i);
				printed = true;
			}
		}
		if (!printed)
			puts("No remaining objects!");
	} else
		printf("Total remaining objects: %d\n", objcount - lowobjid);
	printf("Total objects used: %d\n", objid + 1);
}

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
template <class type> Array<type>::Array():
ObjCounter("ARRY"),
	inner(new type[1]),
	innerlength(1),
	length(0) {
}
template <class type> Array<type>::Array(type t):
ObjCounter("ARRY"),
	inner(new type[1]),
	innerlength(1),
	length(1) {
	inner[0] = t;
}
template <class type> Array<type>::~Array() {
	delete[] inner;
}
//resize the array by the given scale
template <class type> void Array<type>::resize(int scale) {
	type* newinner = new type[innerlength * scale];
	for (int i = 0; i < length; i += 1) {
		newinner[i] = inner[i];
	}
	delete[] inner;
	inner = newinner;
	innerlength *= scale;
}
//add an item to the array at the given spot
template <class type> Array<type>* Array<type>::add(type t, int pos) {
	if (length >= innerlength)
		resize(2);
	for (int i = length; i > pos; i -= 1) {
		inner[i] = inner[i - 1];
	}
	inner[pos] = t;
	length += 1;
	return this;
}
//add an item to the array
template <class type> Array<type>* Array<type>::add(type t) {
	if (length >= innerlength)
		resize(2);
	inner[length] = t;
	length += 1;
	return this;
}
//add another array's items to the array at the given spot, delete the array if indicated
template <class type> Array<type>* Array<type>::add(Array<type>* a, int pos, bool deletable) {
	int shift = a->length;
	if (shift + length > innerlength) {
		int scale = 2;
		while (shift + length > innerlength * scale)
			scale *= 2;
		resize(scale);
	}
	for (int i = length - 1; i >= pos; i -= 1) {
		inner[i + shift] = inner[i];
	}
	type* other = a->inner;
	for (int i = 0; i < shift; i += 1) {
		inner[i + pos] = other[i];
	}
	length += shift;
	if (deletable)
		delete a;
	return this;
}
//add another array's items to the array, delete the array if indicated
template <class type> Array<type>* Array<type>::add(Array<type>* a, bool deletable) {
	return add(a, length, deletable);
}
//add another array's items to the array at the given spot
template <class type> Array<type>* Array<type>::add(Array<type>* a, int pos) {
	return add(a, pos, true);
}
//add another array's items to the array at the given spot
template <class type> Array<type>* Array<type>::add(Array<type>* a) {
	return add(a, length, true);
}
//remove the item at the given spot
template <class type> Array<type>* Array<type>::remove(int pos) {
	return remove(pos, 1);
}
//remove the given number of items at the given spot
template <class type> Array<type>* Array<type>::remove(int pos, int num) {
	for (int i = pos + num; i < length; i += 1) {
		inner[i - num] = inner[i];
	}
	length -= num;
	return this;
}
/*
//get a subarray of this array
template <class type> Array<type>* Array<type>::subArray(int pos, int num) {
	Array<type>* a = new Array<type>();
	a->resize(num);
	for (int i = 0; i < num; i += 1) {
		a->inner[i] = inner[i + pos];
	}
	a->length = num;
	return a;
}
*/
//pop off the last item
template <class type> type Array<type>::pop() {
	length -= 1;
	return inner[length];
}
template <class type> ArrayIterator<type>::ArrayIterator(Array<type>* a)
: ObjCounter("ARYI")
 {
	if (a != NULL) {
		inner = a->inner;
		length = a->length;
	} else
		length = 0;
}
template <class type> ArrayIterator<type>::~ArrayIterator() {}
//get the first element of the array
template <class type> type ArrayIterator<type>::getFirst() {
	index = 0;
	if (length < 1)
		return NULL;
	return inner[0];
}
//get the next element of the array
template <class type> type ArrayIterator<type>::getNext() {
	if ((index += 1) >= length)
		return NULL;
	return inner[index];
}
//check if the iterator has this element
template <class type> bool ArrayIterator<type>::hasThis() {
	return index < length;
}
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
