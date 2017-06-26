#include "../General/globals.h"

template <class Type> class Array onlyInDebug(: public ObjCounter) {
public:
	Array();
	virtual ~Array();

	Type* inner; //private<readonly ArrayIterator>
	int length; //readonly
private:
	int innerLength;

public:
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
private:
	void resize(int scale);
};
template <class Type> class ArrayIterator onlyInDebug(: public ObjCounter) {
public:
	ArrayIterator(Array<Type>* a);
	virtual ~ArrayIterator();

private:
	Type* inner;
	int length;
	int index;

public:
	Type getFirst();
	Type getNext();
	bool hasThis();
};
