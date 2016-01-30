#include "string"
class ObjCounter {
public:
	ObjCounter(char* thectype);
	~ObjCounter();

	static void start();
	static void end();

	char* ctype;
	int objcountid;
	static const int OBJIDSCOUNT = 4096;
	static int objcount;
	static int objid;
	static int lowobjid;
	static bool trackobjids;
	static bool objids [OBJIDSCOUNT];
};
using namespace std;

string to4bytes(int i);
string to2bytes(int i);
int roundup(int num, int divisor);
int intval(bool val);

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
