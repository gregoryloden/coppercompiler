#include "../General/globals.h"

class BigInt onlyInDebug(: public ObjCounter) {
public:
	unsigned char base;
private:
	unsigned char* inner;
	int innerLength;
	int highByte;

public:
	BigInt(int pBase);
	BigInt(BigInt* pSource, bool stealInner);
	virtual ~BigInt();

	static BigInt* createFrom(int i);
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
