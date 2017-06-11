#include "globals.h"
#include "string"
using namespace std;

int min(int a, int b);
int max(int a, int b);

#ifdef DEBUG
class ObjCounter {
public:
	ObjCounter(char* pObjType);
	~ObjCounter();
private:
	char* objType;
	int objID;

	static const int OBJ_IDS_COUNT = 4096;
	static int objCount;
	static int nextObjID;
	static int untrackedObjCount;
	static bool objIDs [OBJ_IDS_COUNT];
public:
	static void start();
	static void end();
};
#endif
/*
string to4bytes(int i);
string to2bytes(int i);
int roundup(int num, int divisor);
int intval(bool val);
*/
template <class type> class Array onlyInDebug(: public ObjCounter) {
public:
	Array();
	virtual ~Array();

	void add(type t, int pos);
	void add(type t);
	void add(Array<type>* a, int pos, bool deletable);
	void add(Array<type>* a, bool deletable);
	void add(Array<type>* a, int pos);
	void add(Array<type>* a);
	void remove(int pos);
	void remove(int pos, int num);
//	void subArray(int pos, int num);
	int getLength();
	type* getInner();
	type first();
	type pop();

private:
	type* inner;
	int length;
	int innerLength;

	void resize(int scale);
};
template <class type> class ArrayIterator onlyInDebug(: public ObjCounter) {
public:
	ArrayIterator(Array<type>* a);
	virtual ~ArrayIterator();

	type getFirst();
	type getNext();
	bool hasThis();
private:
	type* inner;
	int length;
	int index;
};
//used to delete objects during a throw
//should always be stack allocated
template <class type> class Retainer onlyInDebug(: public ObjCounter) {
public:
	Retainer(type* pRetained);
	virtual ~Retainer();

	type* release();
	type* retrieve();
private:
	type* retained;
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
class BigInt2 onlyInDebug(: public ObjCounter) {
public:
	BigInt2(int pBase);
	BigInt2(BigInt2* pSource);
	virtual ~BigInt2();

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
private:
	unsigned char base;
	unsigned char* inner;
	int innerLength;
	int highByte;
};
