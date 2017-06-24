#include "../General/globals.h"

class BigInt2 onlyInDebug(: public ObjCounter) {
public:
	BigInt2(int pBase);
	BigInt2(BigInt2* pSource);
	virtual ~BigInt2();

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
	void multiply(BigInt2* other);
	void lShift(int bits);
	void longDiv(BigInt2* other);
	int compare(BigInt2* other);
	void subtract(BigInt2* other);
	void rShift(int bits);
};
