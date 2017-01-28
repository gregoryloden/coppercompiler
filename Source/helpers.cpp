#include "general.h"
#include "AssemblyInstruction.h"
#include "Representation.h"
int ObjCounter::objCount = 0;
int ObjCounter::nextObjID = 0;
int ObjCounter::untrackedObjCount;
bool ObjCounter::objIDs [ObjCounter::OBJ_IDS_COUNT];
bool ObjCounter::trackObjIDs = false;
ObjCounter::ObjCounter(char* pObjType)
: objType(pObjType)
, objID(nextObjID)
{
	nextObjID++;
	objCount++;
	if (trackObjIDs) {
		printf("  Added %s\t%d,\tobj count: %d\n", objType, objID, objCount);
		objIDs[objID] = true;
	}
}
ObjCounter::~ObjCounter() {
	objCount--;
	if (trackObjIDs) {
		printf("Deleted %s\t%d,\tobj count: %d\n", objType, objID, objCount);
		objIDs[objID] = false;
	}
}
void ObjCounter::start() {
	untrackedObjCount = nextObjID;
}
//final check for the objids array
void ObjCounter::end() {
	if (trackObjIDs) {
		bool printed = false;
		for (int i = untrackedObjCount; i < nextObjID; i += 1) {
			if (objIDs[i]) {
				printf("Remaining object %d\n", i);
				printed = true;
			}
		}
		if (!printed)
			puts("No remaining objects!");
	} else
		printf("Total remaining objects: %d\n", objCount - untrackedObjCount);
	printf("Total objects used: %d\n", nextObjID);
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
int min(int a, int b) {
	return a < b ? a : b;
}
int max(int a, int b) {
	return a > b ? a : b;
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
//------------------------------------------------------------------------------------------------------------------
BigInt2::BigInt2(int pBase)
: ObjCounter("BGNT")
, base(pBase)
, innerLength(1)
, highByte(-1)
, inner(new unsigned char[1]) {
}
//this one is for use right before pSource will be deleted or its inner will be replaced
BigInt2::BigInt2(BigInt2* pSource)
: ObjCounter("BGNT")
, base(pSource->base)
, innerLength(pSource->innerLength)
, highByte(pSource->highByte)
, inner(pSource->inner) {
	pSource->inner = nullptr;
}
BigInt2::~BigInt2() {
	delete[] inner;
}
//add a digit of this BigInt's base
void BigInt2::digit(unsigned char c) {
	short carry = (short)(c) << 8;
	for (int i = 0; i <= highByte; i++) {
		carry = (short)(inner[i]) * (short)(base) + (carry >> 8);
		inner[i] = (unsigned char)carry;
	}
	if (carry >= 256) {
		highByte++;
		if (highByte >= innerLength)
			expand();
		inner[highByte] = (unsigned char)(carry >> 8);
	}
}
//double the length of the inner array
void BigInt2::expand() {
	innerLength *= 2;
	unsigned char* newInner = new unsigned char[innerLength];
	for (int i = highByte; i >= 0; i--)
		newInner[i] = inner[i];
	delete[] inner;
	inner = newInner;
}
//get this BigInt as an int
int BigInt2::getInt() {
	int val = 0;
	for (int i = highByte; i >= 0; i--)
		val = val << 8 | (int)(inner[i]);
	return val;
}
//get the index of the highest set bit
int BigInt2::highBit() {
	if (highByte == -1)
		return -1;
	int num = highByte * 8;
	for (unsigned char val = inner[highByte]; val > 0; val >>= 1)
		num++;
	return num - 1;
}
//square this BigInt
void BigInt2::square() {
	multiply(this);
}
//multiply this BigInt by the other
//assumes neither are 0
void BigInt2::multiply(BigInt2* other) {
	//cache some information
	unsigned char* oldInner = inner;
	unsigned char* otherInner = other->inner;
	int oldHighByte = highByte;
	int otherHighByte = other->highByte;
	//calculate what the new highByte will be and make a new array to fit it
	highByte = oldHighByte + otherHighByte + (((short)(oldInner[oldHighByte]) * (short)(otherInner[otherHighByte])) >= 256 ? 1 : 0);
	while (highByte >= innerLength)
		innerLength *= 2;
	unsigned char* newInner = new unsigned char[innerLength];
	//do a simple O(n^2) multiplication
	for (int i = 0; i <= oldHighByte; i++) {
		for (int j = 0; j <= otherHighByte; j++) {
			int resultIndex = i + j;
			short product = (short)(oldInner[i]) * (short)(otherInner[j]);
			short newSum = newInner[resultIndex] + product;
			while (true) {
				newInner[resultIndex] = (unsigned char)newSum;
				if (newSum >= 256) {
					resultIndex++;
					newSum = (newSum >> 8) + newInner[resultIndex];
				} else
					break;
			}
		}
	}
	delete inner;
	inner = newInner;
}
//left shift the BigInt by the given amount
void BigInt2::lShift(int bits) {
	//no bits so nothing to do
	if (highByte == -1)
		return;

	char bitShift = bits & 7;
	int byteShift = bits >> 3;
	unsigned char* oldInner = inner;
	//make sure there's space to shift to
	int newHighByte = (highBit() + bits) / 8;
	if (newHighByte >= innerLength) {
		while (newHighByte >= innerLength)
			innerLength *= 2;
		inner = new unsigned char[innerLength];
	}
	//no inter-byte shifting is easy
	if (bitShift == 0) {
		for (int i = highByte; i >= 0; i--)
			inner[i + byteShift] = inner[i];
	//inter-byte shifting, combine two bytes after shifting them appropriately
	//we want the upper bits of the lower byte and the lower bits of the upper byte
	} else {
		char reverseShift = 8 - bitShift;
		//first, find out if the highbyte shifted into a new byte
		unsigned char shifted = oldInner[highByte] >> reverseShift;
		if (shifted > 0)
			inner[newHighByte] = shifted;
		for (int i = highByte; i > 0; i--)
			inner[i + byteShift] = (oldInner[i] << bitShift) | (oldInner[i - 1] >> reverseShift);
		inner[byteShift] = inner[0] << bitShift;
	}
	if (oldInner != inner)
		delete inner;
	highByte = newHighByte;
}
//divide this BigInt by the other
//destroys the contents of other
//assumes that this > other > 0
//rounds to-nearest half-to-even
void BigInt2::longDiv(BigInt2* other) {
	//stop if we're dividing by 1
	if (other->highBit() == 0)
		return;

	BigInt2 original (this);
	int shifted = original.highBit() - other->highBit();
	other->lShift(shifted);
	//size our new array to make sure it can handle a quotient rounded up and a half bit
	int maxHighByte = (shifted + 2) / 8;
	innerLength = 1;
	while (maxHighByte >= innerLength)
		innerLength *= 2;
	inner = new unsigned char[innerLength];
	base = 2;
	//subtract and shift accordingly
	//go one extra for rounding purposes
	for (; shifted >= -1; shifted--) {
		if (original.compare(other) > 0) {
			original.subtract(other);
			digit(1);
		} else
			digit(0);
		other->rShift(1);
	}
	//round up if necessary; rounding down has no effect on the final value
	//we need to round up if the half bit is 1 and there is any remainder or rounding up results in an even number
	if ((original.inner[0] & 1) != 0 && (original.highBit() >= 0 ||( original.inner[0] & 2) != 0)) {
		int i = 0;
		//round up, cascade as necessary
		for (; (original.inner[i] += 1) == 0; i++)
			;
		if (i > highByte)
			highByte = i;
	}
	//get rid of the half bit and restore the base
	rShift(1);
	base = original.base;
}
//compare this BigInt to the other
//positive if greater, negative if less, 0 if equal
int BigInt2::compare(BigInt2* other) {
	if (highByte != other->highByte)
		return highByte - other->highByte;
	for (int i = highByte; i >= 0; i--) {
		if (inner[i] != other->inner[i])
			//cast to signed char before extending to int
			return (int)((char)(inner[i] - other->inner[i]));
	}
	return 0;
}
//subtract the other BigInt from this one
//assumes this BigInt is greater than the other
void BigInt2::subtract(BigInt2* other) {
	short carry = 0;
	int i = 0;
	for (; i <= other->highByte; i++) {
		//not sure whether >> is SAR or SHR so this works either way
		carry = (short)(inner[i]) - (short)(other->inner[i]) + (carry /*>> 8*/ < 0 ? -1 : 0);
		inner[i] = (unsigned char)carry;
	}
	for (; carry < 0; i++) {
		//not sure whether >> is SAR or SHR so this works either way
		carry = (short)(inner[i]) + (carry /*>> 8*/ < 0 ? -1 : 0);
	}
	while (highByte >= 0 && inner[highByte] == 0)
		highByte--;
}
//right shift the BigInt by the given amount
void BigInt2::rShift(int bits) {
	char bitShift = bits & 7;
	int byteShift = bits >> 3;
	//no inter-byte shifting is easy
	if (bitShift == 0) {
		for (int i = byteShift; i <= highByte; i++)
			inner[i - byteShift] = inner[i];
	//inter-byte shifting, combine two bytes after shifting them appropriately
	//we want the upper bits of the lower byte and the lower bits of the upper byte
	} else {
		char reverseShift = 8 - bitShift;
		for (int i = byteShift; i < highByte; i++)
			inner[i - byteShift] = (inner[i] >> bitShift) | (inner[i + 1] << reverseShift);
		if (highByte >= byteShift && (inner[highByte - byteShift] = inner[highByte] >> bitShift) == 0)
			highByte -= 1;
	}
	highByte -= byteShift;
}
