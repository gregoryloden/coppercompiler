#include "../General/globals.h"

template <class Type> class Array onlyInDebug(: public ObjCounter) {
public:
	Array();
	virtual ~Array();

	Type* inner; //copper: private<readonly ArrayIterator, Error>
	int length; //copper: readonly
private:
	int innerLength;

public:
	void deleteSelfAndContents();
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
