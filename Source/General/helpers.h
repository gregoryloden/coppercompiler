#include "globals.h"
#include "string"
using namespace std;

int min(int a, int b);
int max(int a, int b);

/*
string to4bytes(int i);
string to2bytes(int i);
int roundup(int num, int divisor);
int intval(bool val);
*/
//used to delete objects during a throw
//should always be stack allocated
template <class Type> class Deleter onlyInDebug(: public ObjCounter) {
public:
	Deleter(Type* pToDelete);
	virtual ~Deleter();

private:
	Type* toDelete;

public:
	Type* release();
	Type* retrieve();
};
/*
class BigInt
: public ObjCounter
 {
public:
	BigInt(int b);
	BigInt(BigInt* b);
	~BigInt();

	void newarray();
	void digit(int c);
	void expand(int scale);
	int getInt();
	int bitCount();
	void lShift(int bitshift);
	void rShift(int bitshift);
	void longDiv(BigInt* b);
	int compare(BigInt* b);
	void subtract(BigInt* b);
	void setBit(int bit);
	int getBit(int bit);

	int* inner;
	int innerlength;
	int base;
	int highbyte;
};
*/
