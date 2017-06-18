#include "globals.h"

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
	void remove(int pos, int num);
	void remove(int pos);
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
