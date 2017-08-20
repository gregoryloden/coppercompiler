#include "../General/globals.h"

template <class Type> class Array onlyInDebug(: public ObjCounter) {
public:
	Array();
	virtual ~Array();

	int length; //copper: readonly
private:
	Type* inner;
	int innerLength;

public:
	void deleteSelfAndContents();
private:
	void resize(int scale);
public:
	Type get(int pos);
	void set(int pos, Type t);
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
	void clear();
};
template <class Type> class ArrayIterator onlyInDebug(: public ObjCounter) {
public:
	ArrayIterator(Array<Type>* pA);
	virtual ~ArrayIterator();

private:
	int index;
	Array<Type>* a;

public:
	Type getFirst();
	Type getNext();
	bool hasThis();
	void replaceThis(Type t);
};
