#include "globals.h"

template <class Type> class Array onlyInDebug(: public ObjCounter) {
public:
	Array();
	virtual ~Array();

	void add(Type t, int pos);
	void add(Type t);
	void add(Array<Type>* a, int pos, bool deletable);
	void add(Array<Type>* a, bool deletable);
	void add(Array<Type>* a, int pos);
	void add(Array<Type>* a);
	void remove(int pos, int num);
	void remove(int pos);
	Type first();
	Type pop();

	Type* inner; //readonly<ArrayIterator>
	int length; //readonly
private:
	int innerLength;

	void resize(int scale);
};
template <class Type> class ArrayIterator onlyInDebug(: public ObjCounter) {
public:
	ArrayIterator(Array<Type>* a);
	virtual ~ArrayIterator();

	Type getFirst();
	Type getNext();
	bool hasThis();
private:
	Type* inner;
	int length;
	int index;
};
