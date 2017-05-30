#include "string"

class ObjCounter {
public:
	ObjCounter(char* pObjType);
	~ObjCounter();

	static void start();
	static void end();

	char* objType;
	int objID;
	static const int OBJ_IDS_COUNT = 4096;
	static int objCount;
	static int nextObjID;
	static int untrackedObjCount;
	static bool trackObjIDs;
	static bool objIDs [OBJ_IDS_COUNT];
};
using namespace std;

string to4bytes(int i);
string to2bytes(int i);
int roundup(int num, int divisor);
int intval(bool val);
int min(int a, int b);
int max(int a, int b);

template <class type> class Array
: public ObjCounter
 {
public:
	Array();
	Array(type t);
	~Array();

	void resize(int scale);
	Array<type>* add(type t, int pos);
	Array<type>* add(type t);
	Array<type>* add(Array<type>* a, int pos, bool deletable);
	Array<type>* add(Array<type>* a, bool deletable);
	Array<type>* add(Array<type>* a, int pos);
	Array<type>* add(Array<type>* a);
	Array<type>* remove(int pos);
	Array<type>* remove(int pos, int num);
//	Array<type>* subArray(int pos, int num);
	type pop();

	type* inner;
	int length;
	int innerlength;
};
template <class type> class ArrayIterator
: public ObjCounter
 {
public:
	ArrayIterator(Array<type>* a);
	~ArrayIterator();

	type getFirst();
	type getNext();
	bool hasThis();

	type* inner;
	int length;
	int index;
};
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
class BigInt2
: public ObjCounter
 {
public:
	BigInt2(int pBase);
	BigInt2(BigInt2* pSource);
	~BigInt2();

	unsigned char base;
	unsigned char* inner;
	int innerLength;
	int highByte;

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
