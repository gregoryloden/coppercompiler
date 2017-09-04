#include "../General/globals.h"

class BigInt onlyInDebug(: public ObjCounter) {
public:
	BigInt(int pBase);
	BigInt(BigInt* pSource);
	virtual ~BigInt();

private:
	unsigned char base;
	unsigned char* inner;
	int innerLength;
	int highByte;

public:
	void digit(unsigned char d);
	void expand();
	int getInt();
	int highBit();
	void square();
	void multiply(BigInt* other);
	void lShift(int bits);
	void longDiv(BigInt* other);
	int compare(BigInt* other);
	void subtract(BigInt* other);
	void rShift(int bits);
};
