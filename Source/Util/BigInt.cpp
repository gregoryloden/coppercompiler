#include "Project.h"

BigInt2::BigInt2(int pBase)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("BIGINT")) COMMA)
base(pBase)
, innerLength(1)
, highByte(-1)
, inner(new unsigned char[1]) {
}
//this one is for use right before pSource will be deleted (stack allocated) or its inner will be replaced
BigInt2::BigInt2(BigInt2* pSource)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("BIGINT")) COMMA)
base(pSource->base)
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
		carry = (short)(inner[i]) * (short)(base)+(carry >> 8);
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
	highByte = oldHighByte + otherHighByte +
		(((short)(oldInner[oldHighByte]) * (short)(otherInner[otherHighByte])) >= 256 ? 1 : 0);
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

	BigInt2 original(this);
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
	if ((original.inner[0] & 1) != 0 && (original.highBit() >= 0 || (original.inner[0] & 2) != 0)) {
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
		carry = (short)(inner[i]) - (short)(other->inner[i]) + (/* carry >> 8 */ carry < 0 ? -1 : 0);
		inner[i] = (unsigned char)carry;
	}
	for (; carry < 0; i++) {
		//not sure whether >> is SAR or SHR so this works either way
		carry = (short)(inner[i]) + (/* carry >> 8 */ carry < 0 ? -1 : 0);
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
			highByte--;
	}
	highByte -= byteShift;
}
